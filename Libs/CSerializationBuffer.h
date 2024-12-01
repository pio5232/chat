#pragma once

namespace C_Utility
{
#define CSERIALIZATION_DEFAULT_SIZE 100
#define CSERIALIZATION_MAX_SIZE 1600
	class CSerializationBuffer
	{
	public:
		CSerializationBuffer(int iBufferSize = CSERIALIZATION_DEFAULT_SIZE);
		~CSerializationBuffer();
		// 패킷 청소
		void Clear();

		// 리사이즈.
		bool Resize();

		//버퍼 포인터 얻기
		char* GetBufferPtr() const;

		// 버퍼 사이즈 얻기
		int GetBufferSize() const;

		// 현재 사용중인 사이즈 얻기
		int GetDataSize() const;

		int GetFreeSize() const;

		int GetData(char* chpDest, int iSize);
		int PutData(const char* chpSrc, int iSrcSize);

		// 연산자 오버로딩
		CSerializationBuffer& operator<<(unsigned char ucValue);
		CSerializationBuffer& operator<<(char cValue);

		CSerializationBuffer& operator<<(unsigned short usValue);
		CSerializationBuffer& operator<<(short shValue);

		CSerializationBuffer& operator<<(unsigned int uiValue);
		CSerializationBuffer& operator<<(int iValue);

		CSerializationBuffer& operator<<(unsigned long ulValue);
		CSerializationBuffer& operator<<(long lValue);

		CSerializationBuffer& operator<< (long long llValue);
		CSerializationBuffer& operator<< (unsigned long long ullValue);

		CSerializationBuffer& operator<< (float fValue);

		//CSerializationBuffer& operator<< (__int64 llValue);
		CSerializationBuffer& operator<< (double dValue);

		CSerializationBuffer& operator>>(unsigned char& ucValue);
		CSerializationBuffer& operator>>(char& cValue);

		CSerializationBuffer& operator>>(unsigned short& usValue);
		CSerializationBuffer& operator>>(short& shValue);

		CSerializationBuffer& operator>>(unsigned int& uiValue);
		CSerializationBuffer& operator>>(int& iValue);

		CSerializationBuffer& operator>>(unsigned long& ulValue);
		CSerializationBuffer& operator>>(long& lValue);
		
		CSerializationBuffer& operator>> (float& fValue);

		//CSerializationBuffer& operator>> (__int64& llValue);
		CSerializationBuffer& operator>> (double& dValue);

		CSerializationBuffer& operator>> (unsigned long long& ullValue);
		CSerializationBuffer& operator>> (long long& llValue);

		// 버퍼 포인터 얻기
		char* GetRearPtr() const;
		char* GetFrontPtr() const;
		// 버퍼 Pos 이동
		int MoveRearPos(int iSize);
		int MoveFrontPos(int iSize);
	private:

	public:
		int m_iBufferCapacity;
		int m_iDataSize;
		int m_iFront;
		int m_iRear;
		char* m_chpBuffer;
	};
}



//#endif // !CSerialization