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
                                                                   "attck",
                                                                   "Attack",
                                                                   juce::NormalisableRange<float> (0.1f, 200.0f, 0.01f, 0.3f),
                                                                   10.0f
                                                                   ));
    params.push_back (std::make_unique<juce::AudioParamterFloat> (
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
    params.push_back (std::make_unique<juce::AudioParamterFloat> (
                                                                  "makeup",
                                                                  "Makeup Gain",
                                                                  juce::NormalisableRange<float> (-12.0f, 24.0f, 0.1f),
                                                                  0.0f
                                                                  ));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
                                                                   "detector",
                                                                   "Detector",
                                                                   juce::StringArray { "Peak", "RMS"}, 1
                                                                   ));
    //default = RMS
    
    return { params.begin(), params.end() };
    
                                                                
}


//==============================================================================

