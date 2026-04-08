#include "ModeManager.h"
#include "Parameters.h"

// ── Mode toggle ───────────────────────────────────────────────────────────────

void ModeManager::toggleMode()
{
    mode = (mode == Mode::Preset) ? Mode::Live : Mode::Preset;
}

// ── Footswitch dispatch ───────────────────────────────────────────────────────

bool ModeManager::handleFootswitch (Footswitch fs, bool held)
{
    if (mode == Mode::Preset)
    {
        handlePresetFootswitch (fs);
        return false;
    }
    return handleLiveFootswitch (fs, held);
}

// ── PRESET mode ───────────────────────────────────────────────────────────────

void ModeManager::handlePresetFootswitch (Footswitch fs)
{
    switch (fs)
    {
        case A: slot.store (0); break;
        case B: slot.store (1); break;
        case C: slot.store (2); break;
        case D: slot.store (3); break;

        case BankMinus:
            bank.store (juce::jmax (0, bank.load() - 1));
            slot.store (-1);   // deselect — user must tap A/B/C/D to pick preset
            break;

        case BankPlus:
            bank.store (juce::jmin (8, bank.load() + 1));
            slot.store (-1);
            break;
    }
}

// ── LIVE mode ─────────────────────────────────────────────────────────────────

bool ModeManager::handleLiveFootswitch (Footswitch fs, bool held)
{
    if (held)
    {
        // Hold gesture = save current state to slot (caller handles LED blink)
        switch (fs)
        {
            case A: slot.store (0); break;
            case B: slot.store (1); break;
            case C: slot.store (2); break;
            case D: slot.store (3); break;
            default: break;
        }
        return true;   // save-preset detected
    }

    // Tap gesture = toggle individual module
    switch (fs)
    {
        case A: reverbOn.store (! reverbOn.load()); break;
        case B: delayOn .store (! delayOn .load()); break;
        case C: modOn   .store (! modOn   .load()); break;
        case D: ampOn   .store (! ampOn   .load()); break;
        default: break;
    }
    return false;
}

// ── Audio-thread queries ──────────────────────────────────────────────────────

bool ModeManager::isTotalBypass() const noexcept
{
    if (tunerActive.load()) return true;
    if (mode == Mode::Preset && slot.load() == -1) return true;
    return false;
}

bool ModeManager::isAmpOn() const noexcept
{
    return (mode == Mode::Preset) || ampOn.load();
}

bool ModeManager::isModOn() const noexcept
{
    return (mode == Mode::Preset) || modOn.load();
}

bool ModeManager::isDelayOn() const noexcept
{
    return (mode == Mode::Preset) || delayOn.load();
}

bool ModeManager::isReverbOn() const noexcept
{
    return (mode == Mode::Preset) || reverbOn.load();
}

// ── APVTS sync ────────────────────────────────────────────────────────────────

void ModeManager::syncFromApvts (juce::AudioProcessorValueTreeState& apvts)
{
    const bool live = apvts.getRawParameterValue (PanzerGGParameters::ID::liveMode)->load() > 0.5f;
    mode = live ? Mode::Live : Mode::Preset;
    bank.store (static_cast<int> (apvts.getRawParameterValue (PanzerGGParameters::ID::bank)->load()));
    slot.store (static_cast<int> (apvts.getRawParameterValue (PanzerGGParameters::ID::slot)->load()));
}

void ModeManager::syncToApvts (juce::AudioProcessorValueTreeState& apvts) const
{
    if (auto* p = apvts.getParameter (PanzerGGParameters::ID::liveMode))
        p->setValueNotifyingHost (mode == Mode::Live ? 1.f : 0.f);

    if (auto* p = dynamic_cast<juce::AudioParameterInt*> (
                      apvts.getParameter (PanzerGGParameters::ID::bank)))
        p->setValueNotifyingHost (p->convertTo0to1 (bank.load()));

    if (auto* p = dynamic_cast<juce::AudioParameterInt*> (
                      apvts.getParameter (PanzerGGParameters::ID::slot)))
        p->setValueNotifyingHost (p->convertTo0to1 (slot.load()));
}
