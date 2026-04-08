#include "DelayProcessor.h"
#include "ZoneKnob.h"

DelayProcessor::DelayProcessor (std::atomic<float>* dlyMix,
                                 std::atomic<float>* dlyTime)
    : dlyMix (dlyMix), dlyTime (dlyTime)
{
}

void DelayProcessor::prepare (double sr, int)
{
    sampleRate = sr;
    // Allocate enough for 1 second at any sample rate + headroom
    const int bufSize = (int) (sr * 1.1);
    delayBufL.assign (bufSize, 0.f);
    delayBufR.assign (bufSize, 0.f);
    reset();
}

void DelayProcessor::reset()
{
    std::fill (delayBufL.begin(), delayBufL.end(), 0.f);
    std::fill (delayBufR.begin(), delayBufR.end(), 0.f);
    writePosL = writePosR = 0;
    feedbackL = feedbackR = 0.f;
    lpfStateL = lpfStateR = 0.f;
    wowPhase  = flutterPhase = 0.f;
}

float DelayProcessor::readDelayed (const std::vector<float>& buf,
                                    int writePos, float delaySamples)
{
    const int size = (int) buf.size();
    float readPos  = (float) writePos - delaySamples;
    while (readPos < 0.f)
        readPos += (float) size;

    const int   i0  = (int) readPos % size;
    const int   i1  = (i0 + 1)      % size;
    const float frac = readPos - std::floor (readPos);
    return buf[i0] + frac * (buf[i1] - buf[i0]);
}

void DelayProcessor::writeSample (std::vector<float>& buf, int& wPos, float x)
{
    buf[wPos] = x;
    wPos = (wPos + 1) % (int) buf.size();
}

float DelayProcessor::lpf (float x, float& state, float a)
{
    state = (1.f - a) * x + a * state;
    return state;
}

void DelayProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const ZoneResult zone = zoneKnob (dlyMix->load());

    if (! zone.isOn)
        return;  // dead zone — pass-through

    const float mix        = zone.param;
    const float timeMs     = dlyTime->load();
    const float sr         = (float) sampleRate;
    const float delaySamps = juce::jlimit (1.f, (float) delayBufL.size() - 2.f,
                                           timeMs * sr * 0.001f);

    // LPF coefficients: Analog = mild (~4 kHz), Tape = warm (~2 kHz)
    const float lpfCoeff   = (zone.type == 0) ? 0.25f : 0.50f;

    // Tape wow + flutter advance (per-block step at block-start)
    const float wowHz     = 0.3f;
    const float flutterHz = 8.0f;
    const float twoPi     = juce::MathConstants<float>::twoPi;

    const int numCh      = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int s = 0; s < numSamples; ++s)
    {
        // ── effective delay time (modified for tape) ─────────────────────
        float effectiveDelay = delaySamps;

        if (zone.type == 1)  // TAPE — wow + flutter
        {
            const float wow     = 0.012f * sr * std::sin (wowPhase);     // ±12 ms
            const float flutter = 0.002f * sr * std::sin (flutterPhase); // ±2 ms
            effectiveDelay = juce::jlimit (1.f,
                                           (float) delayBufL.size() - 2.f,
                                           delaySamps + wow + flutter);

            wowPhase     = std::fmod (wowPhase     + twoPi * wowHz     / sr, twoPi);
            flutterPhase = std::fmod (flutterPhase + twoPi * flutterHz / sr, twoPi);
        }

        const float dryL = buffer.getSample (0,                    s);
        const float dryR = (numCh > 1) ? buffer.getSample (1, s) : dryL;

        float wetL, wetR;

        if (zone.type == 2)  // DUAL (ping-pong)
        {
            // Cross-feedback: L delay fed by R feedback, R delay fed by L feedback
            wetL = readDelayed (delayBufL, writePosL, effectiveDelay);
            wetR = readDelayed (delayBufR, writePosR, effectiveDelay);

            writeSample (delayBufL, writePosL,
                         dryL + lpf (feedbackR * kFeedback, lpfStateL, lpfCoeff));
            writeSample (delayBufR, writePosR,
                         dryR + lpf (feedbackL * kFeedback, lpfStateR, lpfCoeff));

            feedbackL = wetL;
            feedbackR = wetR;
        }
        else  // ANALOG or TAPE (independent channels)
        {
            wetL = readDelayed (delayBufL, writePosL, effectiveDelay);
            wetR = readDelayed (delayBufR, writePosR, effectiveDelay);

            writeSample (delayBufL, writePosL,
                         dryL + lpf (wetL * kFeedback, lpfStateL, lpfCoeff));
            writeSample (delayBufR, writePosR,
                         dryR + lpf (wetR * kFeedback, lpfStateR, lpfCoeff));
        }

        buffer.setSample (0, s, dryL * (1.f - mix) + wetL * mix);
        if (numCh > 1)
            buffer.setSample (1, s, dryR * (1.f - mix) + wetR * mix);
    }
}
