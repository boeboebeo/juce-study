cmake_minimum_required(VERSION 3.22)
project(gain-v2 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../../JUCE JUCE)

juce_add_plugin(gain-v2 
COMPANY_NAME "mudmud" 
IS_SYNTH_FALSE
NEEDS_MIDI_INPUT FALSE
NEEDS_MIDI_OUTPUT FALSE
EDITOR_WANTS_KEYBOARD_FOCUS FALSE
COPY_PLUGIN_AFTER_BUILD TRUE
PLUGIN_MANUFACTURER_CODE Mudm
PLUGIN_CODE Gav2
FORMATS AU VST3 Standalone
PRODUCT_NAME "gain-v2"
MICROPHONE_PERMISSION_ENABLED TRUE
MICROPHONE_PERMISSION_TEXT "gain-v2 needs microphone access to process audio input."
)

target_sources(gain-v2 
PRIVATE
	Source/PluginProcessor.cpp
	Source/PluginProcessor.h
	Source/PluginEditor.cpp
	Source/PluginEditor.h)

target_link_libraries(gain-v2
PRIVATE
	juce::juce_audio_utils
	juce::juce_dsp
PUBLIC
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
	juce::juce_recommended_warning_flags)

target_compile_definitions(gain-v2
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
