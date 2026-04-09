#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PanzerGGEditor : public juce::AudioProcessorEditor
{
public:
    explicit PanzerGGEditor (PanzerGGProcessor& p);
    ~PanzerGGEditor() override;

    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    static constexpr int kPluginWidth  = 860;
    static constexpr int kPluginHeight = 280;

private:
    void drawChassis       (juce::Graphics& g);
    void drawTopStrip      (juce::Graphics& g);
    void drawCornerScrews  (juce::Graphics& g);
    void drawScrewAt       (juce::Graphics& g, float cx, float cy, float r);

    PanzerGGProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanzerGGEditor)
};
