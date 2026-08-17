/*
    PluginProcessor.cpp
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

DelayAudioProcessor::DelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    //초기화
{
}
