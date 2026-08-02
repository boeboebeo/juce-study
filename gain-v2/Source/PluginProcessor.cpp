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

void gainv2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());
    
    gainProcessor.prepare (spec);
    gainProcessor.setRampDurationSeconds (0.02);
}

void gainv2AudioProcessor::releaseResources() {}

bool gainv2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
    && ! layouts.getMainOutputChannelSet().isDisabled();
}

void gainv2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    const float gainDb = apvts.getRawParameterValue (GAIN_DB_ID) -> load();
    const bool invert = apvts.getRawParameterValue (PHASE_INVERT_ID) -> load() > 0.5f;
    
    gainProcessor.setGainDecibels (gainDb);
    
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    gainProcessor.process (context);
    
    if (invert)
        buffer.applyGain (-1.0f);
    
}

juce::AudioProcessorEditor* gainv2AudioProcessor::createEditor()
{
    return new gainv2AudioProcessorEditor (*this);
}

void gainv2AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void gainv2AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState -> hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
