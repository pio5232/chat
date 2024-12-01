#include "LibsPch.h"
#include "CRingBuffer.h"

using namespace C_Utility;

C_Utility::CRingBuffer::CRingBuffer() :m_iCapacity(RINGBUFFER_DEFAULT_SIZE), m_front(0), m_rear(0), m_iSize(0)
{
	m_pBuffer = new char[m_iCapacity];
}


CRingBuffer::CRingBuffer(int iCapacity) : m_front(0), m_rear(0), m_iSize(0)
{
	m_iCapacity = iCapacity;
	m_pBuffer = new char[m_iCapacity];
}

CRingBuffer::~CRingBuffer()
{
	delete[] m_pBuffer;
}

// resize는 무조건 큰 사이즈로만.
bool CRingBuffer::Resize(int newCapacity)
{
	if (newCapacity < m_iCapacity)
		return false;

	char* _tmpBuffer = new char[newCapacity];

	int _iBufferSize = GetUseSize();
	bool bRet = Dequeue(_tmpBuffer, _iBufferSize);

	delete m_pBuffer;

	// 새로운 버퍼에 대한 포인터 재구성.
	m_pBuffer = _tmpBuffer;

	m_iCapacity = newCapacity;
	m_front = 0;
	m_rear = _iBufferSize;

	return bRet;
}

// Capcity return
int CRingBuffer::GetCapacity()
{
	return m_iCapacity;
}

int CRingBuffer::GetUseSize()
{
	int front = m_front;
	int rear = m_rear;

	if (front == (rear + 1) % m_iCapacity)
		return m_iCapacity - 1;

	if (rear >= front)
		return rear - front;

	else
		return m_iCapacity - (front - rear);
}

int CRingBuffer::GetFreeSize()
{
	int front = m_front;
	int rear = m_rear;

	// 이것만 수정함
	if (front == (rear + 1) % m_iCapacity)
		return 0;

	else if (rear >= front)
		return m_iCapacity - (rear - front) - 1;
	else
		return front - rear - 1;
}

// 결국 empty <-> full은 m_rear == m_front 동일
int CRingBuffer::DirectEnqueueSize() // 
{
	int front = m_front;
	int rear = m_rear;

	if (front == (rear + 1) % m_iCapacity)
		return 0;

	if (rear >= front)
	{
		if (front == 0)
			return m_iCapacity - rear - 1;

		return m_iCapacity - rear;
	}
	else
		return front - rear - 1;
}

int CRingBuffer::DirectDequeueSize()
{
	int front = m_front;
	int rear = m_rear;

	if (rear >= front)
		return rear - front;

	return m_iCapacity - front;
}
// =============================================

// 내 정책 -> 링버퍼에 들어갈 수 있는 크기가 아니면 넣거나 꺼내지 못하도록 한다.
// 리턴 값은 성공 또는 실패로 지정한 사이즈를 넣거나 꺼내는 것에 대한 결과를 전달한다.
// 성공했을 때에는 인자로 넣은 iSize만큼 모두 넣거나 꺼내는 작업을 성공한 것이다.
bool CRingBuffer::EnqueueRetBool(char* chpData, int iSize)
{
	if (GetFreeSize() < iSize) // 남아있는 용량 (== GetFreeSize)이 집어넣으려는 용량보다 작으면 실패 return;
	{
		return false;
	}

	int direct_enqueue_size = DirectEnqueueSize();

	if (direct_enqueue_size < iSize)
	{
		memcpy_s(&m_pBuffer[m_rear], direct_enqueue_size, chpData, direct_enqueue_size);
		MoveRear(direct_enqueue_size);

		int extra_enqueue_size = iSize - direct_enqueue_size;

		memcpy_s(&m_pBuffer[m_rear], extra_enqueue_size, chpData + direct_enqueue_size, extra_enqueue_size);
		MoveRear(extra_enqueue_size);

		m_count++;
	}
	else
	{
		memcpy_s(&m_pBuffer[m_rear], iSize, chpData, iSize);
		MoveRear(iSize);
	}

	return true;
}

bool CRingBuffer::DequeueRetBool(char* chpDest, int iSize) // 저장되어 있는 용량 (== GetUseSize)이 빼려는 용량보다 작으면 실패 return;
{
	if (GetUseSize() < iSize)
	{
		return false;
	}

	int direct_dequeue_size = DirectDequeueSize();

	if (iSize > direct_dequeue_size)
	{
		memcpy_s(chpDest, direct_dequeue_size, &m_pBuffer[m_front], direct_dequeue_size);
		MoveFront(direct_dequeue_size);

		int extra_dequeue_size = iSize - direct_dequeue_size;
		memcpy_s(chpDest + direct_dequeue_size, extra_dequeue_size, &m_pBuffer[m_front], extra_dequeue_size);
		MoveFront(extra_dequeue_size);
	}
	else
	{
		memcpy_s(chpDest, iSize, &m_pBuffer[m_front], iSize);
		MoveFront(iSize);
	}
	return true;
}

