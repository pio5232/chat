#include "LibsPch.h"
#include "CTimer.h"

C_Utility::CTimer::CTimer() : _start{}, _end{}
{
	QueryPerformanceFrequency(&_frequency);

}

void C_Utility::CTimer::Start()
{
	QueryPerformanceCounter(&_start);
}

ULONGLONG C_Utility::GetTimeStamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}