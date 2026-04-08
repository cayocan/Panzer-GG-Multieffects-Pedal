#pragma once

// Maps a 0..1 knob value to a 3-zone result:
//   Zone 0  [0.000 – 0.308] : type 0, param normalised 0..1
//   Gap     [0.308 – 0.358] : OFF
//   Zone 1  [0.358 – 0.642] : type 1, param normalised 0..1
//   Gap     [0.642 – 0.692] : OFF
//   Zone 2  [0.692 – 1.000] : type 2, param normalised 0..1
//
// Gap width = 0.05 (5 % of full range) — matches hardware dead zone.

struct ZoneResult
{
    int   type;   // 0, 1, or 2
    float param;  // 0..1 within the active zone
    bool  isOn;   // false when knob is in a gap
};

inline ZoneResult zoneKnob (float v)
{
    constexpr float half_gap = 0.025f;  // ½ × 5 % gap
    constexpr float z        = 1.f / 3.f;

    const float b0 = z       - half_gap;   // 0.308
    const float a1 = z       + half_gap;   // 0.358
    const float b1 = 2.f * z - half_gap;   // 0.642
    const float a2 = 2.f * z + half_gap;   // 0.692

    if (v < b0)
        return { 0, juce::jmap (v, 0.f, b0, 0.f, 1.f), true  };
    if (v < a1)
        return { 0, 0.f,                                false };
    if (v < b1)
        return { 1, juce::jmap (v, a1, b1, 0.f, 1.f), true  };
    if (v < a2)
        return { 1, 0.f,                                false };

    return { 2, juce::jmap (v, a2, 1.f, 0.f, 1.f), true };
}
