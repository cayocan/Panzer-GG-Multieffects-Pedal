#include <JuceHeader.h>
#include "Parameters.h"

struct ParameterLayoutTest : juce::UnitTest
{
    ParameterLayoutTest() : juce::UnitTest ("ParameterLayout", "PanzerGG") {}

    void runTest() override
    {
        beginTest ("createLayout returns non-empty layout");
        auto layout = PanzerGGParameters::createLayout();

        int count = 0;
        for (auto* p : layout.getParameters (false))
        {
            juce::ignoreUnused (p);
            ++count;
        }
        expectGreaterOrEqual (count, 19);

        beginTest ("gate param exists");
        auto* gate = layout.getParameterWithID ("gate");
        expect (gate != nullptr, "gate parameter must exist");

        beginTest ("ampType param exists");
        auto* ampType = layout.getParameterWithID ("ampType");
        expect (ampType != nullptr, "ampType parameter must exist");

        beginTest ("irSlot param exists");
        auto* irSlot = layout.getParameterWithID ("irSlot");
        expect (irSlot != nullptr, "irSlot parameter must exist");

        beginTest ("liveMode param exists");
        auto* live = layout.getParameterWithID ("liveMode");
        expect (live != nullptr, "liveMode parameter must exist");

        beginTest ("tuner param exists");
        auto* tuner = layout.getParameterWithID ("tuner");
        expect (tuner != nullptr, "tuner parameter must exist");

        beginTest ("all 15 knob params exist");
        const juce::StringArray knobIDs {
            "gate", "gain", "treble", "middle", "bass", "ampVol",
            "modFx", "modSpeed", "dlyMix", "dlyTime",
            "rvbDecay", "rvbMix", "master"
        };
        for (auto& id : knobIDs)
            expect (layout.getParameterWithID (id) != nullptr,
                    "Missing param: " + id);
    }
};

static ParameterLayoutTest parameterLayoutTest;

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    return failures > 0 ? 1 : 0;
}
