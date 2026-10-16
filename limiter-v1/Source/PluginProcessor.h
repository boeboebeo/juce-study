/*
    PluginProcessor.h
 */

#pragma once

#include <JuceHeader.h>
#include "Limiter.h"

class LimiterAudioProcessor : public juce::AudioProcessor
{
public:
    LimiterAudioProcessor();
    ~LimiterAudioProcessor() override;
    
    //===================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //===================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    
    //===================================================================
    const juce::String getName() const override { return "limiter-v1"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    //===================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    
    //===================================================================
    void getStateInformation (juce::MemoryBlock destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //===================================================================
    juce::AudioProcessorValueTreeState apvts;
    
    float getCurrentGainReductionDb() const { return limiter.getCurrentGainReductionDb(); }
    //getCurrent... 이건 지금 이 PluginProcessor 클래스의 함수
        //return 으로 결과를 만들어낼때의 limiter.getCurrent.. 함수는
        //밑에서 만들 Limiter 타입의 객체에 들어있는 함수
        //Limiter.h 에 선언해둔 Limiter class 타입의 객체
    //위에서 #include "Limiter.h" 처리를 했기 때문에 Limiter class 사용할 수 있다
        //그 안에 Limiter 클래스의 멤버함수인 getCurrent .. 이거 함수 쓰겠다는 의미
    
    
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    void updateLimiterParameters();
    
    Limiter limiter;
        //위에서 #include "Limiter.h"를 표기했기 때문에 Limiter.h 내부의 클래스 형태의 새로운 객체를 만듦 (이름 : limiter)
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LimiterAudioProcessor)


    
}
