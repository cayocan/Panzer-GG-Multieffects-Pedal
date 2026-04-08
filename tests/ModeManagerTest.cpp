#include <JuceHeader.h>
#include "logic/ModeManager.h"

struct ModeManagerTest : juce::UnitTest
{
    ModeManagerTest() : juce::UnitTest ("ModeManager", "PanzerGG") {}

    void runTest() override
    {
        beginTest ("Default state: PRESET mode, bank 0, slot -1 (bypass)");
        {
            ModeManager mm;
            expect (mm.getMode() == ModeManager::Mode::Preset);
            expectEquals (mm.getBank(), 0);
            expectEquals (mm.getSlot(), -1);
            expect (mm.isTotalBypass());
        }

        beginTest ("PRESET mode: footswitch A selects slot 0");
        {
            ModeManager mm;
            mm.handleFootswitch (ModeManager::A);
            expectEquals (mm.getSlot(), 0);
            expect (! mm.isTotalBypass());
        }

        beginTest ("PRESET mode: BANK+ increments bank and resets slot");
        {
            ModeManager mm;
            mm.handleFootswitch (ModeManager::A);           // slot = 0
            mm.handleFootswitch (ModeManager::BankPlus);    // bank++, slot = -1
            expectEquals (mm.getBank(), 1);
            expectEquals (mm.getSlot(), -1);
            expect (mm.isTotalBypass());
        }

        beginTest ("PRESET mode: BANK- does not go below 0");
        {
            ModeManager mm;
            mm.handleFootswitch (ModeManager::BankMinus);
            expectEquals (mm.getBank(), 0);
        }

        beginTest ("PRESET mode: BANK+ does not exceed 8");
        {
            ModeManager mm;
            for (int i = 0; i < 20; ++i)
                mm.handleFootswitch (ModeManager::BankPlus);
            expectEquals (mm.getBank(), 8);
        }

        beginTest ("toggleMode switches between PRESET and LIVE");
        {
            ModeManager mm;
            mm.toggleMode();
            expect (mm.getMode() == ModeManager::Mode::Live);
            mm.toggleMode();
            expect (mm.getMode() == ModeManager::Mode::Preset);
        }

        beginTest ("LIVE mode: footswitch D toggles ampOn");
        {
            ModeManager mm;
            mm.toggleMode();                          // enter LIVE
            expect (mm.isAmpOn());
            mm.handleFootswitch (ModeManager::D);     // toggle off
            expect (! mm.isAmpOn());
            mm.handleFootswitch (ModeManager::D);     // toggle on
            expect (mm.isAmpOn());
        }

        beginTest ("LIVE mode: all module toggles work independently");
        {
            ModeManager mm;
            mm.toggleMode();
            mm.handleFootswitch (ModeManager::A);   // reverb off
            mm.handleFootswitch (ModeManager::B);   // delay off
            mm.handleFootswitch (ModeManager::C);   // mod off
            mm.handleFootswitch (ModeManager::D);   // amp off
            expect (! mm.isReverbOn());
            expect (! mm.isDelayOn());
            expect (! mm.isModOn());
            expect (! mm.isAmpOn());
        }

        beginTest ("PRESET mode: all modules always on (isXxxOn returns true)");
        {
            ModeManager mm;
            // Even without a slot, module queries return true in PRESET mode
            expect (mm.isAmpOn());
            expect (mm.isModOn());
            expect (mm.isDelayOn());
            expect (mm.isReverbOn());
        }

        beginTest ("Tuner active causes isTotalBypass");
        {
            ModeManager mm;
            mm.handleFootswitch (ModeManager::A);   // select a slot → not bypassed
            expect (! mm.isTotalBypass());
            mm.setTuner (true);
            expect (mm.isTotalBypass());
            mm.setTuner (false);
            expect (! mm.isTotalBypass());
        }

        beginTest ("LIVE hold gesture returns true (save-preset signal)");
        {
            ModeManager mm;
            mm.toggleMode();
            const bool saved = mm.handleFootswitch (ModeManager::B, /*held=*/true);
            expect (saved);
            expectEquals (mm.getSlot(), 1);   // B = slot 1
        }
    }
};

static ModeManagerTest modeManagerTest;
