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

juce::AudioProcessorValueTreeState apvts;

private:
    //파라미터 정의 (delay time, feedback, mix)
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

//circular delay buffer
//python 프로토타입과 달리 곡 길이를 모르므로, 최대로 걸 수 있는 딜레이 타임 크기만큼
//고정 버퍼를 만들어 계속 재사용함
juce::AudioBuffer<float> delayBuffer;
std::vector<int> writePos; //채널별 현재 write 위치
int maxDelaySamples = 1;
static constexpr float maxDelayTimesMs = 2000.0f //버퍼가 감당할 최대 딜레이 (2초)


