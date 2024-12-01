#pragma once

//#ifndef RINGBUFFER_EXPORT
//#define RINGBUFFER_DLLSPEC //__declspec(dllexport)
//#else
//#define RINGBUFFER_DLLSPEC //__declspec(dllimport)
//#endif
;

namespace C_Utility
{
#define RINGBUFFER_DEFAULT_SIZE 10000
	class CRingBuffer
	{
	public:
		CRingBuffer();
		CRingBuffer(int iCapacity);
		~CRingBuffer();
		bool Resize(int newCapacity);
		int GetCapacity(); // 총 용량
		int GetUseSize(); // 사용 용량
		int GetFreeSize(); // 남은 용량

		int DirectEnqueueSize();
		int DirectDequeueSize();

		bool EnqueueRetBool(char* chpData, int iSize);
		bool DequeueRetBool(char* chpDest, int iSize);
		bool PeekRetBool(char* chpDest, int iSize);

		int Enqueue(char* chpData, int iSize);
		int Dequeue(char* chpDest, int iSize);
		int Peek(char* chpDest, int iSize);

		int MoveRear(int iSize);
		int MoveFront(int iSize);

		bool MoveRearRetBool(int iSize);
		bool MoveFrontRetBool(int iSize);

		void ClearBuffer(void);
		char* GetFrontBufferPtr();
		char* GetRearBufferPtr();
		char* GetStartBufferPtr();

		bool IsEmpty();
		bool IsFull();
	public:
		int m_iCapacity;
		int m_iSize;
		int m_front;
		int m_rear;
		int m_count = 0;
		char* m_pBuffer;
	};
}
