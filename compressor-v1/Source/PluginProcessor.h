/*
    PluginProcessor.h
 */



#pragma once
 
#include <JuceHeader.h>
#include "Compressor.h"
 
class CompressorAudioProcessor : public juce::AudioProcessor
{
public:
    CompressorAudioProcessor();
    ~CompressorAudioProcessor() override;
    
    //==================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    //==================================================================
    const juce::String getName() const override { return "compressor-v1"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    //==================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
        //(int) : 형변환 아님. 매개변수의 자료형만 써놓고 변수명은 생략한것.
        //override 함수에서 매개변수를 실제로 사용하지 않을때 흔히 쓰는 방식
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
        // const : 이 참조를 통해 원본 값을 수정하지 않겠다는 뜻
        // juce::String& 이 자료형, & : 참조
        // 복사하지 않고, 참조로 받음
    
    // juce::AudioProcessor 의 가상함수를 override 하고 있기 때문에
    // 그냥 형식만 맞춰서 빈 구현을 해놓은 것
    
    
    //==================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==================================================================
    juce::AudioProcessorValueTreeState apvts;
    
    // for the GUI's gein-reduction meter
    float getCurrentGainReductionDb() const { return compressor.getCurrentGainReductionDb(); }
        //compressor 객체 내부의 getCurrent.. 함수를 실행하여 그 결과를 반환함
        //이전까지의 이펙터에서는 실시간 정보를 중간에 사용자에게 보여주지는 않았는데,
        //comp는 GR 을 보여주어야 함
    
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    void updateCompressorParameters();
    
    Compressor compressor;
        //굳이 이렇게 compressor 를 따로 만들어서 관리하는 이유
        //notion 에 정리해둠
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessor)
    
};
