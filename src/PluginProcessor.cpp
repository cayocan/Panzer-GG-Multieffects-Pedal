#include "PluginProcessor.h"
#include <BinaryData.h>

using namespace PanzerGGParameters;

PanzerGGProcessor::PanzerGGProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this, nullptr, "Parameters", createLayout())
    , gate   (apvts.getRawParameterValue (ID::gate))
    , amp    (apvts.getRawParameterValue (ID::ampType),
              apvts.getRawParameterValue (ID::gain),
              apvts.getRawParameterValue (ID::treble),
              apvts.getRawParameterValue (ID::middle),
              apvts.getRawParameterValue (ID::bass),
              apvts.getRawParameterValue (ID::ampVol))
    , mod    (apvts.getRawParameterValue (ID::modFx),
              apvts.getRawParameterValue (ID::modSpeed))
    , delay  (apvts.getRawParameterValue (ID::dlyMix),
              apvts.getRawParameterValue (ID::dlyTime))
    , reverb (apvts.getRawParameterValue (ID::rvbDecay),
              apvts.getRawParameterValue (ID::rvbMix))
    , irCab  (apvts.getRawParameterValue (ID::irSlot))
{
    // Register all 8 IR slots from BinaryData
    irCab.registerSlotData (1, BinaryData::ir_01_bright_wav,   BinaryData::ir_01_bright_wavSize);
    irCab.registerSlotData (2, BinaryData::ir_02_warm_wav,     BinaryData::ir_02_warm_wavSize);
    irCab.registerSlotData (3, BinaryData::ir_03_midscoop_wav, BinaryData::ir_03_midscoop_wavSize);
    irCab.registerSlotData (4, BinaryData::ir_04_vintage_wav,  BinaryData::ir_04_vintage_wavSize);
    irCab.registerSlotData (5, BinaryData::ir_05_room_wav,     BinaryData::ir_05_room_wavSize);
    irCab.registerSlotData (6, BinaryData::ir_06_tight_wav,    BinaryData::ir_06_tight_wavSize);
    irCab.registerSlotData (7, BinaryData::ir_07_heavy_wav,    BinaryData::ir_07_heavy_wavSize);
    irCab.registerSlotData (8, BinaryData::ir_08_american_wav, BinaryData::ir_08_american_wavSize);
}

void PanzerGGProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gate  .prepare (sampleRate, samplesPerBlock);
    amp   .prepare (sampleRate, samplesPerBlock);
    mod   .prepare (sampleRate, samplesPerBlock);
    delay .prepare (sampleRate, samplesPerBlock);
    reverb.prepare (sampleRate, samplesPerBlock);
    irCab .prepare (sampleRate, samplesPerBlock);
}

void PanzerGGProcessor::releaseResources()
{
    gate  .reset();
    amp   .reset();
    mod   .reset();
    delay .reset();
    reverb.reset();
    irCab .reset();
}

void PanzerGGProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (modeManager.isTotalBypass())
    {
        buffer.clear();   // tuner mutes output; PRESET bypass = silence
        return;
    }

    gate.process (buffer);

    if (modeManager.isAmpOn())    amp   .process (buffer);
    if (modeManager.isModOn())    mod   .process (buffer);
    if (modeManager.isDelayOn())  delay .process (buffer);
    if (modeManager.isReverbOn()) reverb.process (buffer);

    irCab.process (buffer);

    // Master volume — always active
    const float masterGain = juce::jmap (
        apvts.getRawParameterValue (PanzerGGParameters::ID::master)->load(),
        0.f, 100.f, 0.f, 1.5f);
    buffer.applyGain (masterGain);
}

void PanzerGGProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PanzerGGProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        modeManager.syncFromApvts (apvts);
    }
}

juce::AudioProcessorEditor* PanzerGGProcessor::createEditor()
{
    return nullptr;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PanzerGGProcessor();
}
