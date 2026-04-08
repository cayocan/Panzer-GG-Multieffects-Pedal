#pragma once
#include "DspModule.h"

// IrCabProcessor — convolution cabinet simulator.
//   irSlot == 0          : IR OFF (pass-through, zero overhead)
//   irSlot == 1..8       : load and convolve with the corresponding IR
//
// IR data is registered per slot via registerSlotData() before playback.
// The PluginProcessor calls this once at construction using BinaryData pointers.
// Slot changes are detected in process() and trigger an async reload.

class IrCabProcessor : public DspModule
{
public:
    explicit IrCabProcessor (std::atomic<float>* irSlot);

    void prepare (double sampleRate, int samplesPerBlock) override;
    void reset() override;
    void process (juce::AudioBuffer<float>& buffer) override;

    // Register raw WAV bytes for slot 1–8.
    // data must remain valid for the lifetime of this object (e.g. BinaryData).
    void registerSlotData (int slot, const void* data, std::size_t sizeBytes);

private:
    std::atomic<float>* irSlot;

    double sampleRate    = 44100.0;
    int    blockSize     = 512;
    int    cachedSlot    = -1;
    bool   irLoaded      = false;

    juce::dsp::Convolution convolution { juce::dsp::Convolution::NonUniform { 512 } };

    static constexpr int kNumSlots = 9;          // index 0 unused
    struct SlotEntry { const void* data = nullptr; std::size_t size = 0; };
    SlotEntry slots[kNumSlots];

    void loadSlot (int slot);
};
