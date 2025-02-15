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
C_Network::Session::Session(SOCKET sock,const SOCKADDR_IN* pSockAddr) : _socket(sock), _recvEvent(), _sendEvent(),_connectEvent(),
_targetNetAddr(*pSockAddr), _sendFlag(0), _recvBuffer(),_ioCount(0), _isDisconn(0)
{
	InitializeSRWLock(&_sendBufferLock);

	// 비동기로 바뀔 경우 sessinonId의 변경을 atomic하게 해줘야한다.
	static  ULONGLONG sessionId = 0;

	_sessionId = InterlockedIncrement(&sessionId);;
}

C_Network::Session::~Session()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

void C_Network::Session::Send(SharedSendBuffer sendBuf)
{
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		_sendBufferQ.push(sendBuf);
	}
	
	PostSend();
}


C_Network::NetworkErrorCode C_Network::Session::ProcessSend(DWORD transferredBytes)
{
	_sendEvent._owner = nullptr;
	_sendEvent._pendingBuffs.clear();
	
	InterlockedExchange8(&_sendFlag, 0);

	if (transferredBytes == 0)
	{
		printf("Send Transferred Bytes is 0\n");
		return C_Network::NetworkErrorCode::SEND_LEN_ZERO;
	}

	PostSend();

	return C_Network::NetworkErrorCode::NONE;
}


// --------------------------------------------------- ASynchronization ---------------------------------------------------
bool C_Network::Session::ProcessConnect()
{
	_connectEvent._owner = nullptr;

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessAccept()
{	
	PostRecv();

	return false;
}

bool C_Network::Session::ProcessDisconnect()
{
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

	InterlockedIncrement(&_ioCount);

	int sendRet = WSASend(_socket, wsabufs.data(), wsabufs.size(), nullptr, 0, reinterpret_cast<LPWSAOVERLAPPED>(&_sendEvent), nullptr);
	
	if (sendRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();
		if (wsaErr != WSA_IO_PENDING)
		{
			_sendEvent._owner = nullptr;
			InterlockedDecrement(&_ioCount);

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
 	
	InterlockedIncrement(&_ioCount);
	int recvRet = WSARecv(_socket, buf, wsabufSize, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&_recvEvent), nullptr);

	if (recvRet == SOCKET_ERROR)
	{
		DWORD wsaErr = WSAGetLastError();

		if (wsaErr != WSA_IO_PENDING)
		{
			_recvEvent._owner = nullptr;
			InterlockedDecrement(&_ioCount);

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
		}
	}


}
// --------------------------------------------------- ASynchronization ---------------------------------------------------
void C_Network::Session::PostAccept()
{

}

// 강제 종료.
void C_Network::Session::Disconnect()
{
	char isDisconnected = InterlockedExchange8(&_isDisconn, 1);

	if (0 == isDisconnected)
	{
		printf("Force Disconnect[SESSION ID : %lld]", GetId());

		closesocket(_socket);

		_socket = INVALID_SOCKET;
	}
}

bool C_Network::Session::CheckDisconnect()
{
	ULONG ioCount = InterlockedDecrement(&_ioCount);

	if (ioCount == 0)
	{
		char isDisconnected = InterlockedExchange8(&_isDisconn, 1);

		if (0 == isDisconnected)
		{
			printf("Disconnect [SESSION ID : %lld]\n", GetId());

			return true;
		}
	}
	return false;
}


void C_Network::Session::PostConnect()
{

}

// -----------------------------------------------------------------------------------------------------------------------



SharedSession C_Network::SessionManager::CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr, HANDLE iocpHandle)
{
	if (_sessionCnt == _maxSessionCnt)
	{
		wprintf(L"Session Count is Max");

		return nullptr;
	}
	_sessionCnt.fetch_add(1);

	SharedSession newSession = std::make_shared<C_Network::Session>(sock, pSockAddr);

	if (nullptr == CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), iocpHandle, 0, 0))
	{
		_sessionCnt.fetch_sub(1);

		wprintf(L"Error.. Session Not Registed in Iocp "); 
			
		return nullptr;
	}
	
	ULONGLONG id = newSession->GetId();
	
	{
		SRWLockGuard lockGuard(&_lock);
		
		_sessionToIdDic.insert(std::make_pair(newSession, id));
		_idToSessionDic.insert(std::make_pair(id, newSession));
	}


	return newSession;
} 

void C_Network::SessionManager::DeleteSession(SharedSession session)
{
	ULONGLONG sessionId = session->GetId();

	{
		SRWLockGuard lockGuard(&_lock);
		_sessionToIdDic.erase(session);
		_idToSessionDic.erase(sessionId);
	}
	_sessionCnt.fetch_sub(1);
}

C_Network::SessionManager::~SessionManager()
{
	SRWLockGuard lockGuard(&_lock);
	
	for (auto iter = _sessionToIdDic.begin(); iter != _sessionToIdDic.end(); )
	{
		_idToSessionDic.erase(iter->second);
		iter = _sessionToIdDic.erase(iter);
	}
}

SharedSession C_Network::SessionManager::GetSession(ULONGLONG sessionId)
{
	SRWSharedLockGuard lockGuard(&_lock);

	std::unordered_map<ULONGLONG, SharedSession>::iterator iter = _idToSessionDic.find(sessionId);

	if (iter != _idToSessionDic.end())
		return iter->second;
	else
		return nullptr;

	
}
