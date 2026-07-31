#include "PluginProcessor.h"
#include "PluginEditor.h"

gainv2AudioProcessor::gainv2AudioProcessor()
    : AudioProcessor (BusesProperties()
              .withInput ("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput ("Output", juce::AudioChannelset::stereo(), true)),
    apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

