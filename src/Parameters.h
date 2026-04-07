#pragma once
#include <JuceHeader.h>

namespace PanzerGGParameters
{
    namespace ID
    {
        static const juce::String gate       = "gate";
        static const juce::String ampType    = "ampType";
        static const juce::String gain       = "gain";
        static const juce::String treble     = "treble";
        static const juce::String middle     = "middle";
        static const juce::String bass       = "bass";
        static const juce::String ampVol     = "ampVol";
        static const juce::String modFx      = "modFx";
        static const juce::String modSpeed   = "modSpeed";
        static const juce::String dlyMix     = "dlyMix";
        static const juce::String dlyTime    = "dlyTime";
        static const juce::String rvbDecay   = "rvbDecay";
        static const juce::String rvbMix     = "rvbMix";
        static const juce::String irSlot     = "irSlot";
        static const juce::String master     = "master";
        static const juce::String liveMode   = "liveMode";
        static const juce::String bank       = "bank";
        static const juce::String slot       = "slot";
        static const juce::String tuner      = "tuner";
    }

    namespace Default
    {
        constexpr float gate      = -80.f;
        constexpr int   ampType   = 0;
        constexpr float gain      = 50.f;
        constexpr float treble    = 50.f;
        constexpr float middle    = 50.f;
        constexpr float bass      = 50.f;
        constexpr float ampVol    = 70.f;
        constexpr float modFx     = 0.31f;   // dead zone = MOD OFF by default
        constexpr float modSpeed  = 50.f;
        constexpr float dlyMix    = 0.31f;   // dead zone = DELAY OFF by default
        constexpr float dlyTime   = 350.f;
        constexpr float rvbDecay  = 0.31f;   // dead zone = REVERB OFF by default
        constexpr float rvbMix    = 25.f;
        constexpr int   irSlot    = 0;       // 0 = IR OFF
        constexpr float master    = 75.f;
        constexpr int   bank      = 0;
        constexpr int   slot      = -1;      // -1 = BYPASS
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using P = juce::AudioParameterFloat;
        using I = juce::AudioParameterInt;
        using B = juce::AudioParameterBool;

        juce::AudioProcessorValueTreeState::ParameterLayout params;

        // NOISE GATE — far-left (-80dB) = gate OFF
        params.add (std::make_unique<P> (
            ID::gate, "Noise Gate",
            juce::NormalisableRange<float> (-80.f, 0.f, 0.1f), Default::gate));

        // AMP MODULE
        params.add (std::make_unique<I> (ID::ampType, "AMP Type", 0, 8, Default::ampType));
        params.add (std::make_unique<P> (
            ID::gain, "Gain",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::gain));
        params.add (std::make_unique<P> (
            ID::treble, "Treble",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::treble));
        params.add (std::make_unique<P> (
            ID::middle, "Middle",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::middle));
        params.add (std::make_unique<P> (
            ID::bass, "Bass",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::bass));
        params.add (std::make_unique<P> (
            ID::ampVol, "AMP Volume",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::ampVol));

        // MOD — 3-zone knob
        // Zone 1 [0.0–0.305]: CHORUS mix
        // Dead zone [0.31–0.355]: MOD OFF
        // Zone 2 [0.36–0.63]: PHASER depth
        // Dead zone [0.63–0.685]: MOD OFF
        // Zone 3 [0.69–1.0]: TREMOLO mix
        params.add (std::make_unique<P> (
            ID::modFx, "MOD FX",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::modFx));
        params.add (std::make_unique<P> (
            ID::modSpeed, "MOD Speed",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::modSpeed));

        // DELAY — 3-zone knob
        // Zone 1: ANALOG | Dead: OFF | Zone 2: TAPE | Dead: OFF | Zone 3: DUAL
        params.add (std::make_unique<P> (
            ID::dlyMix, "DLY Mix",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::dlyMix));
        params.add (std::make_unique<P> (
            ID::dlyTime, "DLY Time",
            juce::NormalisableRange<float> (0.f, 1000.f, 1.f), Default::dlyTime));

        // REVERB — 3-zone knob
        // Zone 1: ROOM | Dead: OFF | Zone 2: SPRING | Dead: OFF | Zone 3: CLOUD
        params.add (std::make_unique<P> (
            ID::rvbDecay, "RVB Decay",
            juce::NormalisableRange<float> (0.f, 1.f, 0.001f), Default::rvbDecay));
        params.add (std::make_unique<P> (
            ID::rvbMix, "RVB Mix",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::rvbMix));

        // IR CAB — 0=OFF, 1–8=8 IR slots
        params.add (std::make_unique<I> (ID::irSlot, "IR CAB", 0, 8, Default::irSlot));

        // MASTER
        params.add (std::make_unique<P> (
            ID::master, "Master",
            juce::NormalisableRange<float> (0.f, 100.f, 0.1f), Default::master));

        // OPERATIONAL
        params.add (std::make_unique<B>  (ID::liveMode, "Live Mode", false));
        params.add (std::make_unique<I>  (ID::bank,     "Bank",      0, 8, Default::bank));
        // slot: -1=BYPASS total, 0-3=presets A/B/C/D
        params.add (std::make_unique<I>  (ID::slot,     "Slot",     -1, 3, Default::slot));
        params.add (std::make_unique<B>  (ID::tuner,    "Tuner",    false));

        return params;
    }

} // namespace PanzerGGParameters
