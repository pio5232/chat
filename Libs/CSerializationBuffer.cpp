#include "LibsPch.h"
#include "CSerializationBuffer.h"

using namespace C_Utility;
// 버퍼에서 데이터 크기를 초과한 데이터는 들어가서 터지도록 설계
CSerializationBuffer::CSerializationBuffer(int iBufferSize) :m_iBufferCapacity(iBufferSize), m_iFront(0), m_iRear(0), m_iDataSize(0)
{
	m_chpBuffer = new char[m_iBufferCapacity];
}

CSerializationBuffer::~CSerializationBuffer()
{
	delete[] m_chpBuffer;
}


// 패킷 청소
void CSerializationBuffer::Clear()
{
	m_iDataSize = 0;
	m_iFront = 0;
	m_iRear = 0;
}

bool CSerializationBuffer::Resize()
{
	if (m_iBufferCapacity >= CSERIALIZATION_MAX_SIZE)
		return false;

	int _tempBufferCapacity = m_iBufferCapacity * 2;

	char* _tempBuffer = new char[_tempBufferCapacity];

	errno_t errcpy = memcpy_s(_tempBuffer, _tempBufferCapacity, m_chpBuffer, m_iBufferCapacity);

	if (errcpy) // 실패
	{
		delete[] _tempBuffer;

		return false;
	}

	delete[] m_chpBuffer;

	m_chpBuffer = _tempBuffer;

	m_iBufferCapacity = _tempBufferCapacity;

	return true;
}

// 버퍼 사이즈 얻기
int CSerializationBuffer::GetBufferSize() const
{
	return m_iBufferCapacity;
}

// 현재 사용중인 사이즈 얻기
int CSerializationBuffer::GetDataSize() const
{
	return m_iDataSize;
}

int CSerializationBuffer::GetFreeSize() const
{
	return m_iBufferCapacity - m_iRear;
}
// 버퍼 포인터 얻기
char* CSerializationBuffer::GetBufferPtr() const
{
	return m_chpBuffer;
}

char* CSerializationBuffer::GetRearPtr() const
{
	return &m_chpBuffer[m_iRear];
}

char* CSerializationBuffer::GetFrontPtr() const
{
	return &m_chpBuffer[m_iFront];
}
// 버퍼 Pos 이동
int CSerializationBuffer::MoveRearPos(int iSize)
{
	if (iSize <= 0)
		return 0;

	int _iMoveSize = iSize;

	if (m_iBufferCapacity - m_iDataSize < iSize)
		_iMoveSize = m_iBufferCapacity - m_iDataSize;

	m_iRear += _iMoveSize;
	m_iDataSize += _iMoveSize;
	return _iMoveSize;
}

int CSerializationBuffer::MoveFrontPos(int iSize)
{
	if (iSize <= 0)
		return 0;

	int _iMoveSize = iSize;

	if (m_iDataSize < iSize)
		_iMoveSize = m_iDataSize;

	m_iFront += _iMoveSize;
	m_iDataSize -= _iMoveSize;

	return _iMoveSize;
}
// 여기서 사용하지 말고 로직에서 사용.

int CSerializationBuffer::GetData(char* chpDest, int iSize) // 바깥으로 데이터 빼기, Throw (int), 1 번 (뺄 사이즈가 요청한 사이즈보다 작음.) 
{
	//if (m_iDataSize < iSize)
	//	throw (1);

	errno_t errCpy = memcpy_s(chpDest, iSize, &m_chpBuffer[m_iFront], iSize);

	m_iDataSize -= iSize;
	m_iFront += iSize;

	return iSize;
}
int CSerializationBuffer::PutData(const char* chpSrc, int iSrcSize) // 데이터 넣기, Throw (int) , 0번 (넣을 사이즈 작음)
{
	//if (m_iBufferCapacity - m_iDataSize < iSrcSize)
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, chpSrc, iSrcSize);

	m_iDataSize += iSrcSize;
	m_iRear += iSrcSize;

	return iSrcSize;
}

