#include "LibsPch.h"
#include "Session.h"
#include "NetworkBase.h"
using namespace C_Utility;

C_Network::IocpEvent::IocpEvent(IocpEventType type) : _type(type), _owner(nullptr) {}

void C_Network::IocpEvent::Reset()
{
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}

/*------------------------------
			Session
------------------------------*/

// new로 했을 때 사용.
C_Network::Session::Session() : _socket(INVALID_SOCKET), _recvEvent(), _sendEvent(), _isDisconn(0),
_targetNetAddr(), _sendFlag(0), _recvBuffer()
{
	InitializeSRWLock(&_sendBufferLock);

	// 비동기로 바뀔 경우 sessinonId의 변경을 atomic하게 해줘야한다.
	static  ULONGLONG sessionId = 0;

	_sessionId = InterlockedIncrement(&sessionId);;

	_ownerServer = std::weak_ptr<C_Network::ServerBase>();
}

C_Network::Session::~Session()
{
}

void C_Network::Session::Init(SOCKET sock, const SOCKADDR_IN* pSockAddr, std::weak_ptr<C_Network::ServerBase> owner)
{
	_socket = sock;
	_targetNetAddr = *pSockAddr;
	_ownerServer = owner;
	_lastTimeStamp = GetTimeStamp();
}

void C_Network::Session::Send(SharedSendBuffer sendBuf)
{
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		_sendBufferQ.push(sendBuf);
	}
	
	PostSend();
}


ErrorCode C_Network::Session::ProcessRecv(DWORD transferredBytes)
{
	_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		Disconnect();
		//printf("정상 종료\n");
		return ErrorCode::NONE; // 정상 종료로 판단.
	}

	if (!_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		printf("Buffer Size Overflow\n");
	
		Disconnect();

		return ErrorCode::RECV_BUF_OVERFLOW;
	}

	uint dataSize = _recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = _recvBuffer.GetUseSize();

		// packetheader보다 작은 상태



#ifdef ECHO
		if (bufferSize < sizeof(PacketHeader::size))
			break;
#else
		if (bufferSize < sizeof(PacketHeader))
			break;
#endif

		PacketHeader header;

		_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

#ifdef ECHO
			if (bufferSize < (sizeof(PacketHeader::size) + header.size))
			break;
#else
			if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;
#endif

#ifdef ECHO
		_recvBuffer.MoveFront(sizeof(header.size));
#else
		_recvBuffer.MoveFront(sizeof(header));
#endif

		if (!_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			wprintf(L"OnRecv Dequeue Error\n");

			return ErrorCode::RECV_BUF_DEQUE_FAILED;
		}

		tempBuffer.MoveRearPos(header.size);

#ifdef ECHO
		OnRecv(tempBuffer, 0);
#else
		OnRecv(tempBuffer, header.type);
#endif
	}

	PostRecv();

	return ErrorCode::NONE;
}

ErrorCode C_Network::Session::ProcessSend(DWORD transferredBytes)
{
	_sendEvent._owner = nullptr;
	_sendEvent._pendingBuffs.clear();
	
	InterlockedExchange8(&_sendFlag, 0);

	if (transferredBytes == 0)
	{
		printf("Send Transferred Bytes is 0\n");
		
		Disconnect();
		return ErrorCode::SEND_LEN_ZERO;
	}

	PostSend();

	return ErrorCode::NONE;
}



bool C_Network::Session::ProcessConnect()
{
	OnConnected();

	PostRecv();

	return false;
}



void C_Network::Session::PostSend()
{
	if (InterlockedExchange8(&_sendFlag, 1) == 1)
		return;

	_sendEvent.Reset();
	_sendEvent._owner = shared_from_this();

	std::vector<WSABUF> wsabufs;
	
	{
		SRWLockGuard lockGuard(&_sendBufferLock);

		int sendBufferCount = _sendBufferQ.size();

		if (sendBufferCount == 0)
		{
			_sendEvent._owner = nullptr;
			
			InterlockedExchange8(&_sendFlag, 0);
			
			return;
		}

		wsabufs.reserve(sendBufferCount);

		while (_sendBufferQ.size())
		{
			SharedSendBuffer sendData = _sendBufferQ.front();

			_sendBufferQ.pop();

			_sendEvent._pendingBuffs.push_back(sendData);
		}
	}
	
	for (auto& sendData : _sendEvent._pendingBuffs)
	{
		WSABUF buf;

		buf.buf = sendData->GetFrontPtr();
		buf.len = sendData->GetDataSize();

		wsabufs.push_back(buf);
	}

	int sendRet = WSASend(_socket, wsabufs.data(), wsabufs.size(), nullptr, 0, reinterpret_cast<LPWSAOVERLAPPED>(&_sendEvent), nullptr);
	
	if (sendRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();
		if (wsaErr != WSA_IO_PENDING)
		{
			_sendEvent._owner = nullptr;

			switch (wsaErr)
			{
			case 10053:;

				// 사용자가 일방적으로 연결을 끊은 경우는 에러 출력을 하지 않도록 하겠다.  WSAECONNRESET
			case 10054:;
				break;
			default:
				printf("WSASend Error - [ errCode %d ]\n", wsaErr);
				break;
			}
			printf("WSASEND DISCONNECT");
			Disconnect();
		}
	}
}

