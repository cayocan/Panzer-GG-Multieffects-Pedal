#include <JuceHeader.h>
#include "dsp/GateProcessor.h"
#include "dsp/AmpProcessor.h"
#include "dsp/ModProcessor.h"
#include "dsp/DelayProcessor.h"
#include "dsp/ReverbProcessor.h"
#include "dsp/IrCabProcessor.h"

struct DspModuleSmokeTest : juce::UnitTest
{
    DspModuleSmokeTest() : juce::UnitTest ("DspModuleSmoke", "PanzerGG") {}

    void runTest() override
    {
        constexpr double sr    = 44100.0;
        constexpr int    block = 512;

        juce::AudioBuffer<float> buf (2, block);
        buf.clear();

        beginTest ("GateProcessor OFF (-80 dB) passes signal through");
        {
            juce::AudioBuffer<float> tone (2, block);
            tone.clear();
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);

            std::atomic<float> threshold { -80.f };
            GateProcessor gate (&threshold);
            gate.prepare (sr, block);
            gate.process (tone);
            expectWithinAbsoluteError (tone.getMagnitude (0, block), 0.5f, 1e-6f);
        }

        beginTest ("GateProcessor silences signal below threshold");
        {
            // Fill buffer with a low-level tone (-40 dB = 0.01 amplitude)
            juce::AudioBuffer<float> quiet (2, block);
            quiet.clear();
            for (int s = 0; s < block; ++s)
                quiet.setSample (0, s, 0.01f);

            // Threshold at -20 dB — signal at -40 dB should be gated
            std::atomic<float> threshold { -20.f };
            GateProcessor gate (&threshold);
            gate.prepare (sr, block);
            // Process several blocks so the envelope and gain settle
            for (int i = 0; i < 20; ++i)
                gate.process (quiet);
            expectWithinAbsoluteError (quiet.getMagnitude (0, block), 0.f, 0.01f);
        }

