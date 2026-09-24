/*
    PluginProcessor.cpp
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//===========================================================================
ThreeBandEQAudioProcessor::ThreeBandEQAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    apvts (*this, nullptr, "PARAMETERS", createParameterLayout())

{
}

ThreeBandEQAudioProcessor::~ThreeBandEQAudioProcessor() = default;


//===========================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ThreeBandEQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    //High-pass band
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "hpFreq", //Parameter ID (첫번째 인자로 전달되는 문자열이 파라미터 식별자)
                                                                   "HP Freq",   //사용자에게 보여주긴 위한 파라미터 이름
                                                                   juce::NormalisableRange<float> (20.0f, 2000.0f, 0.1f, 0.3f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "hpQ",
                                                                   "HP Q",
                                                                   juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f, 0.5f), 0.707f
                                                                   ));
    
    //위의 AudioParameterFloat 을 params 가 소유함 (이 객체의 유일한 소유자)
    //그래서 delete 할 필요 없음
    
    //Peaking band
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
                                                                    "peakFreq",
                                                                    "Peak Freq",
                                                                    juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 100.0f
                                                                    ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "peakQ",
                                                                   "Peak Q",
                                                                   juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f, 0.5f), 1.0f
                                                                   ));
    params.push_back (std::make_unique<juce::AudioParameterFloat>(
                                                                  "peakGain",
                                                                  "Peak Gain",
                                                                  juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 6.0f
                                                                  ));
    
    //Low-pass band
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "lpFreq",
                                                                   "LP Freq",
                                                                   juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 1000.0f
                                                                   ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "lpQ",
                                                                   "LP Q",
                                                                   juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f, 0.5f), 0.707f
                                                                   ));
    
    return { params.begin(), params.end() };
    //params 의 원소 전체를 이용해서 ParameterLayout 객체를 만들어 반환
    
}
    
//=========================================================================
void ThreeBandEQAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
    {
        const int numChannels = juce::jmax (getTotalNumInputChannels(), 1);
        
        highPassBands.assign ((size_t) numChannels, BiquadBand());
        peakingBands.assign ((size_t) numChannels, BiquadBand());
        lowPassBands.assign ((size_t) numChannels, BiquadBand());
        
        for (auto& band : highPassBands) band.prepare (sampleRate);
        for (auto& band : peakingBands) band.prepare (sampleRate);
        for (auto& band : lowPassBands) band.prepare (sampleRate);
        
        updateBandParameters();
            //ThreeBandEQAudioProessor 객체의 멤버 함수 -> 밑에서 정의됨
    }
    
void ThreeBandEQAudioProcessor::releaseResources()
{
}
    
bool ThreeBandEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
    && ! layouts.getMainOutputChannelSet().isDisabled();
}
    

//=========================================================================
void ThreeBandEQAudioProcessor::updateBandParameters()
        //prepareToPlay 에서 쓰였던 함수
    {
        const float hpFreq  = apvts.getRawParameterValue ("hpFreq")->load();
        const float hpQ     = apvts.getRawParameterValue ("hpQ")->load();
        const float peakFreq = apvts.getRawParameterValue ("peakFreq")->load();
        const float peakQ   = apvts.getRawParameterValue ("peakQ")->load();
        const float peakGain   = apvts.getRawParameterValue ("peakGain")->load();
        const float lpFreq  = apvts.getRawParameterValue ("lpFreq")->load();
        const float lpQ     = apvts.getRawParameterValue ("lpQ")->load();
        
        for (auto& band : highPassBands)
            band.setParameters (FilterType::HighPass, hpFreq, hpQ, 0.0);
        
        for (auto& band : peakingBands)
            band.setParameters (FilterType::Peaking, peakFreq, peakQ, peakGain);
        
        for (auto& band : lowPassBands)
            band.setParameters (FilterType::LowPass, lpFreq, lpQ, 0.0);
    
    }
    
    
void ThreeBandEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& /*midiMessages*/)
    
    {
        juce::ScopedNoDenormals noDenormals;
        
        updateBandParameters(); //여기서 block 마다 계수계산 다시 함
        
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            //cascade(직렬) 구조. hp -> peak -> lp 로 순차적으로 계산됨
            //각 채널마다 독립적인 cascade
            //Heq(z) = Hhp​(z) * Hpeak​(z) * Hlp(z)
            auto* data = buffer.getWritePointer (ch);
            auto& hp   = highPassBands[(size_t) ch];
            auto& peak = peakingBands[(size_t) ch];
            auto& lp   = lowPassBands[(size_t) ch];
            
            for (int n = 0; n < numSamples; ++n)
            {
                float sample = data[n];
                sample = hp.processSample (sample);
                sample = peak.processSample (sample);
                sample = lp.processSample (sample);
                data[n] = sample;
            }
        }
        
    }
    
//==============================================================================
void ThreeBandEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
    
void ThreeBandEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
 
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
    
//==============================================================================
juce::AudioProcessorEditor* ThreeBandEQAudioProcessor::createEditor()
{
    return new ThreeBandEQAudioProcessorEditor (*this);
}

    
//==============================================================================
// creates new instances of the plugin.
    
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThreeBandEQAudioProcessor();
}