void C_Network::Session::PostRecv()
{
	_recvEvent.Reset();
	_recvEvent._owner = shared_from_this();

	WSABUF buf[2];

	int directEnqueueSize = _recvBuffer.DirectEnqueueSize();
	int remainderSize = _recvBuffer.GetFreeSize()  - directEnqueueSize;
	int wsabufSize = 1;

	buf[0].buf = _recvBuffer.GetFrontBufferPtr();
	buf[0].len = directEnqueueSize;

	if (remainderSize > 0)
	{
		++wsabufSize;
		buf[1].buf = _recvBuffer.GetStartBufferPtr();
		buf[1].len = remainderSize;
	}

	DWORD flag = 0;
 	
	int recvRet = WSARecv(_socket, buf, wsabufSize, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&_recvEvent), nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();

		if (wsaErr != WSA_IO_PENDING)
		{
			_recvEvent._owner = nullptr;

			switch (wsaErr)
			{
				//
			case 10053:
				// 사용자가 일방적으로 연결을 끊은 경우는 에러 출력을 하지 않도록 하겠다. WSAECONNRESET
			case 10054:
				break;
			default:
				printf("WSASend Error - [ errCode %d ]\n", wsaErr);
				break;
			}
			printf("WSARECV DISCONNECT");

			Disconnect();
		}
	}


}
// 강제 종료.
void C_Network::Session::Disconnect()
{
	char isDisconnected = InterlockedExchange8(&_isDisconn, 1);

	if (0 == isDisconnected)
	{
		//printf("Force Disconnect[SESSION ID : %lld]", GetSessionId());
		
		OnDisconnected();

		std::shared_ptr<ServerBase> serverPtr = GetServer();
		
		if(serverPtr != nullptr)
		GetServer()->DeleteSession(GetSessionPtr());

		if (_socket != INVALID_SOCKET)
		{
			printf("Session 종료\n");
			closesocket(_socket);
			_socket = INVALID_SOCKET;
		}
	}
}

std::shared_ptr<C_Network::ServerBase> C_Network::Session::GetServer()
{
	if (_ownerServer.expired())
		return nullptr;

	return _ownerServer.lock();
}

void C_Network::Session::UpdateHeartbeat(ULONGLONG now)
{
	if (now > _lastTimeStamp)
		_lastTimeStamp = now;

}

void C_Network::Session::CheckHeartbeatTimeout(ULONGLONG now)
{
	if (now - _lastTimeStamp > HEARTBEAT_TIMEOUT)
	{
		printf("[Timeout] Session ID: %llu\n", _sessionId); 

		Disconnect();
	}
}



// -----------------------------------------------------------------------------------------------------------------------
SessionPtr C_Network::SessionManager::CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr, HANDLE iocpHandle, std::shared_ptr<C_Network::ServerBase> owner)
{
	if (_sessionCnt == _maxSessionCnt)
	{
		wprintf(L"Session Count is Max");

		return nullptr;
	}
	_sessionCnt.fetch_add(1);

	SessionPtr newSession = _createFunc();

	newSession->Init(sock, pSockAddr, owner);

	if (nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), iocpHandle, 0, 0))
	{
		_sessionCnt.fetch_sub(1);

		wprintf(L"Error.. Session Not Registed in Iocp "); 
			
		return nullptr;
	}
	
	ULONGLONG id = newSession->GetSessionId();
	
	{
		SRWLockGuard lockGuard(&_lock);
		
		_sessionSet.insert(newSession);
	}


	return newSession;
} 

void C_Network::SessionManager::DeleteSession(SessionPtr sessionPtr)
{
	SRWLockGuard lockGuard(&_lock);
	
	_sessionSet.erase(sessionPtr);
	
	_sessionCnt.fetch_sub(1);
}

void C_Network::SessionManager::SetMaxCount(uint maxSessionCnt)
{
	if (_maxSessionCnt == 0)
		_maxSessionCnt = maxSessionCnt;
}

void C_Network::SessionManager::CheckHeartbeatTimeOut(ULONGLONG now)
{
	int sessionCnt = _sessionCnt;

	if (sessionCnt == 0)
		return;

	std::vector<SessionPtr> tempSessions;
	tempSessions.reserve(sessionCnt);
	{
		SRWLockGuard lockGuard(&_lock);

		for (SessionPtr sessionPtr : _sessionSet)
		{
			tempSessions.push_back(sessionPtr);
		}
	}

	for (SessionPtr& sessionPtr : tempSessions)
	{
		sessionPtr->CheckHeartbeatTimeout(now);
	}
}

C_Network::SessionManager::~SessionManager()
{
	SRWLockGuard lockGuard(&_lock);
	
	_sessionSet.clear();
}

