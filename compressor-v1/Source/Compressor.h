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
    
    
    
}


