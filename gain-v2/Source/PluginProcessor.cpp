#include "PluginProcessor.h"
#include "PluginEditor.h"

gainv2AudioProcessor::gainv2AudioProcessor()
    : AudioProcessor (BusesProperties()
              .withInput ("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput ("Output", juce::AudioChannelset::stereo(), true)),
    apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

gainv2AudioProcessor::~gainv2AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
    gainv2AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
       juce::ParameterID { GAIN_DB_ID, 1},
       "Gain",
       juce::NormalisableRange<float> (-60.0f, 12.0f, 0.01f, 0.5f),
       "dB" )); // 내부의 4개 매개변수
        
    params.push_back (std::make_unique<juce::AudioParameterBool> (
      juce::ParameterID { PHASE_INVERT_ID, 1},
      "Phase Invert",
      false));
        
        return { params.begin(), params.end() };
}

void gainv2EffectorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());
    
    gainProcessor.prepare (spec);
    gainProcessor.setRampDurationSeconds (0.02);
}
