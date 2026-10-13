/*
 Limiter.h
 
 Signal flow (per sample):
    
    input * inputGain
        |
        +--> delay line (lookahead) -------------------------+
        |                                                    |
        +--> stereo-linked peak detector                     |
        |
        required gain g[n] = min(1, ceiling/peak)
        |
        release stage r[n] = (instant down, exponential up)
        |
        sliding MINIMUM over the lookahead window m[n]
            // 큰 피크를 제한하기 위해 gain 을 0.5까지 낮췄다면,
            // 그 피크가 실제 출력될때까지 gain 이 너무 빨리 회복되지 않도록 낮은 값 유지함
            // (release 때문에 gain 이 회복되기 시작하면 lookahead 라는 delay 때문에 시점이 다름, 그 딜레이 샘플들 각각의 g[n] 중 제일 작은 걸 선택)
        |
        moving AVERAGE over the lookahead window s[n]-------+
            // gain 변화를 부드럽게 만듦

                                                            |
                                                         ouput (<-ceiling)
 
    lookahead : L
    - The audio is delayed by L samples,
    - 출력시점 n 에 나오는 오디오는 L 의 영향으로 과거샘플 x[n - L]임.
 
    the gain ramp is spread over(-에 분산하다) the lookahead window, the
    attack is "as fast as the lookahead allows" and is not a separate user parameter
 
 */
