#pragma once
#include <JuceHeader.h>

// ModeManager — pure state machine for PRESET / LIVE operation.
//
// PRESET mode:
//   slot == -1          → total bypass (all modules skipped)
//   slot == 0..3        → run full chain with current APVTS parameter values
//   footswitch A/B/C/D  → select slot 0/1/2/3 in current bank
//   footswitch BANK-/+  → navigate bank 0..8
//
// LIVE mode:
//   footswitch A → toggle reverbOn
//   footswitch B → toggle delayOn
//   footswitch C → toggle modOn
//   footswitch D → toggle ampOn
//   hold A/B/C/D → save current state to slot (LED blink = confirmation)
//   B+C tap      → handled by toggleMode()   (called by editor)
//   B+C 2 s      → tuner toggle              (called by editor timer)
//
// processBlock() queries isTotalBypass() and isXxxOn() — no locks needed
// because all writes happen on the message thread and reads are atomic.

class ModeManager
{
public:
    enum class Mode { Preset, Live };

    // ── Footswitch IDs ────────────────────────────────────────────────────
    enum Footswitch { A = 0, B = 1, C = 2, D = 3, BankMinus = 4, BankPlus = 5 };

    // ── Construction ──────────────────────────────────────────────────────
    ModeManager() = default;

    // ── Mode toggle (B+C tap) ─────────────────────────────────────────────
    void toggleMode();

    // ── Footswitch handler — call from message thread ─────────────────────
    // held: true when the button was held ≥ 2 s (save-preset gesture in LIVE)
    // Returns true if a preset-save gesture was detected (caller blinks LED).
    bool handleFootswitch (Footswitch fs, bool held = false);

    // ── Tuner (B+C 2 s) ───────────────────────────────────────────────────
    void setTuner (bool active) noexcept { tunerActive.store (active); }

    // ── State queries (safe to call from audio thread) ────────────────────
    bool isTotalBypass() const noexcept;
    bool isAmpOn()       const noexcept;
    bool isModOn()       const noexcept;
    bool isDelayOn()     const noexcept;
    bool isReverbOn()    const noexcept;
    bool isTunerActive() const noexcept { return tunerActive.load(); }

    // ── APVTS sync ────────────────────────────────────────────────────────
    // Call after loading state information to restore mode from saved XML.
    void syncFromApvts (juce::AudioProcessorValueTreeState& apvts);
    // Call before saving state information to persist mode to XML.
    void syncToApvts   (juce::AudioProcessorValueTreeState& apvts) const;

    // ── Accessors (message thread) ────────────────────────────────────────
    Mode getMode()  const noexcept { return mode; }
    int  getBank()  const noexcept { return bank; }
    int  getSlot()  const noexcept { return slot; }

private:
    // All written on the message thread; read on both threads via atomics.
    Mode mode { Mode::Preset };

    std::atomic<int>  bank      { 0 };   // 0–8
    std::atomic<int>  slot      { -1 };  // -1=bypass, 0-3=A/B/C/D

    std::atomic<bool> ampOn     { true  };
    std::atomic<bool> modOn     { true  };
    std::atomic<bool> delayOn   { true  };
    std::atomic<bool> reverbOn  { true  };
    std::atomic<bool> tunerActive { false };

    void handlePresetFootswitch (Footswitch fs);
    bool handleLiveFootswitch   (Footswitch fs, bool held);
};
