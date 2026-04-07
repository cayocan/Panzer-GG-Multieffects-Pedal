cmake_minimum_required(VERSION 3.22)
project(PanzerGG VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable JUCE extras/examples to speed up build
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(extern/JUCE)

juce_add_plugin(PanzerGG
    COMPANY_NAME                "PanzerGG"
    COMPANY_WEBSITE             ""
    COMPANY_EMAIL               ""
    PLUGIN_MANUFACTURER_CODE    "PnGG"
    PLUGIN_CODE                 "PnG1"
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Panzer-GG"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Fx" "Distortion"
)

target_sources(PanzerGG
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
)

target_include_directories(PanzerGG
    PRIVATE
        src/
)

target_compile_definitions(PanzerGG
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(PanzerGG
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Unit tests (JUCE console app) ────────────────────────────────
juce_add_console_app(PanzerGGTests
    PRODUCT_NAME "PanzerGG Tests"
)

target_sources(PanzerGGTests PRIVATE tests/ParameterTests.cpp)
target_include_directories(PanzerGGTests PRIVATE src/)
target_compile_definitions(PanzerGGTests PRIVATE JUCE_UNIT_TESTS=1)
target_link_libraries(PanzerGGTests
    PRIVATE
        juce::juce_audio_processors
        juce::juce_recommended_warning_flags
)
