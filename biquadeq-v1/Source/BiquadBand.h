/*
 BiquadBand.h
 
 Design notes:
 - Each BiquadBand instance owns BOTH the coefficients AND the per-channel
   state (x1, x2, y1, y2). This means a stereo signal needs TWO BiquadBand
   instances per filter stage (one per channel) so their histories don't mix.
   See PluginProcessor.h/.cpp for how the instances are allocated per channel.
 - Coefficients are stored already normalized (a0 = 1), same as the Python
   code's final normalization step.
 
 - BiquadBand.h : 이 코드가 각 샘플을 어떻게 계산하는지에 대한 수학적인 실체 코드
 - PluginProcessor.h/.cpp : 언제, 몇번, 무슨 순서로 호출하는가에 대한 기본 이펙터 처리 전반
 */

#pragma once

#include <JuceHeader.h>
#include <cmath>

enum class FilterType
{
    LowPass,
    HighPass,
    BandPass,
    Notch,
    Peaking,
    LowShelf,
    HighShelf
};

class BiquadBand
{
public:
    void prepare (double sampleRate)
    // 오디오 처리를 시작할 준비를 함
    {
        fs = sampleRate;
        reset();
        calculateCoefficients();
    }
}

//Clears the filter's memory(x1, x2, y1, y2)
//Call this whenever playback restarts or a discontinuity is expected

void reset()
{
    x1 = x2 = y1 = y2 = 0.0;
    //필터의 state 초기화 하는 함수
}

//gainDb is only used by peaking / lowshelf / highshelf;
//pass 0.0 for Lowpass / Highpass / Notch
void setParameters (FilterType newType, double newFreq, double newQ, double newGainDb)
    //필터 설정이 바뀌었으니 새로운 설정을 적용함
    //매 processBlock 마다 갱신되는것이 아니고, 값이 변경되면 값 변경을 감지하고 이 함수가 작동함
{
    filterType = newType;
    f0 = juce::jlimit(10.0, fs * 0.49, newFreq);    //stay safely below Nyquist
    Q = juce::jmax (newQ, 0.01);                    //avoid divide-by-zero in alpha
    gainDb = newGainDb;
    calculateCoefficients();
}

// Direct Form I
// y0 = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2
inline float processSample (float xIn) noexcept
{
    const double x0 = (double xIn);
    //xIn 원래 float 형 -> double 형으로 형변환
    
    //y0 이라는 새로운 변수를 생성(선언) 하면서 자료형을 다 지정한것.
    const double y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    
    //이미 만들어져 있는 변수에 값을 대입하고 있는것이라서 자료형을 다시 써주지 않아도 됨
    //이미 다 double 형
    x2 = x1;
    x1 = x0;    // const 로 x0 의 값을 변경하지 못하게 했어도, 그 값을 다른 변수에 넣는건 가능
    y2 = y1;
    y1 = y0;
    // 상태값 업데이트
    
    
    return (float) y0;
}

private:
    void calculateCoefficients()
{
        
    }