bool CRingBuffer::PeekRetBool(char* chpDest, int iSize) // Deque와 마찬가지로 성공,실패 반환. 임시 프론트 활용.
{

	if (GetUseSize() < iSize)
	{
		return false;
	}

	//int direct_dequeue_size = DirectDequeueSize();

	//memcpy_s(chpDest, direct_dequeue_size, &m_pBuffer[m_front], direct_dequeue_size);

	//int extra_dequeue_size = iSize - direct_dequeue_size;
	//memcpy_s(chpDest + direct_dequeue_size, extra_dequeue_size, m_pBuffer, extra_dequeue_size);

	int direct_dequeue_size = DirectDequeueSize();

	if (iSize > direct_dequeue_size)
	{
		memcpy_s(chpDest, direct_dequeue_size, &m_pBuffer[m_front], direct_dequeue_size);
		int prev_front = m_front;
		MoveFrontRetBool(direct_dequeue_size);

		int extra_dequeue_size = iSize - direct_dequeue_size;
		memcpy_s(chpDest + direct_dequeue_size, extra_dequeue_size, &m_pBuffer[m_front], extra_dequeue_size);

		m_front = prev_front;
	}
	else
	{
		memcpy_s(chpDest, iSize, &m_pBuffer[m_front], iSize);
	}
	return true;
}

//===== 막 집어넣는 경우.
int CRingBuffer::Enqueue(char* chpData, int iSize)
{
	int _freeSize = GetFreeSize();
	int enqueueSize = iSize;

	if (_freeSize < iSize)
	{
		enqueueSize = _freeSize;
	}

	int direct_enqueue_size = DirectEnqueueSize();

	if (direct_enqueue_size < enqueueSize)
	{
		memcpy_s(&m_pBuffer[m_rear], direct_enqueue_size, chpData, direct_enqueue_size);
		MoveRear(direct_enqueue_size);

		int extra_enqueue_size = enqueueSize - direct_enqueue_size;
		m_count++;

		memcpy_s(&m_pBuffer[m_rear], extra_enqueue_size, chpData + direct_enqueue_size, extra_enqueue_size);
		MoveRear(extra_enqueue_size);
	}
	else
	{
		memcpy_s(&m_pBuffer[m_rear], enqueueSize, chpData, enqueueSize);
		MoveRear(enqueueSize);
	}

	//m_iSize += enqueueSize;
	return enqueueSize;
}
int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	int use_size = GetUseSize();
	int dequeueSize = iSize;

	if (use_size < iSize)
		dequeueSize = use_size;

	int direct_dequeue_size = DirectDequeueSize();

	if (dequeueSize > direct_dequeue_size)
	{
		memcpy_s(chpDest, direct_dequeue_size, &m_pBuffer[m_front], direct_dequeue_size);
		MoveFront(direct_dequeue_size);

		int extra_dequeue_size = dequeueSize - direct_dequeue_size;
		memcpy_s(chpDest + direct_dequeue_size, extra_dequeue_size, &m_pBuffer[m_front], extra_dequeue_size);
		MoveFront(extra_dequeue_size);
	}
	else
	{
		memcpy_s(chpDest, dequeueSize, &m_pBuffer[m_front], dequeueSize);
		MoveFront(dequeueSize);
	}

	//m_iSize -= dequeueSize;
	return dequeueSize;
}
int CRingBuffer::Peek(char* chpDest, int iSize)
{
	int peekSize = iSize;

	int useSize = GetUseSize();
	if (useSize < iSize)
	{
		peekSize = useSize;
	}

	int direct_dequeue_size = DirectDequeueSize();

	if (peekSize > direct_dequeue_size)
	{
		memcpy_s(chpDest, direct_dequeue_size, &m_pBuffer[m_front], direct_dequeue_size);
		int prev_front = m_front;
		MoveFrontRetBool(direct_dequeue_size);

		int extra_dequeue_size = peekSize - direct_dequeue_size;
		memcpy_s(chpDest + direct_dequeue_size, extra_dequeue_size, &m_pBuffer[m_front], extra_dequeue_size);

		m_front = prev_front;
	}
	else
	{
		memcpy_s(chpDest, peekSize, &m_pBuffer[m_front], peekSize);
	}

	return peekSize;
}
int CRingBuffer::MoveRear(int iSize)
{
	int _imoveSize = iSize;
	int free_size = GetFreeSize();
	if (free_size < iSize) // 남아있는 용량 (== GetFreeSize)이 이동시키려는 용량보다 작으면 실패 return;
	{
		_imoveSize = m_iCapacity - free_size;
	}

	m_rear = (m_rear + _imoveSize) % m_iCapacity;

	//m_iSize += _imoveSize;
	return _imoveSize;
}

int CRingBuffer::MoveFront(int iSize)
{
	int _imoveSize = iSize;
	int use_size = GetUseSize();
	if (use_size < iSize)
	{
		_imoveSize = use_size;
	}

	m_front = (m_front + _imoveSize) % m_iCapacity;

	//m_iSize -= _imoveSize;
	return _imoveSize;
}

bool CRingBuffer::MoveRearRetBool(int iSize)
{
	if (GetFreeSize() < iSize) // 남아있는 용량 (== GetFreeSize)이 이동시키려는 용량보다 작으면 실패 return;
	{
		return false;
	}

	m_rear = (m_rear + iSize) % m_iCapacity;

	//m_iSize += iSize;
	return true;
}

bool CRingBuffer::MoveFrontRetBool(int iSize)
{
	if (GetUseSize() < iSize)
	{
		return false;
	}

	m_front = (m_front + iSize) % m_iCapacity;

	//m_iSize -= iSize;
	return true;
}

void CRingBuffer::ClearBuffer()
{
	m_front = m_rear;


	//m_iSize = 0;
}

char* CRingBuffer::GetFrontBufferPtr()
{
	return &m_pBuffer[m_front];
}

char* CRingBuffer::GetRearBufferPtr()
{
	return &m_pBuffer[m_rear];
}

char* CRingBuffer::GetStartBufferPtr()
{
	return m_pBuffer;
}
bool CRingBuffer::IsEmpty()
{
	return m_rear == m_front;
}

bool CRingBuffer::IsFull()
{
	return m_front == (m_rear + 1) % m_iCapacity;
}