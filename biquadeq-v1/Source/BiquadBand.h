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
        //step1: omega0 -- cutoff expressed in rad/sample
        const double w0 = 2.0 * juce::MathConstants<double>::pi * (f0 / fs);
            //w0 = omega. 디지털 각주파수(rad/sample)
            //ex. cutoff = 1000
            //w0 = 2*pi*(1000/48000)
        
        //step2: alpha -- analog Q term folded through the bilinear transform
        const double alpha = std::sin (w0) / (2.0 * Q);
            //alpha 계산시 sin(w0) 필요
            //coeffiecients 계산시 cos(wo) 필요
            //아래의 계수 계산식 내에서 std::cos(w0)로 적으면 계속 계산해야하기때문에,
            //한번 계산해놓고, 재사용함
        
        const double cosw0 = std::cos (w0);
        
        //Only used by peaking/shelving filters
        //dB -> linear gain, sqrt for shelf slope
        const double A = std::pow (10.0, gainDb / 40.0);
        const double sqrtA = std::sqrt (A);
            //중간 값 A
        
        double rb0 = 1.0, rb1 = 0.0, rb2 = 0.0;
        double ra0 = 1.0, ra1 = 0.0, ra2 = 0.0;
        //중간계수 초기화. 마지막에는 r빼고 b0, b1..으로 리턴할 예정
        
        switch (filterType)
        {
            case FilterType::LowPass:
                rb0 = (1.0 - cosw0) / 2.0;
                rb1 = 1.0 - cosw0;
                rb2 = (1.0 - cosw0) / 2.0;
                ra0 = 1.0 + alpha;
                ra1 = -2.0 * cosw0;
                ra2 = 1.0 - alpha;
                break;
                //스위치 빠져나오기
                
            case FilterType::HighPass:
                 rb0 =  (1.0 + cosw0) / 2.0;
                 rb1 = -(1.0 + cosw0);
                 rb2 =  (1.0 + cosw0) / 2.0;
                 ra0 =  1.0 + alpha;
                 ra1 = -2.0 * cosw0;
                 ra2 =  1.0 - alpha;
                 break;
  
             case FilterType::BandPass:
                 rb0 =  alpha;
                 rb1 =  0.0;
                 rb2 = -alpha;
                 ra0 =  1.0 + alpha;
                 ra1 = -2.0 * cosw0;
                 ra2 =  1.0 - alpha;
                 break;
  
             case FilterType::Notch:
                 rb0 =  1.0;
                 rb1 = -2.0 * cosw0;
                 rb2 =  1.0;
                 ra0 =  1.0 + alpha;
                 ra1 = -2.0 * cosw0;
                 ra2 =  1.0 - alpha;
                 break;
  
             case FilterType::Peaking:
                 rb0 =  1.0 + alpha * A;
                 rb1 = -2.0 * cosw0;
                 rb2 =  1.0 - alpha * A;
                 ra0 =  1.0 + alpha / A;
                 ra1 = -2.0 * cosw0;
                 ra2 =  1.0 - alpha / A;
                 break;
  
             case FilterType::LowShelf:
                 rb0 =        A * ((A + 1.0) - (A - 1.0) * cosw0 + 2.0 * sqrtA * alpha);
                 rb1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * cosw0);
                 rb2 =        A * ((A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqrtA * alpha);
                 ra0 =            (A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqrtA * alpha;
                 ra1 =      -2.0 * ((A - 1.0) + (A + 1.0) * cosw0);
                 ra2 =            (A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqrtA * alpha;
                 break;
  
             case FilterType::HighShelf:
                 rb0 =        A * ((A + 1.0) + (A - 1.0) * cosw0 + 2.0 * sqrtA * alpha);
                 rb1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw0);
                 rb2 =        A * ((A + 1.0) + (A - 1.0) * cosw0 - 2.0 * sqrtA * alpha);
                 ra0 =            (A + 1.0) - (A - 1.0) * cosw0 + 2.0 * sqrtA * alpha;
                 ra1 =       2.0 * ((A - 1.0) - (A + 1.0) * cosw0);
                 ra2 =            (A + 1.0) - (A - 1.0) * cosw0 - 2.0 * sqrtA * alpha;
                 break;
                
        //여기까지가 calculateCoefficients() 함수의 내용
        //아래는 그 바깥임
        //calculateCoefficients() 는 void 형이기 때문에 그냥 이 객체안의 b0 ~ a2를 업데이트하기만 함 -> 딱히 return 안함
         }
        
        //Normalize by a0
        b0 = rb0/ra0;
        b1 = rb1/ra0;
        b2 = rb2/ra0;
        a1 = ra1/ra0;
        a2 = ra2/ra0;
        }

FilterType filtertype = FilterType::LowPass;
    //처음 실행시에는 FilterType = LowPass 로 셋팅되어 있음
    //FilterType 이라는 자료형의 filtertype 이라는 멤버변수 하나를 만들자
    //근데 그걸 그냥 초기값은 LowPass 로 설정

double f0 = 1000.0, Q = 0.707, gainDb = 0.0, fs = 48000.0;
    //초기값
    //fs 는 바깥에서 받아서 쓰지만, 그냥 fs라는 공간을 하나 만들어 두는것.

//Normalized (a0 = 1) coefficients used by processSample()
double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    //계수 초기값

//Direct Form I state -- per-instance, so per-channel when you allocate
//One BiquadBand per channel
double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
    //초기값
        
};
