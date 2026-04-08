#include "PresetManager.h"
#include "Parameters.h"

using namespace PanzerGGParameters;

// ── Factory presets ───────────────────────────────────────────────────────────
// Layout: { name, ampType, gain, treble, middle, bass, ampVol,
//           modFx, modSpeed, dlyMix, dlyTime, rvbDecay, rvbMix,
//           irSlot, master, gate }
// Zone knob OFF ≈ 0.33 | Zone0 ≈ 0.15 | Zone1 ≈ 0.50 | Zone2 ≈ 0.85

const Preset PresetManager::kFactory[kBanks][kSlots] =
{
    // ── Bank 0: CLEAN ────────────────────────────────────────────────────
    {
        { "Clean Bright",  0, 28, 65, 50, 45, 75,  0.33f, 50, 0.33f, 350, 0.15f, 20, 1, 75, -80 },
        { "Clean Warm",    0, 25, 42, 55, 62, 75,  0.15f, 40, 0.33f, 350, 0.15f, 30, 2, 75, -80 },
        { "Clean Jazz",    0, 20, 38, 50, 68, 80,  0.33f, 50, 0.33f, 350, 0.50f, 25, 4, 75, -80 },
        { "Clean Ambient", 0, 25, 50, 45, 55, 70,  0.15f, 35, 0.33f, 450, 0.85f, 40, 1, 72, -80 },
    },
    // ── Bank 1: BLUES ────────────────────────────────────────────────────
    {
        { "Blues Edge",    1, 55, 62, 55, 50, 72,  0.33f, 50, 0.15f, 380, 0.15f, 22, 1, 75, -45 },
        { "Blues Crunch",  2, 60, 65, 50, 46, 70,  0.33f, 50, 0.33f, 350, 0.15f, 18, 3, 75, -50 },
        { "Blues Lead",    1, 68, 55, 62, 50, 68,  0.33f, 50, 0.15f, 420, 0.15f, 20, 1, 73, -48 },
        { "SRV Style",     1, 72, 60, 55, 56, 70,  0.33f, 50, 0.15f, 360, 0.50f, 28, 3, 74, -45 },
    },
    // ── Bank 2: CRUNCH ───────────────────────────────────────────────────
    {
        { "Crunch I",      2, 62, 60, 52, 48, 70,  0.33f, 50, 0.33f, 350, 0.15f, 20, 1, 75, -50 },
        { "Crunch II",     2, 68, 65, 50, 45, 70,  0.33f, 50, 0.15f, 400, 0.15f, 18, 3, 75, -52 },
        { "Power Crunch",  3, 70, 58, 55, 50, 72,  0.33f, 50, 0.33f, 350, 0.15f, 15, 5, 73, -55 },
        { "British Crunch",5, 65, 62, 48, 52, 70,  0.33f, 50, 0.15f, 380, 0.15f, 22, 3, 74, -50 },
    },
    // ── Bank 3: LEAD ─────────────────────────────────────────────────────
    {
        { "Classic Lead",  3, 75, 58, 60, 48, 68,  0.33f, 50, 0.15f, 380, 0.15f, 20, 5, 73, -55 },
        { "Smooth Lead",   1, 70, 55, 58, 52, 70,  0.33f, 50, 0.15f, 420, 0.50f, 30, 2, 72, -52 },
        { "Screaming Ld",  3, 82, 62, 55, 45, 68,  0.33f, 50, 0.15f, 350, 0.15f, 15, 5, 72, -58 },
        { "British Lead",  5, 78, 65, 50, 48, 70,  0.33f, 50, 0.15f, 400, 0.15f, 18, 3, 73, -55 },
    },
    // ── Bank 4: METAL ────────────────────────────────────────────────────
    {
        { "Modern Metal",  4, 85, 68, 40, 52, 70,  0.33f, 50, 0.33f, 350, 0.15f, 12, 5, 73, -60 },
        { "Tight Metal",   4, 90, 65, 38, 55, 68,  0.33f, 50, 0.33f, 350, 0.33f,  0, 6, 72, -62 },
        { "Death Metal",   4, 95, 70, 35, 58, 65,  0.33f, 50, 0.33f, 350, 0.15f, 10, 7, 70, -65 },
        { "Nu Metal",      4, 88, 60, 42, 60, 70,  0.33f, 50, 0.15f, 300, 0.15f, 12, 6, 73, -60 },
    },
    // ── Bank 5: AMBIENT ──────────────────────────────────────────────────
    {
        { "Shoegaze",      0, 30, 55, 45, 50, 68,  0.15f, 30, 0.85f, 500, 0.85f, 50, 1, 70, -80 },
        { "Post-Rock",     0, 25, 52, 50, 48, 70,  0.15f, 25, 0.50f, 600, 0.85f, 55, 1, 70, -80 },
        { "Dream Pop",     0, 28, 58, 48, 45, 68,  0.15f, 32, 0.85f, 480, 0.85f, 60, 2, 68, -80 },
        { "Dark Ambient",  7, 35, 42, 52, 58, 65,  0.33f, 50, 0.50f, 700, 0.85f, 65, 7, 68, -80 },
    },
    // ── Bank 6: BOUTIQUE ─────────────────────────────────────────────────
    {
        { "Boutique Cln",  7, 28, 58, 52, 50, 75,  0.33f, 50, 0.33f, 350, 0.50f, 25, 2, 75, -80 },
        { "Boutique OD",   7, 58, 60, 55, 48, 72,  0.33f, 50, 0.15f, 400, 0.15f, 20, 3, 74, -52 },
        { "Tweed Style",   6, 62, 65, 52, 55, 70,  0.33f, 50, 0.15f, 380, 0.50f, 28, 1, 73, -50 },
        { "Blackface",     6, 35, 68, 48, 52, 75,  0.15f, 42, 0.33f, 350, 0.15f, 22, 2, 75, -80 },
    },
    // ── Bank 7: FUZZ ─────────────────────────────────────────────────────
    {
        { "Classic Fuzz",  8, 80, 60, 50, 50, 68,  0.33f, 50, 0.33f, 350, 0.15f, 15, 5, 72, -55 },
        { "Velcro Fuzz",   8, 90, 65, 45, 52, 65,  0.33f, 50, 0.33f, 350, 0.33f,  0, 6, 70, -58 },
        { "Octave Fuzz",   8, 88, 70, 42, 48, 65,  0.85f, 60, 0.33f, 350, 0.15f, 12, 5, 70, -60 },
        { "Fuzz Face",     8, 85, 58, 52, 54, 68,  0.33f, 50, 0.15f, 360, 0.50f, 20, 3, 72, -55 },
    },
    // ── Bank 8: CUSTOM (init patches) ────────────────────────────────────
    {
        { "Init Clean",    0, 50, 50, 50, 50, 70,  0.33f, 50, 0.33f, 350, 0.33f, 25, 0, 75, -80 },
        { "Init Lead",     3, 65, 55, 55, 50, 70,  0.33f, 50, 0.33f, 350, 0.33f, 25, 0, 75, -55 },
        { "Init Metal",    4, 80, 65, 40, 55, 68,  0.33f, 50, 0.33f, 350, 0.33f, 15, 0, 73, -60 },
        { "Init Ambient",  0, 25, 52, 48, 50, 68,  0.15f, 35, 0.15f, 450, 0.85f, 45, 0, 70, -80 },
    },
};

