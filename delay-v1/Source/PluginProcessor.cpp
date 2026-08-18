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
    
    //
}
