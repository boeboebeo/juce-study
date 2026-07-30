#pragma once

#include <juce_audio_processors/juce_audio_procesors.h>
#include <juce_dsp/juce_dsp.h>

class gainv2AudioProcessor : public juce::AudioProcessor
{
public:
    GainEffectorAudioProcessor();
    ~GainEffectorAudioProcessor() override;
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
}