// ── Construction ─────────────────────────────────────────────────────────────

PresetManager::PresetManager()
{
    resetAllToFactory();
}

// ── Factory reset ─────────────────────────────────────────────────────────────

void PresetManager::resetAllToFactory()
{
    for (int b = 0; b < kBanks; ++b)
        for (int s = 0; s < kSlots; ++s)
            resetToFactory (b, s);
}

void PresetManager::resetToFactory (int bank, int slot)
{
    jassert (bank >= 0 && bank < kBanks);
    jassert (slot >= 0 && slot < kSlots);
    presets[bank][slot]  = kFactory[bank][slot];
    modified[bank][slot] = false;
}

// ── Load / Save ───────────────────────────────────────────────────────────────

void PresetManager::loadPreset (int bank, int slot,
                                 juce::AudioProcessorValueTreeState& apvts) const
{
    jassert (bank >= 0 && bank < kBanks);
    jassert (slot >= 0 && slot < kSlots);
    applyPreset (presets[bank][slot], apvts);
}

void PresetManager::savePreset (int bank, int slot,
                                 juce::AudioProcessorValueTreeState& apvts)
{
    jassert (bank >= 0 && bank < kBanks);
    jassert (slot >= 0 && slot < kSlots);
    const char* name = presets[bank][slot].name;
    presets[bank][slot]  = capturePreset (name, apvts);
    modified[bank][slot] = true;
}

