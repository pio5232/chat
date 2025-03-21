#pragma once

namespace C_Utility
{
	class CTimer
	{
	public:
		CTimer();
		void Start();

		// end - start 
		template<typename T = double>
		T End()
		{
			QueryPerformanceCounter(&_end);

			T ret = static_cast<T>(_end.QuadPart - _start.QuadPart) / _frequency.QuadPart;

			_start.QuadPart = 0;

			return ret;
		}

		// end - start
		// start = end
		template<typename T = double>
		T Lap()
		{
			QueryPerformanceCounter(&_end);

			T ret = static_cast<T>(_end.QuadPart - _start.QuadPart) / _frequency.QuadPart;

			_start = _end;

			return ret;
		}
	private:
		LARGE_INTEGER _start;
		LARGE_INTEGER _end;
		LARGE_INTEGER _frequency;

	};

	// 1970_01_01 ���� �ð� ms�� ������
	ULONGLONG GetTimeStamp();
}

