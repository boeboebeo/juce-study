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

// 파라미터 정의
// 여기 등록된 ID 로 DAW automation , 프리셋 저장/불러오기가 자동 처리됨
juce::AudioProcessorValueTreeState::ParameterLayout DelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    //1) Delay time 0 ~ 2000ms 까지 사용할 수 있도록 셋팅 (버퍼도 그만큼 크게 준비함)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID {"delayTime", 1},
         "Delay Time",
         juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.3f),
         300.0f,
         "ms"
                                                                 ));
    
    //2) Feedback (1.0 이상이면 소리가 무한 증폭디므로, 0.98까지만 노출 (안전장치))
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID { "feedback", 1 },
         "Feedback",
         juce::NormalisableRange<float>(0.0f, 0.98f, 0.01f),
         0.3f
                                                                 ));
    
    //3) Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID { "mix", 1 },
         "Mix",
         juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
         0.5f
                                                                 ));
    
    return { params.begin(), params.end() };
}
