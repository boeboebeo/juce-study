/*
    PluginProcessor.cpp
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

DelayAudioProcessor::DelayAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    //초기화
{
}

// 파라미터 정의
// 여기 등록된 ID 로 DAW automation , 프리셋 저장/불러오기가 자동 처리됨
juce::AudioProcessorValueTreeState::ParameterLayout DelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    //1) Delay time 0 ~ 2000ms 까지 사용할 수 있도록 셋팅 (버퍼도 그만큼 크게 준비함)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID {"delayTime", 1},
         "Delay Time",
         juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.3f),
         300.0f,
         "ms"
                                                                 ));
    
    //2) Feedback (1.0 이상이면 소리가 무한 증폭디므로, 0.98까지만 노출 (안전장치))
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID { "feedback", 1 },
         "Feedback",
         juce::NormalisableRange<float>(0.0f, 0.98f, 0.01f),
         0.3f
                                                                 ));
    
    //3) Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID { "mix", 1 },
         "Mix",
         juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
         0.5f
                                                                 ));
    
    return { params.begin(), params.end() };
}


//재생 시작전 준비 : 버퍼 크기 확보, 스무딩 초기화
void DelayAudioProcessor::prepareToPlay(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;
    
    //circular buffer 크기 = 최대로 걸수 있는 딜레이타임 기준으로 고정 확보
    maxDelaySamples = (int) (maxDelayTimeMs / 1000.0 * sampleRate) + 1;
    
    //maxDelaySamples, delayBuffer, writePos .. 등 다 .h 파일에서 선언해둔 객체명임
    delayBuffer.setSize(getTotalNumOutputChannels(), maxDelaySamples);
    delayBuffer.clear();
    
    writePos.assign((size_t) getTotalNumOutputChannels(), 0);
    
    //파라미터가 목표값 까지 이동하는 데 걸리는 시간 (클릭 방지)
    const double smoothTimeSec = 0.02; //20ms
    
    smoothedDelayMs.reset(sampleRate, smoothTimeSec);
    smoothedFeedback.reset(sampleRate, smoothTimeSec);
    smoothedMix.reset(sampleRate, smoothTimeSec);
    
    //시작하자마자 현재 apvts 값으로 스무딩 값도 즉시 맞춰둠 (처음에 서서히 안올라오게)
    smoothedDelayMs.setCurrentAndTargetValue(apvts.getRawParameterValue("delayTime") -> load());
    smoothedFeedback.setCurrentAndTargetValue(apvts.getRawParameterValue("feedback") -> load());
    smoothedMix.setCurrentAndTargetValue(apvts.getRawParameterValue("mix") -> load());
    
}

/*
 juce::SmoothedValue<float> smoothedDelayMs;
 juce::SmoothedValue<float> smoothedFeedback;
 juce::SmoothedValue<float> smoothedMix;
                        -> header file 에 이렇게 선언해두었음
 */



void DelayAudioProcessor::releaseResources()
{
    delayBuffer.clear();
}
    //juce::AudioBuffer<float> delayBuffer; 가 헤더파일에 이렇게 선언되어 있음
    //AudioBuffer<float> 이라는 객체 - 여러채널의 float 오디오 샘플을 관리하는 버퍼

//apvts 의 현재 파라미터 값을 스무딩 목표값으로 반영 (매 block 마다 호출)
void DelayAudioProcessor::updateSmoothedTargets()
{
    //atomic<float>* 이므로 오디오 스레드에서 읽어도 안전 (lock-free)
    smoothedDelayMs.setTargetValue(apvts.getRawParameterValue("delayTime")->load());
    smoothedFeedback.setTargetValue(apvts.getRawParameterValue("feedback")->load());
    smoothedMix.setTargetValue(apvts.getRawParameterValue("mix")->load());
}


