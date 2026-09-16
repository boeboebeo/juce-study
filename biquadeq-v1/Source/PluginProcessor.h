/*
    PluginProcessor.h
 
 
    Uses juce::AudioProcessorValueTreeState (APVTS) for parameters, which is
    the standard, DAW-automation-safe way to expose plugin parameters in JUCE.
    (이 플러그인의 파라미터를 DAW 가 제대로 인식하고 자동화할 수 있도록 연결해주는 JUCE 의 표준방법)
*/

#pragma once
 
#include <JuceHeader.h>
#include "BiquadBand.h"

class ThreeBandEQAudioProcessor : public juce::AudioProcessor
//DAW, JUCE 와 연결하는 중심 클래스
{
public:
    ThreeBandEQAudioProcessor(); //생성자
    ~ThreeBandEQAudioProcessor() override; //소멸자
    
    //========================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //========================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    
    //========================================================
    const juce::String getName() const override { return "BiquadEQ-v1"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    //========================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    
    //========================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //========================================================
    juce::AudioProcessorValueTreeState apvts;
    
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    //Reads current APVTS values and pushes them into every channel's bands.
    void updateBandParameters();
    
    //filter state (x1, x2, y1, y2)
    std::vector<BiquadBand> highPassBands;
    std::vector<BiquadBand> peakingBands;
    std::vector<BiquadBand> lowPassBands;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessor)
    
    
}
