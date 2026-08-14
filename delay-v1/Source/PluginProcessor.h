#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class DelayAudioProcessor : public juce::AudioProcessor
{
public:
    DelayAudioProcessor();
    ~DelayAudioProcessor() override = default;
}

    //DAW가 재생 시작 직전 1회 호출 -> 여기서 버퍼/스무딩 값 준비
void prepareToPlay(double sampleRate, int samplesPerBlock) override;
void releaseResources() override;

void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

juce::AudioProcessorEditor* createEditor() override;
bool hasEditor() const override { return true; }


const juce::String getName() const override { return "SimpleDelay"; }
bool acceptsMidi() const override { return false; }
bool producesMidi() const override { return false; }
double getTailLengthSeconds() const override { return 2.0; }


int getNumPrograms() override { return 1; }
int getCurrentProgram() override { return 0; }
void setCurrentProgram(int) override {}
const juce::String getProgramName(int) override { return {}; }
void changeProgramName(int, const juce::String&) override {}


void getStateInformation(juce::MemoryBlock& destData) override;
void setStateInformation(const void* data, int sizeInbytes) override;

bool isBusesLayoutSupported(const BusesLayout& layouts) const override;


