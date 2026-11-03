cmake_minimum_required(VERSION 3.22)
project(SuperTune VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")

# Downloads JUCE automatically the first time you configure.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# On a Mac, building also installs the plugin into ~/Library/Audio/Plug-Ins automatically.
if(APPLE)
    set(ST_AUTO_INSTALL TRUE)
else()
    set(ST_AUTO_INSTALL FALSE)
endif()

juce_add_plugin(SuperTune
    COMPANY_NAME                "Bodie Audio"
    PRODUCT_NAME                "SuperTune"
    PLUGIN_MANUFACTURER_CODE    Bodi
    PLUGIN_CODE                 SpTn
    FORMATS                     VST3 AU Standalone
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     ${ST_AUTO_INSTALL}
    VST3_CATEGORIES             Fx Pitch Shift
    AU_MAIN_TYPE                kAudioUnitType_Effect)

target_sources(SuperTune PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/PitchEngine.cpp)

target_compile_definitions(SuperTune PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SuperTune
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