// << 연산자 오버로딩, Throw (int) , 넣을 사이즈가 작을 때 0번 / 뺄 사이즈가 요청한 사이즈보다 작을 때 1번
CSerializationBuffer& CSerializationBuffer::operator<<(unsigned char ucValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(ucValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&ucValue, sizeof(ucValue));

	m_iDataSize += sizeof(ucValue);
	m_iRear += sizeof(ucValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(char cValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(cValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&cValue, sizeof(cValue));

	m_iDataSize += sizeof(cValue);
	m_iRear += sizeof(cValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator<<(bool bValue)
{
	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&bValue, sizeof(bValue));

	m_iDataSize += sizeof(bValue);
	m_iRear += sizeof(bValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(unsigned short usValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(usValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&usValue, sizeof(usValue));

	m_iDataSize += sizeof(usValue);
	m_iRear += sizeof(usValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(short shValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(shValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&shValue, sizeof(shValue));

	m_iDataSize += sizeof(shValue);
	m_iRear += sizeof(shValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(unsigned int uiValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(uiValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&uiValue, sizeof(uiValue));

	m_iDataSize += sizeof(uiValue);
	m_iRear += sizeof(uiValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(int iValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(iValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&iValue, sizeof(iValue));

	m_iDataSize += sizeof(iValue);
	m_iRear += sizeof(iValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(unsigned long ulValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(ulValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&ulValue, sizeof(ulValue));

	m_iDataSize += sizeof(ulValue);
	m_iRear += sizeof(ulValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(long lValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(lValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&lValue, sizeof(lValue));

	m_iDataSize += sizeof(lValue);
	m_iRear += sizeof(lValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator<<(long long llValue)
{
	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&llValue, sizeof(llValue));

	m_iDataSize += sizeof(llValue);
	m_iRear += sizeof(llValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator<<(unsigned long long ullValue)
{
	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&ullValue, sizeof(ullValue));

	m_iDataSize += sizeof(ullValue);
	m_iRear += sizeof(ullValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator<<(float fValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(fValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&fValue, sizeof(fValue));

	m_iDataSize += sizeof(fValue);
	m_iRear += sizeof(fValue);

	return *this;
}

//CSerializationBuffer& CSerializationBuffer::operator<<(__int64 llValue)
//{
//	//if (m_iBufferCapacity - m_iDataSize < sizeof(llValue))
//	//	throw 0;
//
//	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&llValue, sizeof(llValue));
//
//	m_iDataSize += sizeof(llValue);
//	m_iRear += sizeof(llValue);
//
//	return *this;
//}

CSerializationBuffer& CSerializationBuffer::operator<<(double dValue)
{
	//if (m_iBufferCapacity - m_iDataSize < sizeof(dValue))
	//	throw 0;

	errno_t errCpy = memcpy_s(&m_chpBuffer[m_iRear], m_iBufferCapacity - m_iDataSize, (void*)&dValue, sizeof(dValue));

	m_iDataSize += sizeof(dValue);
	m_iRear += sizeof(dValue);

	return *this;
}


// 잘못된 데이터 혹은 없는 데이터에 대한 처리는 오류를 내는 함수를 만들어서하고
// 컨텐츠 로직에서 잘못된 에러 발생에 대해서 처리
CSerializationBuffer& CSerializationBuffer::operator>>(unsigned char& ucValue)
{
	errno_t errCpy = memcpy_s(&ucValue, sizeof(ucValue), &m_chpBuffer[m_iFront], sizeof(ucValue));

	m_iDataSize -= sizeof(ucValue);
	m_iFront += sizeof(ucValue);

	return *this;
}
CSerializationBuffer& CSerializationBuffer::operator>>(char& cValue)
{
	errno_t errCpy = memcpy_s(&cValue, sizeof(cValue), &m_chpBuffer[m_iFront], sizeof(cValue));

	m_iDataSize -= sizeof(cValue);
	m_iFront += sizeof(cValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator>>(bool& bValue)
{
	errno_t errCpy = memcpy_s(&bValue, sizeof(bValue), &m_chpBuffer[m_iFront], sizeof(bValue));

	m_iDataSize -= sizeof(bValue);
	m_iFront += sizeof(bValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(unsigned short& usValue)
{
	errno_t errCpy = memcpy_s(&usValue, sizeof(usValue), &m_chpBuffer[m_iFront], sizeof(usValue));

	m_iDataSize -= sizeof(usValue);
	m_iFront += sizeof(usValue);

	return *this;
}
CSerializationBuffer& CSerializationBuffer::operator>>(short& shValue)
{
	errno_t errCpy = memcpy_s(&shValue, sizeof(shValue), &m_chpBuffer[m_iFront], sizeof(shValue));

	m_iDataSize -= sizeof(shValue);
	m_iFront += sizeof(shValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(unsigned int& uiValue)
{

	errno_t errCpy = memcpy_s(&uiValue, sizeof(uiValue), &m_chpBuffer[m_iFront], sizeof(uiValue));

	m_iDataSize -= sizeof(uiValue);
	m_iFront += sizeof(uiValue);

	return *this;
}
CSerializationBuffer& CSerializationBuffer::operator>>(int& iValue)
{
	errno_t errCpy = memcpy_s(&iValue, sizeof(iValue), &m_chpBuffer[m_iFront], sizeof(iValue));

	m_iDataSize -= sizeof(iValue);
	m_iFront += sizeof(iValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>>(unsigned long& ulValue)
{
	errno_t errCpy = memcpy_s(&ulValue, sizeof(ulValue), &m_chpBuffer[m_iFront], sizeof(ulValue));

	m_iDataSize -= sizeof(ulValue);
	m_iFront += sizeof(ulValue);

	return *this;
}
CSerializationBuffer& CSerializationBuffer::operator>>(long& lValue)
{
	errno_t errCpy = memcpy_s(&lValue, sizeof(lValue), &m_chpBuffer[m_iFront], sizeof(lValue));

	m_iDataSize -= sizeof(lValue);
	m_iFront += sizeof(lValue);

	return *this;
}

CSerializationBuffer& CSerializationBuffer::operator>> (float& fValue)
{
	errno_t errCpy = memcpy_s(&fValue, sizeof(fValue), &m_chpBuffer[m_iFront], sizeof(fValue));

	m_iDataSize -= sizeof(fValue);
	m_iFront += sizeof(fValue);

	return *this;
}
//
//CSerializationBuffer& CSerializationBuffer::operator>> (__int64& llValue)
//{
//	errno_t errCpy = memcpy_s(&llValue, sizeof(llValue), &m_chpBuffer[m_iFront], sizeof(llValue));
//
//	m_iDataSize -= sizeof(llValue);
//	m_iFront += sizeof(llValue);
//
//	return *this;
//}
CSerializationBuffer& CSerializationBuffer::operator>> (double& dValue)
{
	errno_t errCpy = memcpy_s(&dValue, sizeof(dValue), &m_chpBuffer[m_iFront], sizeof(dValue));

	m_iDataSize -= sizeof(dValue);
	m_iFront += sizeof(dValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator>>(long long& llValue)
{
	errno_t errCpy = memcpy_s(&llValue, sizeof(llValue), &m_chpBuffer[m_iFront], sizeof(llValue));

	m_iDataSize -= sizeof(llValue);
	m_iFront += sizeof(llValue);

	return *this;
}

CSerializationBuffer& C_Utility::CSerializationBuffer::operator>>(unsigned long long& ullValue)
{
	errno_t errCpy = memcpy_s(&ullValue, sizeof(ullValue), &m_chpBuffer[m_iFront], sizeof(ullValue));

	m_iDataSize -= sizeof(ullValue);
	m_iFront += sizeof(ullValue);

	return *this;
}
