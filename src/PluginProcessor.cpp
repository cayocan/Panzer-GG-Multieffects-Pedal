#include "PluginProcessor.h"

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

    gate  .process (buffer);
    amp   .process (buffer);
    mod   .process (buffer);
    delay .process (buffer);
    reverb.process (buffer);
    irCab .process (buffer);

    // Master volume — always active, applied after the full chain
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
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* PanzerGGProcessor::createEditor()
{
    return nullptr;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PanzerGGProcessor();
}