//실제 오디오처리 (오디오 스레드, 매 block 마다 호출)
//y[n] = x[n](dry)*(1-mix) + x[n-D](wet)*mix
//실시간 처리이므로 -> circular buffer 를 사용하고
//delay time이 정수 sample 이 아닐 수 있으므로 선형보간으로 소수점 위치를 읽음

void DelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals; //아주 작은 값 처리시 CPU 급증하는것 방지
    
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());
    //입력채널보다 출력채널이 더 많다면, 입력이 없는 출력 채널부분의 내부의 샘플들을 다 지우는 과정
    
    updateSmoothedTargets();
    //위에서 처리해놓은 함수의 내용이 매 블럭마다 이거때문에 처리가 됨
    //매 블락마다 apvts 의 현재 파라미터의 값을 스무딩 목표값으로 반영함
    
    const int numChannels = buffer.getNumChannels(); //2 반환
    const int numSamples = buffer.getNumSamples(); //512 반환
    //이거 buffer._ 가 뭘 처리하는게 아니고, buffer 가 가지고 있는 채널 개수를 물어보는 함수
    
    for (int channel = 0; channel < numChannels; ++channel)
        //L 채널 부터 for 문 한번 쭉 돌리고(sample0 - 511까지 처리) 그 다음 R 채널 처리
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* delayData = delayBuffer.getWritePointer(channel);
        
        int& wPos = writePos[(size_t) channel]; //.h 에서 선언 (vector<int> 형)
        //wPos 라는 이름을 writePos[channel]에 붙이는 것 -> L 채널 부터 처리
        //현재 채널의 Circular Buffer write 의 위치(index)
        //wPos 가 4라면 L 채널의 delayBuffer 에서 4번위치를 쓰고 있음
        //int& -> writePos 이라는 vector<int> 안의 값 참조
        
        for (int i = 0; i < numSamples; ++i)
            //이건 샘플을 하나씩 처리하는 반복문
        {
            //이번 샘플에서 쓸 파라미터 값을 한 스텝 진행 (부드럽게 목표값으로 접근)
            const float currentDelayMs = smoothedDelayMs.getNextValue();
            const float currentFeedback = smoothedFeedback.getnextValue();
            const float currentMix = smoothedMix.getNextValue();
            
            
            
            const float delaySamplesFloat = (float) (currentDelayMs / 1000.0 * sampleRate);
            
            const float drySignal = channelData[i];
            //현재 처리할 드라이시그널의 샘플 번호
            
            // wet 신호 읽기 (fractional delay: 선형보간)
            // delay time 이 float 이므로, 정수 인덱스 두곳을 읽어 그 사이를 보간
            float readPosFloat = (float) wPos - delaySamplesFloat;
            
            //readPosFloat 이 정수여도 매 샘플마다 이 계산은 실행됨
            
            while (readPosFloat < 0.0f)
                readPosFloat += (float) maxDelaySamples;
                //원형이므로 순환시켜 유효 범위로
            
            const int readIndex0 = ((int) readPosFloat) % maxDelaySamples;
            const int readIndex1 = (readIndex0 + 1) % maxDelaySamples;
            const float frac = readPosFloat - (float) (int) readPosFloat;
            
            const float wetSignal =
                delayData[readIndex0] * (1.0f - frac) +
                delayData[readIndex1] * frac;
            
            //mix
            channelData[i] = drySignal * (1.0f - currentMix) + wetSignal * currentMix;
                //채널 데이터 이번 block(ex. 512개의 샘플)의 i 번째 샘플
            
            //circular buffer 에 현재값 기록(feedback 포함)
            delayData[wPos] = drySignal + wetSignal * currentFeedback;
                //circular buffer 는 2초짜리 delayBuffer (샘플 96001개)
                //이건 현재 들어온 소리를 DelayBuffer의 wPos 위치에 기록하는 것
            
            //write 위치 한칸 전진, 끝에 도달하면 처음으로 순환(circular buffer)
            wPos = (wPos + 1) % maxDelaySamples;
                //다음 delayBuffer index 로 이동
            
        }
    }
}

