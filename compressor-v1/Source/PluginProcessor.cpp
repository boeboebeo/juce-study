/*
    PluginProcessor.cpp
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
: AudioProcessor (BusesProperties()
                  .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    
}

CompressorAudioProcessor::~CompressorAudioProcessor() = default;


//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CompressorAudioProcessor::createParameterLayout()
    //파라미터를 모아서 ParameterLayout 객체로 반환하는 함수
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "threshold",
                                                                   "Threshold",
                                                                   juce::NormalisableRange<float> (-60.0f, 0.0f, 0.1f),
                                                                   -18.0f
                                                                   ));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "ratio",
                                                                   "Ratio",
                                                                   juce::NormalisableRange<float>
                                                                   (1.0f, 20.0f, 0.1f, 0.5f),
                                                                   4.0f
                                                                   ));
    // (최소값, 최대값, 값의 간격(step), skew factor)
    // 기본값 (default value)
    // 여기서 skew factor 라는 값을 1보다 작은 값을 사용하면 낮은 ratio 영역이 슬라이더의 더 넓은 구간을 차지하게 함
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "knee",
                                                                   "Knee",
                                                                   juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f),
                                                                   6.0f
                                                                   ));
    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "attack",
                                                                   "Attack",
                                                                   juce::NormalisableRange<float> (0.1f, 200.0f, 0.01f, 0.3f),
                                                                   10.0f
                                                                   ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                  "release",
                                                                  "Release",
                                                                  juce::NormalisableRange<float> (5.0f, 2000.0f, 1.0f, 0.3f),
                                                                  100.0f
                                                                  ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "rmsWindow",
                                                                   "RMS Window",
                                                                   juce::NormalisableRange<float> (1.0f, 50.0f, 0.1f, 0.5f),
                                                                   10.0f
                                                                   ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                  "makeup",
                                                                  "Makeup Gain",
                                                                  juce::NormalisableRange<float> (-12.0f, 24.0f, 0.1f),
                                                                  0.0f
                                                                  ));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
                                                                   "detector",
                                                                   "Detector",
                                                                   juce::StringArray { "Peak", "RMS"}, 1
                                                                   ));
    //default = RMS
    //얘는 선택하는 애라서 juce::AudioParameterChoice 로 사용해야 함
    
    return { params.begin(), params.end() };
    
                                                                
}


//==============================================================================
void CompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    compressor.prepare (sampleRate, samplesPerBlock);
    updateCompressorParameters();
}

void CompressorAudioProcessor::releaseResources()
{
    
}

bool CompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
    && ! layouts.getMainOutputChannelSet().isDisabled();
}


//==============================================================================
void CompressorAudioProcessor::updateCompressorParameters()
    //아래의 processBlock()에서 한 블럭마다 실행시켜서 파라미터 값 업데이트시킴
{
    compressor.setThreshold (apvts.getRawParameterValue ("threshold")->load());
    compressor.setRatio     (apvts.getRawParameterValue ("ratio")->load());
    compressor.setKnee      (apvts.getRawParameterValue ("knee")->load());
    compressor.setAttack    (apvts.getRawParameterValue ("attack")->load());
    compressor.setRelease   (apvts.getRawParameterValue ("release")->load());
    compressor.setRmsWindow (apvts.getRawParameterValue ("rmsWindow")->load());
    compressor.setMakeupGain(apvts.getRawParameterValue ("makeup")->load());
        // ->load() : 그 포인터가 가리키는 원자적 값(std::atomic<float>) 에서 현재 값을 읽어옴
    
    const int detectorChoice = (int) apvts.getRawParameterValue ("detector")->load();
    compressor.setDetectorType (detectorChoice == 0 ? Compressor::DetectorType::Peak : Compressor::DetectorType::RMS);
        //조건문 ( 0: peak, 1:RMS )
        //선택 인덱스를 Compressor 의 DetectorType 이 enum으로 변환하여 전달함
    
}



//==============================================================================
void CompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& /*midiMessages*/)
    //이 함수는 JUCE 플러그인 구조와 DAW에서 서로 연결되어 있어서, 호스트(DAW)가 오디오 처리를 요청할때 자동으로 호출해줌 -> 블럭마다 한번씩
{
    juce::ScopedNoDenormals noDenormals;
    
    // Recomputed once per block -- attack/release/RMS-window coefficients
    // only change when the user moves a knob
    updateCompressorParameters();
    
    compressor.processBlock (buffer);
        //이게 실제 오디오 처리를 수행하는 단계
}



//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
 
void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
 
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
 
//==============================================================================
juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new CompressorAudioProcessorEditor (*this);
}
 
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}
 
