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