// ── Metadata ─────────────────────────────────────────────────────────────────

const char* PresetManager::getPresetName (int bank, int slot) const
{
    return presets[bank][slot].name;
}

bool PresetManager::isModified (int bank, int slot) const
{
    return modified[bank][slot];
}

// ── Apply / Capture helpers ───────────────────────────────────────────────────

void PresetManager::applyPreset (const Preset& p,
                                  juce::AudioProcessorValueTreeState& apvts)
{
    auto setFloat = [&] (const juce::String& id, float v)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (v));
    };
    auto setInt = [&] (const juce::String& id, int v)
    {
        if (auto* param = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (id)))
            param->setValueNotifyingHost (param->convertTo0to1 (v));
    };

    setInt   (ID::ampType,  p.ampType);
    setFloat (ID::gain,     p.gain);
    setFloat (ID::treble,   p.treble);
    setFloat (ID::middle,   p.middle);
    setFloat (ID::bass,     p.bass);
    setFloat (ID::ampVol,   p.ampVol);
    setFloat (ID::modFx,    p.modFx);
    setFloat (ID::modSpeed, p.modSpeed);
    setFloat (ID::dlyMix,   p.dlyMix);
    setFloat (ID::dlyTime,  p.dlyTime);
    setFloat (ID::rvbDecay, p.rvbDecay);
    setFloat (ID::rvbMix,   p.rvbMix);
    setInt   (ID::irSlot,   p.irSlot);
    setFloat (ID::master,   p.master);
    setFloat (ID::gate,     p.gate);
}

Preset PresetManager::capturePreset (const char* name,
                                      juce::AudioProcessorValueTreeState& apvts)
{
    auto getF = [&] (const juce::String& id) -> float
    {
        return apvts.getRawParameterValue (id)->load();
    };
    auto getI = [&] (const juce::String& id) -> int
    {
        return static_cast<int> (apvts.getRawParameterValue (id)->load());
    };

    Preset p;
    p.name     = name;
    p.ampType  = getI (ID::ampType);
    p.gain     = getF (ID::gain);
    p.treble   = getF (ID::treble);
    p.middle   = getF (ID::middle);
    p.bass     = getF (ID::bass);
    p.ampVol   = getF (ID::ampVol);
    p.modFx    = getF (ID::modFx);
    p.modSpeed = getF (ID::modSpeed);
    p.dlyMix   = getF (ID::dlyMix);
    p.dlyTime  = getF (ID::dlyTime);
    p.rvbDecay = getF (ID::rvbDecay);
    p.rvbMix   = getF (ID::rvbMix);
    p.irSlot   = getI (ID::irSlot);
    p.master   = getF (ID::master);
    p.gate     = getF (ID::gate);
    return p;
}

// ── Serialisation ─────────────────────────────────────────────────────────────

