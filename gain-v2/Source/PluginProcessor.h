#pragma once

#include <juce_audio_processors/juce_audio_procesors.h>
#include <juce_dsp/juce_dsp.h>

class gainv2AudioProcessor : public juce::AudioProcessor
{
public:
    gainv2AudioProcessor();
    ~gainv2AudioProcessor() override;
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    
    const juce::String getName() const override {return "GainEffector";}
    
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    double getTailLengthSeconds() const override {return 0.0;}
    
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override {return {};}
    void changeProgramName (int, const juce::String&) override {}
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    
    juce::AudioProcessorValueTreeState apvts;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    static constexpr auto GAIN_DB_ID = "gainDb";
    static constexpr auto PHASE_INVERT_ID = "phaseInvert";
    
private:
    juce::dsp::Gain<float> gainProcessor;
    std::atomic<bool> phaseInverted {false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (gainv2AudioProcessor)
};
