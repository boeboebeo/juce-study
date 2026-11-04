/*
    PluginProcessor.cpp
    
    - Limiter.cpp 가 실제 오디오 신호를 처리하는 DSP 엔진이라면
    - PluginProcessor.cpp는 그 DSP 엔진을 JUCE 플러그인 시스템과 연결, 관리하는 역할
    
 */


#include "PluginProcessor.h"
#include "PluginEditor.h"


//===========================================================================
LimiterAudioProcessor::LimiterAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
        //apvts 초기화
{
}

LimiterAudioProcessor::~LimiterAudioProcessor() = default;

//===========================================================================
juce::AudioProcessorValueTreeState::ParameterLayout LimiterAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Input gain pushes the signal into the (fixed) ceiling. This is how most
    // modern limiters expose "threshold": more input gain = more limiting.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "inputGain", "Input Gain",
        juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "ceiling", "Ceiling",
        juce::NormalisableRange<float> (-12.0f, 0.0f, 0.1f), -1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "release", "Release",
        juce::NormalisableRange<float> (5.0f, 1000.0f, 1.0f, 0.4f), 100.0f));
        //0.4f : skew 값
        //파라미터의 정규화된 값과 실제 값 사이의 매핑 곡선을 조정함

    return { params.begin(), params.end() };
}

//===========================================================================
void LimiterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
    //오디오 처리전 필요한 환경을 설정하는 함수
{
    limiter.prepare (sampleRate, samplesPerBlock, getTotalNumInputChannels());
        //limiter 초기화

    // The lookahead delays the audio, so the host must compensate (PDC).
    setLatencySamples (limiter.getLatencySamples());
    
        //.getLatencySamples() : limiter 가 발생시키는 지연 샘플 수를 가져옴
        //setLatencySamples (...) : 를 통해 DAW 에 해당 지연을 알려줌
        //=> 그러면 DAW 가 다른 트랙들의 재생시점을 조정해서 전체적인 시간 정렬을 맞춘다.

    updateLimiterParameters();
}

void LimiterAudioProcessor::releaseResources()
{
}

bool LimiterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
    //지원하는 오디오 채널 구성 확인
{
    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet()
        && ! layouts.getMainOutputChannelSet().isDisabled();
}

//===========================================================================
void LimiterAudioProcessor::updateLimiterParameters()
{
    limiter.setInputGain (apvts.getRawParameterValue ("inputGain")->load());
    limiter.setCeiling   (apvts.getRawParameterValue ("ceiling")->load());
    limiter.setRelease   (apvts.getRawParameterValue ("release")->load());
}

void LimiterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& /*midiMessages*/)
    //실제 오디오 처리! => DAW 가 오디오블록을 전달하면 이 함수가 호출된다.
    //별도의 오디오 버퍼를 복사해서 또 만드는게 아니고, DAW가 전달한 버퍼를 직접 다룸

    //private: Limiter limiter; 로 만들어둔 limiter 객체
    //아래의 limiter.processBlock(buffer); 라는 코드를 실행하면 limiter 객체에 속한 Limiter::processBlock() 함수가 호출되는것
        // 그 함수의 구현은 Limiter.cpp에 작성되어있다.
        // private 선언이 되어있지만, LimiterAudioProcessor 클래스 내부에 작성된 멤버함수인 processBlock()에는 접근가능
{
    juce::ScopedNoDenormals noDenormals;

    updateLimiterParameters();
    limiter.processBlock (buffer);
        //limiter.processBlock(buffer)가 원본 버퍼를 직접 수정함
        //따라서 함수가 끝나면 buffer 에는 처리된 오디오가 들어있고, DAW 는 이 결과를 출력함
    
        //& buffer : 이렇게 AudioBuffer 를 참조했기 때문에 buffer.getWrtiePointer[][] 이 코드가 실행된다면 버퍼 안에 저장된 해당 샘플의 값 자체가 변경됨 (입력 받은 버퍼를 직접 수정하는 방식)
}

//===========================================================================
void LimiterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
        //XML 형태로 변환 후 저장
}

void LimiterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//===========================================================================
juce::AudioProcessorEditor* LimiterAudioProcessor::createEditor()
    //GUI 생성
{
    return new LimiterAudioProcessorEditor (*this);
}

//===========================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
    //DAW 가 플러그인을 로드할때 JUCE 가 이 함수를 통해서 Processor 객체를 생성할 수 있음
{
    return new LimiterAudioProcessor();
}
