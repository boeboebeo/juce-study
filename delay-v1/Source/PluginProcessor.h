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


