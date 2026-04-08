#include "IrCabProcessor.h"

IrCabProcessor::IrCabProcessor (std::atomic<float>* irSlot)
    : irSlot (irSlot)
{
}

void IrCabProcessor::registerSlotData (int slot, const void* data, std::size_t size)
{
    jassert (slot >= 1 && slot <= 8);
    slots[slot] = { data, size };
}

void IrCabProcessor::prepare (double sr, int blockSz)
{
    sampleRate = sr;
    blockSize  = blockSz;
    cachedSlot = -1;   // force reload on first process()
    irLoaded   = false;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sr;
    spec.maximumBlockSize = (juce::uint32) blockSz;
    spec.numChannels      = 2;
    convolution.prepare (spec);
}

void IrCabProcessor::reset()
{
    convolution.reset();
    irLoaded = false;
}

void IrCabProcessor::loadSlot (int slot)
{
    irLoaded = false;

    if (slot < 1 || slot > 8)
        return;

    const auto& entry = slots[slot];
    if (entry.data == nullptr || entry.size == 0)
        return;   // no IR registered for this slot → pass-through

    convolution.loadImpulseResponse (
        entry.data,
        entry.size,
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        0,                                      // use full IR length
        juce::dsp::Convolution::Normalise::yes
    );
    irLoaded = true;
}

void IrCabProcessor::process (juce::AudioBuffer<float>& buffer)
{
    const int slot = static_cast<int> (irSlot->load());

    if (slot == 0)
        return;   // OFF — pass-through

    // Reload if slot changed
    if (slot != cachedSlot)
    {
        cachedSlot = slot;
        loadSlot (slot);
    }

    if (! irLoaded)
        return;   // slot has no registered IR — pass-through

    juce::dsp::AudioBlock<float>              block  (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    convolution.process (context);
}