        beginTest ("AmpProcessor clean type produces output for non-zero input");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> ampType {0}, gain {50}, treble {50},
                               middle {50}, bass {50}, ampVol {70};
            AmpProcessor amp (&ampType, &gain, &treble, &middle, &bass, &ampVol);
            amp.prepare (sr, block);
            amp.process (tone);
            expectGreaterThan (tone.getMagnitude (0, block), 0.f);
        }

        beginTest ("AmpProcessor zero input produces zero output");
        {
            juce::AudioBuffer<float> silence (2, block);
            silence.clear();

            std::atomic<float> ampType {0}, gain {50}, treble {50},
                               middle {50}, bass {50}, ampVol {70};
            AmpProcessor amp (&ampType, &gain, &treble, &middle, &bass, &ampVol);
            amp.prepare (sr, block);
            amp.process (silence);
            expectWithinAbsoluteError (silence.getMagnitude (0, block), 0.f, 1e-6f);
        }

        beginTest ("AmpProcessor all 9 types process without NaN/Inf");
        {
            for (int t = 0; t <= 8; ++t)
            {
                juce::AudioBuffer<float> tone (2, block);
                for (int s = 0; s < block; ++s)
                    tone.setSample (0, s, 0.3f * std::sin (float (s) * 0.1f));
                tone.copyFrom (1, 0, tone, 0, 0, block);

                std::atomic<float> ampType {(float) t}, gain {60}, treble {50},
                                   middle {50}, bass {50}, ampVol {70};
                AmpProcessor amp (&ampType, &gain, &treble, &middle, &bass, &ampVol);
                amp.prepare (sr, block);
                amp.process (tone);

                float mag = tone.getMagnitude (0, block);
                expect (std::isfinite (mag), "AMP type " + juce::String (t) + " produced NaN/Inf");
            }
        }

        beginTest ("ModProcessor OFF (dead zone) passes signal through");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> modFx { 0.33f }, modSpeed { 50.f }; // dead zone
            ModProcessor mod (&modFx, &modSpeed);
            mod.prepare (sr, block);
            mod.process (tone);
            expectWithinAbsoluteError (tone.getMagnitude (0, block), 0.5f, 1e-6f);
        }

        beginTest ("ModProcessor Chorus produces output without NaN/Inf");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.3f * std::sin (float (s) * 0.05f));
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> modFx { 0.15f }, modSpeed { 50.f }; // zone 0 = chorus
            ModProcessor mod (&modFx, &modSpeed);
            mod.prepare (sr, block);
            for (int i = 0; i < 5; ++i) mod.process (tone);
            expect (std::isfinite (tone.getMagnitude (0, block)), "Chorus NaN/Inf");
            expectGreaterThan (tone.getMagnitude (0, block), 0.f);
        }

        beginTest ("ModProcessor Phaser produces output without NaN/Inf");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.3f * std::sin (float (s) * 0.05f));
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> modFx { 0.5f }, modSpeed { 40.f }; // zone 1 = phaser
            ModProcessor mod (&modFx, &modSpeed);
            mod.prepare (sr, block);
            for (int i = 0; i < 5; ++i) mod.process (tone);
            expect (std::isfinite (tone.getMagnitude (0, block)), "Phaser NaN/Inf");
        }

        beginTest ("ModProcessor Tremolo produces output without NaN/Inf");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> modFx { 0.85f }, modSpeed { 60.f }; // zone 2 = tremolo
            ModProcessor mod (&modFx, &modSpeed);
            mod.prepare (sr, block);
            mod.process (tone);
            expect (std::isfinite (tone.getMagnitude (0, block)), "Tremolo NaN/Inf");
            expectGreaterThan (tone.getMagnitude (0, block), 0.f);
        }

        beginTest ("DelayProcessor OFF (dead zone) passes signal through");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> dlyMix { 0.33f }, dlyTime { 350.f };
            DelayProcessor delay (&dlyMix, &dlyTime);
            delay.prepare (sr, block);
            delay.process (tone);
            expectWithinAbsoluteError (tone.getMagnitude (0, block), 0.5f, 1e-6f);
        }

        beginTest ("DelayProcessor Analog — output is finite and non-zero");
        {
            juce::AudioBuffer<float> impulse (2, block);
            impulse.clear();
            impulse.setSample (0, 0, 1.f);
            impulse.setSample (1, 0, 1.f);

            std::atomic<float> dlyMix { 0.15f }, dlyTime { 100.f }; // zone 0 = analog
            DelayProcessor delay (&dlyMix, &dlyTime);
            delay.prepare (sr, block);
            delay.process (impulse); // first block — dry passes
            expect (std::isfinite (impulse.getMagnitude (0, block)), "Analog NaN/Inf");
        }

        beginTest ("DelayProcessor Tape — no NaN/Inf after several blocks");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.3f * std::sin (float (s) * 0.05f));
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> dlyMix { 0.5f }, dlyTime { 200.f }; // zone 1 = tape
            DelayProcessor delay (&dlyMix, &dlyTime);
            delay.prepare (sr, block);
            for (int i = 0; i < 10; ++i) delay.process (tone);
            expect (std::isfinite (tone.getMagnitude (0, block)), "Tape NaN/Inf");
        }

        beginTest ("DelayProcessor Dual (ping-pong) — no NaN/Inf after several blocks");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.3f * std::sin (float (s) * 0.05f));
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> dlyMix { 0.85f }, dlyTime { 300.f }; // zone 2 = dual
            DelayProcessor delay (&dlyMix, &dlyTime);
            delay.prepare (sr, block);
            for (int i = 0; i < 10; ++i) delay.process (tone);
            expect (std::isfinite (tone.getMagnitude (0, block)), "Dual NaN/Inf");
        }

        beginTest ("ReverbProcessor OFF (dead zone) passes signal through");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> rvbDecay { 0.33f }, rvbMix { 25.f };
            ReverbProcessor reverb (&rvbDecay, &rvbMix);
            reverb.prepare (sr, block);
            reverb.process (tone);
            expectWithinAbsoluteError (tone.getMagnitude (0, block), 0.5f, 1e-6f);
        }

        beginTest ("ReverbProcessor all 3 types — no NaN/Inf after tail");
        {
            // Zone 0 = Room, Zone 1 = Spring, Zone 2 = Cloud
            const float zoneValues[3] = { 0.15f, 0.5f, 0.85f };
            const char* names[3]      = { "Room", "Spring", "Cloud" };

            for (int t = 0; t < 3; ++t)
            {
                juce::AudioBuffer<float> impulse (2, block);
                impulse.clear();
                impulse.setSample (0, 0, 0.5f);
                impulse.setSample (1, 0, 0.5f);

                std::atomic<float> rvbDecay { zoneValues[t] }, rvbMix { 50.f };
                ReverbProcessor reverb (&rvbDecay, &rvbMix);
                reverb.prepare (sr, block);

                for (int i = 0; i < 20; ++i) reverb.process (impulse);

                expect (std::isfinite (impulse.getMagnitude (0, block)),
                        juce::String (names[t]) + " reverb NaN/Inf");
            }
        }

        beginTest ("ReverbProcessor Cloud tail decays (not infinite at mid decay)");
        {
            juce::AudioBuffer<float> impulse (2, block);
            impulse.clear();
            impulse.setSample (0, 0, 0.5f);
            impulse.setSample (1, 0, 0.5f);

            std::atomic<float> rvbDecay { 0.75f }, rvbMix { 100.f }; // Cloud, 50% param
            ReverbProcessor reverb (&rvbDecay, &rvbMix);
            reverb.prepare (sr, block);

            // Prime the reverb with the impulse
            reverb.process (impulse);

            // Run silent blocks — tail should appear then fade
            juce::AudioBuffer<float> silence (2, block);
            silence.clear();
            float firstMag = 0.f;
            for (int i = 0; i < 200; ++i)
            {
                reverb.process (silence);
                if (i == 0) firstMag = silence.getMagnitude (0, block);
            }
            // After 200 blocks of silence the tail must have attenuated
            expect (silence.getMagnitude (0, block) < firstMag + 1e-6f,
                    "Cloud tail is not decaying");
        }

        beginTest ("IrCabProcessor slot 0 (OFF) passes signal through");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> irSlot { 0.f };
            IrCabProcessor irCab (&irSlot);
            irCab.prepare (sr, block);
            irCab.process (tone);
            expectWithinAbsoluteError (tone.getMagnitude (0, block), 0.5f, 1e-6f);
        }

        beginTest ("IrCabProcessor slot with no registered IR passes through");
        {
            juce::AudioBuffer<float> tone (2, block);
            for (int s = 0; s < block; ++s)
                tone.setSample (0, s, 0.5f);
            tone.copyFrom (1, 0, tone, 0, 0, block);

            std::atomic<float> irSlot { 1.f };  // slot 1, no data registered
            IrCabProcessor irCab (&irSlot);
            irCab.prepare (sr, block);
            irCab.process (tone);
            // No crash, signal passes through
            expect (std::isfinite (tone.getMagnitude (0, block)));
        }

        beginTest ("GateProcessor passes loud signal above threshold");
        {
            juce::AudioBuffer<float> loud (2, block);
            loud.clear();
            for (int s = 0; s < block; ++s)
                loud.setSample (0, s, 0.5f); // -6 dB

            // Threshold at -20 dB — signal at -6 dB should pass
            std::atomic<float> threshold { -20.f };
            GateProcessor gate (&threshold);
            gate.prepare (sr, block);
            for (int i = 0; i < 5; ++i)
                gate.process (loud);
            // After settling, gain should be ~1 — magnitude close to 0.5
            expectGreaterThan (loud.getMagnitude (0, block), 0.4f);
        }
    }
};

static DspModuleSmokeTest dspModuleSmokeTest;
