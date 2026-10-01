/*
    Compressor.cpp
 */


#include "Compressor.h"

//=================================================================
//Compressor.h 에서 선언해둔 Compressor class 내부의 함수들 정의

void Compressor::prepare (double sampleRate, int /*maximumBlockSize*/)
{
    fs = sampleRate;
    reset();
    updateTimeConstants();
}

void Compressor::reset()
{
    //이 셋다 컴프의 이전 상태 초기화
    rmsMeanSquare = 0.0f;
    envelopeDb = 0.0f;
    currentGrDb.store (0.0f);
        //이건 currentGrDb.load() 와 한쌍 (이건 현재 currentGrDb에 들어있는 값을 읽어와라)
        //.h 파일에서 std::atomic<float> currentGrDb { 0.0f }; 이렇게 선언해둠
        //.store : currentGrDb에 0.0을 저장해라
        //currentGrDb.store 와 .load 로 불러오는 이유는 GUI 에게 GR을 보여주고 미터를 그리기 위함임
}


//=================================================================
//parameter update
void Compressor::setThreshold(float newThresholdDb) { thresholdDb = newThresholdDb; }
void Compressor::setRatio(float newRatio) { ratio = juce::jmax (newRatio, 1.0f); }
void Compressor::setKnee(float newKneeDb) { kneeDb = juce::jmax (newKneeDb, 0.0f); }
void Compressor::setMakeupGain(float newMakeupDb) { makeupDb = newMakeupDb; }
void Compressor::setDetectorType(DetectorType newType) { detectorType = newType; }

void Compressor::setAttack (float newAttackMs)
{
    attackMs = juce::jmax(newAttackMs, 0.01f);
    updateTimeConstants();
}

void Compressor::setRelease(float newReleaseMs)
{
    releaseMs = juce::jmax(newReleaseMs, 0.01f);
    updateTimeConstants();
}

void Compressor::setRmsWindow (float newWindowMs)
{
    rmsWindowMs = juce::jmax(newWindowMs, 0.1f);
    updateTimeConstants();
}

void Compressor::updateTimeConstants()
{
    // Standard one-pole exponential ballstics
    // after 'timeMs' ms : 약 63%의 amount (not the "time to fully settle")
    attackCoeff = std::exp (-1.0f / (0.001f * attackMs * (float) fs));
    releaseCoeff = std::exp (-1.0f / (0.001f * releaseMs * (float) fs));
    rmsCoeff = std::exp (-1.0f / (0.001f * rmsWindowMs * (float) fs));
    
    // fs 는 double fs 로 선언되어있는데,
    // 0.001f 나 attackMs 등은 float 으로 되어있음
    // 계산을 float 으로 통일하려고 한 값 -> 사실상 안해도 상관 없음
    // double 로 하면 정밀도가 더 높아지기는 한다
}


//==============================================================================
float Compressor::computeLinkedLevel(const juce::AudioBuffer<float>& buffer, int sampleIndex)
{
    const int numChannels = buffer.getNumChannels();
    
    if (detectorType == DetectorType::Peak)
    {
        /*
         stereo-linked PEAK : the largest absolute sample across all channels
         at this time index. No temporal smoothing here
         peak detection should react instantly
         the attack/release ballistics are applied later, to the gain reduction filter
         -> 한 샘플 시점 n 에서 모든 채널 중 가장 큰 절댓값을 detector 값으로 사용함
         -> 시간방향의 smoothing 을 하지 않음
         */
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peak = juce::jmax (peak, std::abs (buffer.getSample (ch, sampleIndex)));
            //외부for문에서는 ++sampleIndex 처리 (이 함수를 호출하는 부분에서의 for문
        
        return peak;
    }
    else  // RMS detectorType 이라면
    {
        /*
         stereo-linked RMS : average instantaneous power across channels, then run it through a one-pole low-pass (rmsCoeff) to get a moving average
         - this is what makes it an "RMS window" rather than an instantaneous value.
         sqrt() at the end converts the averaged power back to an amplitude-like level.
         - 한 샘플 짜리 순간값으로 쓰는게 아니라, 각 채널의 순간 파워의 제곱을 더하고 2로 나눈후에, one-pole low pass 를 먼저 통과시키고(rmsCoeff) -> sqrt()
         */
        
        float sumSquares = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float x = buffer.getSample (ch, sampleIndex);
            sumSquares += x * x;
        }
        const float instantPower = sumSquares / float juce::jmax (numchannels, 1);
            //현재 sampleIndex 의 각 채널의 값 제곱후 평균 - 임시변수
            //현재 한 샘플 시점의 평균 power
            //but, RMS 의 핵심은 "시간 평균". instantPower 는 현재 평균
        
            //여기서는 매번 정확히 N 개의 샘플을 잘라서 평균을 내는대신
            //one-pole LPF 를 이용해서 시간평균을 만드는 방식
        
        rmsMeanSquare = rmsCoeff * rmsMeanSquare + (1.0f - rmsCoeff) * instantPower;
            //우변의 rmsMeanSquare = 이전에 들어있던 평균 power
            //이전평균 _% + 현재평균 (1-_)더해서 평균을 구하는것
            //ex. 이전평균 power 90% + 현재 power 10%
        
        return std::sqrt (rmsMeanSquare);
            //위에서는 Mean Square 를 처리했으니, 이 sqrt 로 루트 처리하는것
        
        
    }
}
