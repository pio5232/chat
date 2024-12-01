#include "LibsPch.h"
#include "NetworkUtils.h"
#include <WS2tcpip.h>

/*--------------------
	  NetAddress
--------------------*/

C_Network::NetAddress::NetAddress(SOCKADDR_IN sockAddr) :_sockAddr(sockAddr) {}


C_Network::NetAddress::NetAddress(std::wstring ip, uint16 port)
{
	memset(&_sockAddr, 0, sizeof(_sockAddr));

	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_port = htons(port);
	_sockAddr.sin_addr = IpToAddr(ip.c_str());
}

C_Network::NetAddress::NetAddress(const NetAddress& other) : _sockAddr(other._sockAddr) {}

void C_Network::NetAddress::Init(SOCKADDR_IN sockAddr)
{
	_sockAddr = sockAddr;
}

const std::wstring C_Network::NetAddress::GetIpAddress() const
{
	WCHAR ipWstr[100];

	// 주소체계, &IN_ADDR
	InetNtopW(AF_INET, &_sockAddr.sin_addr, ipWstr, sizeof(ipWstr) / sizeof(WCHAR));

	return std::wstring(ipWstr);
}

IN_ADDR C_Network::NetAddress::IpToAddr(const WCHAR* ip)
{
	IN_ADDR addr;
	InetPtonW(AF_INET, ip, &addr);
	return addr;
}

C_Network::RecvBuffer::RecvBuffer() : _readPos(0), _writePos(0), _buffer{} {}

thread_local C_Network::SharedSendBufChunk sendBufChunks;

void C_Network::RecvBuffer::Reset()
{
	uint useSize = UseSize();

	if (useSize == 0)
	{
		_readPos = _writePos = 0;
		return;
	}

	// _readPos < _writePos
	uint freeSize = FreeSize();
	if (freeSize < RECV_BUF_CLEAR_SIZE)
	{
		printf("RecvBuffer Memcpy 비용 발생!! 버퍼 크기가 작아요.\n");
		memcpy(&_buffer[0], &_buffer[_readPos], useSize);
		_readPos = 0;
		_writePos = useSize;
	}
}

bool C_Network::RecvBuffer::MoveReadPos(uint transferredBytes)
{
	if(UseSize()< transferredBytes)
		return false;

	_readPos += transferredBytes;
	return true;
}

bool C_Network::RecvBuffer::MoveWritePos(uint len)
{
	if (FreeSize() < len)
		return false;

	_writePos += len;
	return true;
}
