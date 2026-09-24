cmake_minimum_required(VERSION 3.22)
project(biquadeq-v1  VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(../../JUCE JUCE)

juce_add_plugin(biquadeq-v1
COMPANY_NAME "mudmud"
IS_SYNTH FALSE
NEEDS_MIDI_INPUT FALSE
NEEDS_MIDI_OUTPUT FALSE
EDITOR_WANTS_KEYBOARD_FOCUS FALSE
COPY_PLUGIN_AFTER_BUILD TRUE 
PLUGIN_MANUFACTURER_CODE Mudm
PLUGIN_CODE Bieq
FORMATS AU VST3 Standalone
PRODUCT_NAME "Biquad-EQ_v1"
)

juce_generate_juce_header(biquadeq-v1)

target_compile_definitions(biquadeq-v1
    PRIVATE
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_sources(biquadeq-v1
PRIVATE
	Source/BiquadBand.h
	Source/PluginProcessor.cpp
	Source/PluginProcessor.h
	Source/PluginEditor.cpp
	Source/PluginEditor.h)

target_link_libraries(biquadeq-v1
PRIVATE
	juce::juce_audio_utils
	juce::juce_dsp
PUBLIC
	juce::juce_recommended_config_flags
	juce::juce_recommended_lto_flags
	juce::juce_recommended_warning_flags)
