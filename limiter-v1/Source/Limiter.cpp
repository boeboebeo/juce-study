/*
    Limiter.cpp
 */

#include "Limiter.h"

//SlidingMinimum
void SlidingMinimum::prepare(int newWindowSize)
{
    windowSize = juce::jmax (1, newWindowSize); //윈도우 크기가 최소 1은 되게 함
    capacity = windowSize + 1;
        //worst case : widowSize + 1 candidates before expiry
        //내부 자료구조가 처리중에 필요한 여유공간 확보 (새후보를 먼저 추가하고, 오래된 후보를 나중에 제거하는 순서라면 잠깐 동안 후보가 6개 일 수 있음)
    
    values.assign ((size_t) capacity, 1.0f);
        //values = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
        //최솟값 후보가 가진 실제값 저장
    indices.assign ((size_t) capacity, 0);
        // indices = {0, 0, 0, 0, 0, 0}
        //그 후보가 들어온 샘플의 인덱스 저장
    reset();
}

void SlidingMinimum::reset()
{
    head = 0; //후보중 가장 앞에 있는 값의 위치
    count = 0;  //현재 관리하고 있는 후보의 개수
    sampleCounter = 0;  //현재 샘플의 번호
}

float SlidingMinimum::process (float x)
//const float windowMin = slidingMin.process(releaseState); 이렇게 호출받아 쓸 예정
{
    while (count > 0) //현재관리하고 있는 후보의 개수가 > 0이라면 마지막 후보의 위치를 찾음
    {
        const int back = (head + count - 1) % capacity;
        //모듈로 : 배열의 끝을 넘어가면 처음으로 다시 돌아오게 하기 위해서
        
        if (values[(size_t) back] >= x)
            --count;
            //values 덱 안의 마지막 후보(back)가 새 값보다 크거나 같은지 확인
            //더 크다면 count 를 1줄여서 마지막 후보를 덱에서 제외함
            //그리고 그 자리에 새 값 x 를 저장하면 됨
        else
            break;
    }
    
    //2. push the new value.
    const int pos = (head + count) % capacity;
        //덱의 마지막 후도 다음에 사용할 위치, 새 값 저장할 위치
    
    values[(size_t) pos] = x;   //그 위치에 x 저장
    indices[(size_t) pos] = sampleCounter; //샘플 번호 저장
    ++count;    //새로운 후보 추가되었으니 현재 관리하는 후보의 개수를 하나 늘림
    
    
    //3.drop the front if it has slid out of the window(너무 오래되어서 현재 윈도우 밖으로 밀려난 후보를 앞에서 제거)
    while (indices[(size_t) head] <= sampleCounter - windowSize)
    {
        head = (head + 1) % capacity; //head 를 옮기면 그 앞의 위치의 값은 더이상 현재 덱의 후보로 취급되지 않음
        --count;
    }
    
    ++sampleCounter //다음 샘플 처리할 준비 (그 다음 호출에서는 다음 샘플 처리)
    
    //the front of the deque is the minimum of the current window
    return values[(size_t) head];
        //새로운 값보다 크거나 같은 뒤쪽 후보는 제거함
        //현재 윈도우에서 만료된 앞쪽 후보들도 제거함
        //값은 오름차운으로 저장되어있으므로, 맨 앞의 후보가 현재 윈도우 내에서의 최솟값이 되는것 - 그걸 windowMin 에 저장함
}
