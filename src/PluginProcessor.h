#pragma once
#include <JuceHeader.h>
#include "Parameters.h"
#include "dsp/GateProcessor.h"
#include "dsp/AmpProcessor.h"
#include "dsp/ModProcessor.h"
#include "dsp/DelayProcessor.h"
#include "dsp/ReverbProcessor.h"
#include "dsp/IrCabProcessor.h"
#include "logic/ModeManager.h"
#include "logic/PresetManager.h"

class PanzerGGProcessor : public juce::AudioProcessor
{
public:
    PanzerGGProcessor();
    ~PanzerGGProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return false; }

    //==============================================================================
    const juce::String getName() const override { return "Panzer-GG"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    //==============================================================================
    int getNumPrograms() override    { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    ModeManager   modeManager;
    PresetManager presetManager;

private:
    // DSP chain — declaration order matches initialization order
    GateProcessor   gate;
    AmpProcessor    amp;
    ModProcessor    mod;
    DelayProcessor  delay;
    ReverbProcessor reverb;
    IrCabProcessor  irCab;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanzerGGProcessor)
};
