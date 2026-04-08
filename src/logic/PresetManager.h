#pragma once
#include <JuceHeader.h>

// PresetManager — 36 presets organised as 9 banks × 4 slots (A/B/C/D).
//
// Slots are initially loaded from factory defaults.
// Users can overwrite any slot via savePreset() and restore via resetToFactory().
// The full preset bank is serialised/deserialised as a child XML element inside
// the plugin's getStateInformation / setStateInformation XML blob.

struct Preset
{
    const char* name = "Init";

    // AMP
    int   ampType = 0;
    float gain    = 50.f, treble = 50.f, middle = 50.f, bass = 50.f, ampVol = 70.f;

    // MOD  (zone knob 0-1: ~0.15=Chorus, ~0.33=OFF, ~0.5=Phaser, ~0.85=Tremolo)
    float modFx = 0.33f, modSpeed = 50.f;

    // DELAY (zone knob: ~0.15=Analog, ~0.33=OFF, ~0.5=Tape, ~0.85=Dual)
    float dlyMix = 0.33f, dlyTime = 350.f;

    // REVERB (zone knob: ~0.15=Room, ~0.33=OFF, ~0.5=Spring, ~0.85=Cloud)
    float rvbDecay = 0.33f, rvbMix = 25.f;

    // CAB / MASTER / GATE
    int   irSlot = 0;
    float master = 75.f;
    float gate   = -80.f;   // -80 = gate OFF
};

class PresetManager
{
public:
    static constexpr int kBanks = 9;
    static constexpr int kSlots = 4;

    PresetManager();

    // ── Load / Save / Reset ───────────────────────────────────────────────
    void loadPreset   (int bank, int slot, juce::AudioProcessorValueTreeState& apvts) const;
    void savePreset   (int bank, int slot, juce::AudioProcessorValueTreeState& apvts);
    void resetToFactory (int bank, int slot);
    void resetAllToFactory();

    // ── Metadata ──────────────────────────────────────────────────────────
    const char* getPresetName (int bank, int slot) const;
    bool        isModified    (int bank, int slot) const;

    // ── Serialisation ─────────────────────────────────────────────────────
    std::unique_ptr<juce::XmlElement> createXml() const;
    void loadFromXml (const juce::XmlElement& xml);

private:
    Preset presets[kBanks][kSlots];
    bool   modified[kBanks][kSlots] = {};

    static const Preset kFactory[kBanks][kSlots];

    static void applyPreset (const Preset& p, juce::AudioProcessorValueTreeState& apvts);
    static Preset capturePreset (const char* name, juce::AudioProcessorValueTreeState& apvts);
};
