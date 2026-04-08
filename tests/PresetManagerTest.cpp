#include <JuceHeader.h>
#include "logic/PresetManager.h"
#include "Parameters.h"

// Minimal stub AudioProcessor so we can create an APVTS in tests
struct StubProcessor : juce::AudioProcessor
{
    const juce::String getName() const override { return "Stub"; }
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
};

struct PresetManagerTest : juce::UnitTest
{
    PresetManagerTest() : juce::UnitTest ("PresetManager", "PanzerGG") {}

    void runTest() override
    {
        StubProcessor proc;
        juce::AudioProcessorValueTreeState apvts (
            proc, nullptr, "Parameters", PanzerGGParameters::createLayout());

        beginTest ("Factory presets cover all 9×4 slots");
        {
            PresetManager pm;
            for (int b = 0; b < PresetManager::kBanks; ++b)
                for (int s = 0; s < PresetManager::kSlots; ++s)
                    expect (pm.getPresetName (b, s) != nullptr);
        }

        beginTest ("loadPreset applies parameters to APVTS");
        {
            PresetManager pm;
            pm.loadPreset (0, 0, apvts);  // "Clean Bright"
            const float gain = apvts.getRawParameterValue ("gain")->load();
            expectWithinAbsoluteError (gain, 28.f, 0.5f);
        }

        beginTest ("loadPreset bank 4 slot 0 sets metal amp type");
        {
            PresetManager pm;
            pm.loadPreset (4, 0, apvts);  // "Modern Metal"
            const int ampType = static_cast<int> (
                apvts.getRawParameterValue ("ampType")->load());
            expectEquals (ampType, 4);
        }

        beginTest ("savePreset captures current APVTS state");
        {
            PresetManager pm;
            // Set a custom gain in APVTS
            if (auto* p = apvts.getParameter ("gain"))
                p->setValueNotifyingHost (p->convertTo0to1 (77.f));

            pm.savePreset (8, 0, apvts);  // save into Bank 8 slot A
            expect (pm.isModified (8, 0));

            // Load it back and verify
            apvts.getRawParameterValue ("gain")->store (0.f); // reset
            pm.loadPreset (8, 0, apvts);
            const float gain = apvts.getRawParameterValue ("gain")->load();
            expectWithinAbsoluteError (gain, 77.f, 0.5f);
        }

        beginTest ("resetToFactory restores default and clears modified flag");
        {
            PresetManager pm;
            if (auto* p = apvts.getParameter ("gain"))
                p->setValueNotifyingHost (p->convertTo0to1 (99.f));
            pm.savePreset (0, 0, apvts);
            expect (pm.isModified (0, 0));

            pm.resetToFactory (0, 0);
            expect (! pm.isModified (0, 0));

            pm.loadPreset (0, 0, apvts);
            const float gain = apvts.getRawParameterValue ("gain")->load();
            expectWithinAbsoluteError (gain, 28.f, 0.5f);  // factory "Clean Bright"
        }

        beginTest ("XML round-trip preserves modified preset");
        {
            PresetManager pm;
            if (auto* p = apvts.getParameter ("master"))
                p->setValueNotifyingHost (p->convertTo0to1 (88.f));
            pm.savePreset (3, 2, apvts);

            auto xml = pm.createXml();
            expect (xml != nullptr);

            PresetManager pm2;
            pm2.loadFromXml (*xml);
            expect (pm2.isModified (3, 2));

            pm2.loadPreset (3, 2, apvts);
            const float master = apvts.getRawParameterValue ("master")->load();
            expectWithinAbsoluteError (master, 88.f, 0.5f);
        }
    }
};

static PresetManagerTest presetManagerTest;
