cmake_minimum_required(VERSION 3.22)
project(GainEffector VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../JUCE JUCE)

juce_add_plugin(GainEffector
COMPANY_NAME "mudmud"
IS_SYNTH FALSE
NEEDS_MIDI_INPUT FALSE
NEEDS_MIDI_OUTPUT FALSE
EDITOR_WANTS_KEYBOARD_FOCUS FALSE
COPY_PLUGIN_AFTER_BUILD TRUE
PLUGIN_MANUFACTURER_CODE Mudm
PLUGIN_CODE Gain
FORMATS AU VST3 Standalone
PRODUCT_NAME "GainEffector")

target_sources(GainEffector
PRIVATE
	Source/PluginProcessor.cpp
	Source/PluginProcessor.h
	Source/PluginEditor.cpp
	Source/PluginEditor.h)

target_link_libraries(GainEffector
PRIVATE
	juce::juce_audio_utils
	juce::juce_dsp
PUBLIC
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
	juce::juce_recommended_warning_flags)
