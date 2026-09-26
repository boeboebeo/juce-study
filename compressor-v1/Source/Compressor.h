/*
 Compressor.h
 
 - Feedforward dynamic compressor
    + switchable Peak / RMS detection
    + soft-knee static curve
    + branching attack/release ballistics smoothing
    (applied in the dB domain, on the gain-reduction signal,
    not on the raw input level)
    + stereo Linked detection
 
 
 - Giannoulis, Massberg, Reiss, "Digital Dynamic Range Compressor Design --
 A Tutorial and Analysis" (JAES, 2012) -- 이 논문에서의 표준 feedforward compressor topology 를 따름
 */

#pragma once

#include <JuceHeader.h>
#include <atomic>
    //std::atomic 기능을 쓰기 위해 필요한 헤더 (여러 스레드 동시 접근시 안전하게 처리하는 도구)

class Compressor
{
public:
    enum class DetectorType
    {
        Peak,
        RMS
    };
    
    void prepare (double sampleRate, int maximumBlockSize);
    void reset();
    
    
    //내부 파라미터를 새로운 값으로 업데이트 하는 setter
    //setter = "사용자가 정한 Compressor 설정값을 내부 DSP가 사용할 형태로 전달하는 함수"
    void setThreshold (float newThresholdDb);
    void setRatio (float newRatio);
    void setKnee (float newKneeDb);
    void setAttack (float newAttackMs);
    void setRelease (float newReleaseMs);
    void setDetectorType (DetectorType newType);
        // 위에서 새롭게 정의한 자료형 : DetectorType
    void setRmsWindow (float newWindowMs);
        // 최근 "newWindowMs" 동안의 신호 에너지를 이용해서 RMS level 을 계산해라
        // window 크기가 바뀌면 RMS 계산에 필요한 sample 수나 buffer 를 업데이트 해야 할수도
    void setMakeupGain (float newMakeupDb);
    
    //linked detector value
    void processBlock (juce::AudioBuffer<float>& buffer);
    
    //Thread-safe to read from the message thread while the audio thread writes it.
    float getCurrentGainReductionsDb() const noexcept { return currentGrDb.load(); }
        // .load() : atomic 객체의 값을 읽는 연산
        // (std::atomic 객체가 제공하는 원자적 읽기 연산)
    
private:
    float computeLinkedLevel (const juce::AudioBuffer<float>& buffer, int sampleIndex);
    float staticCurveDb (float inputDb) const;
        //static : 정적
        //static (in programming) : 객체를 생성하지 않고도 메모리에 고정적으로 할당되어, 클래스 자체에 속하게 되는 멤버 - 객체 생성 불필요 (바로 클래스이름.변수명 이렇게 사용가능)
    void updateTimeConstants();
    
    double fs = 44100.0;
    
    // Parameters
    float thresholdDb = -18.0f;
    float ratio = 4.0f;
    float kneeDb = 6.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float makeupDb = 0.0f;
    DetectorType detectorType = DetectorType::RMS;
    float rmsWindowMs = 10.0f;
    
    
    // GR 이 값자기 0dB -> -10dB 이렇게 변해버리면 소리가 너무 갑작스럽게 변함
    // 조금씩 따라가게 만드는 one-pole 식 : y[n] = (1−α)x[n] + αy[n−1]
    // 위 식에서의 α (alpha)가 smoothing coefficient
    // time parameter 가 바뀌면 다시 계산해야함 -> smoothing 의  속도가 달라져야하므로
    // updateTimeConstants() 함수와 관련이 있다.
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float rmsCoeff = 0.0f;
    
    
    // Running state
    float rmsMeanSquare = 0.0f; //linear power, for the RMS detector
        // 평균, 제곱을 먼저함. 선형영역의 제곱값
    float envelopeDb = 0.0f; //smoothed gain-reduction envelope (dB, <= 0 : 정상적인 GR env 은 양수가 되면 안됨 )
        // 만약 0 -> -10dB 까지 게인 리덕션이 천천히 내려간다면
        // 현재 envelope 0 -> -1 -> -2 -> ... -> -9 -> -10 이렇게 줄어드는 와중에
        // 현재 위치를 저장하는게 envelopeDb 이다.
    
    std::atomic<float> currentGrDb { 0.0f };
        //위의 getCurrentGainReductionsDb() 이 함수에서 매개변수로 값을 불러오게 함
        // gain reduction 의 값을 Audio Thread 와 GUI Thread 사이에서 안전하게 공유하기 위한 변수
    
    // envelopeDb = DSP 내부에서 실제 Comp 가 사용하는 상태
    // currentGrDb = GUI 가 현재 GR 을 표시하기 위해 읽는 공유값
    
    static constexpr float minusInfDb = -100.0f;
        // minusInfDb 를 근사값으로 표현하기 위한 -100dB 상수
        // 20log(0) = -무한대가 나오므로, 그냥 실용적인 근사값인 -100.0f 를 사용한다. (사실상 무음 취급)
        // 컴파일 할때 이미 -100.0이라는 값이 확정이므로 고정된 상수임
    
    
};