std::unique_ptr<juce::XmlElement> PresetManager::createXml() const
{
    auto root = std::make_unique<juce::XmlElement> ("PresetBank");

    for (int b = 0; b < kBanks; ++b)
    {
        for (int s = 0; s < kSlots; ++s)
        {
            const auto& p = presets[b][s];
            auto* e = root->createNewChildElement ("Preset");
            e->setAttribute ("bank",     b);
            e->setAttribute ("slot",     s);
            e->setAttribute ("name",     p.name);
            e->setAttribute ("modified", modified[b][s]);
            e->setAttribute ("ampType",  p.ampType);
            e->setAttribute ("gain",     p.gain);
            e->setAttribute ("treble",   p.treble);
            e->setAttribute ("middle",   p.middle);
            e->setAttribute ("bass",     p.bass);
            e->setAttribute ("ampVol",   p.ampVol);
            e->setAttribute ("modFx",    p.modFx);
            e->setAttribute ("modSpeed", p.modSpeed);
            e->setAttribute ("dlyMix",   p.dlyMix);
            e->setAttribute ("dlyTime",  p.dlyTime);
            e->setAttribute ("rvbDecay", p.rvbDecay);
            e->setAttribute ("rvbMix",   p.rvbMix);
            e->setAttribute ("irSlot",   p.irSlot);
            e->setAttribute ("master",   p.master);
            e->setAttribute ("gate",     p.gate);
        }
    }
    return root;
}

void PresetManager::loadFromXml (const juce::XmlElement& xml)
{
    if (! xml.hasTagName ("PresetBank")) return;

    for (auto* e : xml.getChildIterator())
    {
        const int b = e->getIntAttribute ("bank", -1);
        const int s = e->getIntAttribute ("slot", -1);
        if (b < 0 || b >= kBanks || s < 0 || s >= kSlots) continue;

        Preset& p      = presets[b][s];
        // name is stored as a String; we keep the pointer from factory for
        // unmodified presets and use a persistent string for user presets.
        // For simplicity, we reuse the factory name pointer if unmodified.
        modified[b][s] = e->getBoolAttribute ("modified", false);

        p.ampType  = e->getIntAttribute    ("ampType",  kFactory[b][s].ampType);
        p.gain     = (float) e->getDoubleAttribute ("gain",     kFactory[b][s].gain);
        p.treble   = (float) e->getDoubleAttribute ("treble",   kFactory[b][s].treble);
        p.middle   = (float) e->getDoubleAttribute ("middle",   kFactory[b][s].middle);
        p.bass     = (float) e->getDoubleAttribute ("bass",     kFactory[b][s].bass);
        p.ampVol   = (float) e->getDoubleAttribute ("ampVol",   kFactory[b][s].ampVol);
        p.modFx    = (float) e->getDoubleAttribute ("modFx",    kFactory[b][s].modFx);
        p.modSpeed = (float) e->getDoubleAttribute ("modSpeed", kFactory[b][s].modSpeed);
        p.dlyMix   = (float) e->getDoubleAttribute ("dlyMix",   kFactory[b][s].dlyMix);
        p.dlyTime  = (float) e->getDoubleAttribute ("dlyTime",  kFactory[b][s].dlyTime);
        p.rvbDecay = (float) e->getDoubleAttribute ("rvbDecay", kFactory[b][s].rvbDecay);
        p.rvbMix   = (float) e->getDoubleAttribute ("rvbMix",   kFactory[b][s].rvbMix);
        p.irSlot   = e->getIntAttribute    ("irSlot",   kFactory[b][s].irSlot);
        p.master   = (float) e->getDoubleAttribute ("master",   kFactory[b][s].master);
        p.gate     = (float) e->getDoubleAttribute ("gate",     kFactory[b][s].gate);
        p.name     = modified[b][s] ? kFactory[b][s].name : kFactory[b][s].name;
    }
}
