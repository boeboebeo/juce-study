cmake_minimum_required(VERSION 3.22)
project(delay-v1 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../../JUCE JUCE)

juce_add_plugin(delay-v1
COMPANY_NAME "mudmud"
IS_SYNTH_FALSE
NEEDS_MIDI_INPUT FALSE
NEEDS_MIDI_OUTPUT FALSE
EDITOR_WANTS_KEYBOARD_FOCUS FALSE
COPY_PLUGIN_AFTER_BUILD TRUE
PLUTIN_MANUFACTURER_CODE Mudm
PLUGIN_CODE DLY1
FORMATS AU VST3 Standalone
PRODUCT_NAME "delay-v1"
MICROPHONE_PERMISSION_ENABLED TRUE
MICROPHONE_PERMISSION_TEXT "Delay needs microphone access to process audio input."
)

target_sources(delay-v1
PRIVATE
	Source/PluginProcessor.cpp
	Source/PluginProcessor.h
	Source/PluginEditor.cpp
	Source/PluginEditor.h)

target_link_libraries(delay-v1
PRIVATE
	juce::juce_audio_utils
	juce::juce_dsp
PUBLIC
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
	juce::juce_recommended_warning_flags)
