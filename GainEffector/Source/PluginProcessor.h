#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class GainEffectorAudioProcessor : public juce::AudioProcessor
{
public:
	// 플러그인이 DAW 에 처음 로드될때 
	GainEffectorAudioProcessor();
	// 플러그인을 트랙에서 빼거나 DAW 를 종료할때
	~GainEffectorAudioProcessor() override;

	// 재생/처리가 시작되기 직전 (샘플레이트나 버퍼크기가 바뀔때도 다시 호출됨)
	void prepareToPlay (double sampleRate, int samplesPerBlock) override;
	// 재생이 멈추거나, 플러그인이 비활성화 될때)
	void releaseResources() override;
	// 오디오가 흐르는 동안 계속 반복(초당 수십~수백 번) - 실제 게인계산이 일어나는 곳
	void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	// {..} 이렇게 함수 안에 바로 정의되어있는것은 다 inline 으로 처리 된것.
	// 사용자가 플러그인 창(GUI)를 열 때
	juce::AudioProcessorEditor* createEditor() override;
	// DAW 가 이 플러그인이 GUI 를 지원하나 확인할때
	bool hasEditor() const override {return true;}

	// DAW 가 플러그인 목록에 이름을 표시해야 할때
	const juce::String getName() const override {return "GainEffector";}

	// DAW 가 이 플러그인을 트랙에 적용할수있는지 판단할때 (MIDI track & AUDIO track인지 확인)
	bool acceptsMidi() const override {return false;}
	bool producesMidi() const override {return false;}
	// 소리가 멈춘 뒤에도 이펙트가 얼마나더 울리는지 (리버브 등) - DAW 가 알아야 할때
	double getTailLengthSeconds() const override {return 0.0;}

	// DAW 가 프리셋 목록 UI 그릴 때 
	int getNumPrograms() override {return 1;} // DAW 가 프리셋 목록 UI 를 그릴때
	int getCurrentProgram() override {return 0;}  
	void setCurrentProgram (int) override {} 
	const juce::String getProgramName (int) override {return {};} //사용자가 DAW 에서 프리셋 번호 바꿀때
	void changeProgramName (int, const juce::String&) override {}

	void getStateInformation (juce::MemoryBlock& destData) override; //DAW 세션 저장시
	void setStateInformation (const void* data, int sizeInBytes) override; //DAW 세션 다시 열때

	// 플러그인이 트랙에 꽂히기 직전, 이 채널 구성(모노/스테레오)를 지원하나 확인할때
	bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

	// apvts 를 만들때 딱 한번 호출
	juce::AudioProcessorValueTreeState apvts;

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	static constexpr auto GAIN_DB_ID = "gainDb";
	static constexpr auto PHASE_INVERT_ID = "phaseInvert";

private:
	juce::dsp::Gain<float> gainProcessor;
	std::atomic<bool> phaseInverted {false};
	
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainEffectorAudioProcessor)
};
