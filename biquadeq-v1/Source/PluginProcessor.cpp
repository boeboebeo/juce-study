/*
    PluginProcessor.cpp
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//=============================================================
ThreeBandEQAudioProcessor::ThreeBandEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    apvts (*this, nullptr, "PARAMETERS", createParameterLayout())

{
}

ThreeBandEQAudioProcessor::~ThreeBandEQAudioProcessor() = default;


//=============================================================


