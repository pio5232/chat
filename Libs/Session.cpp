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

// Pool ���·� ����� ���� ����.
C_Network::Session::Session() : _socket(INVALID_SOCKET), _recvEvent(), _sendEvent(), _connectEvent(), _disconnEvent(), _ioCount(0),
_netAddr{}, _sendFlag(0), _recvBuffer(), _sessionId(ULLONG_MAX)
{
	InitializeSRWLock(&_sendBufferLock);
}

// new�� ���� �� ���.
C_Network::Session::Session(SOCKET sock, SOCKADDR_IN* pSockAddr) : _socket(sock), _recvEvent(), _sendEvent(),_connectEvent(), _disconnEvent(),
_netAddr(*pSockAddr), _sendFlag(0),_ioCount(0), _recvBuffer() //_isConnected(1),
{
	InitializeSRWLock(&_sendBufferLock);

	TODO_UPDATE_EX_LIST;
	// �񵿱�� �ٲ� ��� sessinonId�� ������ atomic�ϰ� ������Ѵ�.
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

void C_Network::Session::Init(SOCKET sock, SOCKADDR_IN* pSockAddr)
{
	_socket = sock;
	_netAddr.Init(*pSockAddr);
	//_isConnected = 1;
	_recvBuffer.ClearBuffer();

	
	while (_sendBufferQ.size())
	{
		_sendBufferQ.pop();
	}
	_sendEvent._pendingBuffs.clear();

	InterlockedExchange8(&_sendFlag, 0);
	InterlockedExchange(&_ioCount, 0);

	static  ULONGLONG sessionId = 0;

	_sessionId = InterlockedIncrement(&sessionId);;
}

void C_Network::Session::Send(SharedSendBuffer sendBuf)
{
	{
		SRWLockGuard lockGuard(&_sendBufferLock);
		_sendBufferQ.push(sendBuf);
	}
	
	PostSend();
}

bool C_Network::Session::CanDisconnect()
{	
	////char isConnected = InterlockedExchange8(&_isConnected, 0);
	////if (!isConnected)
	////	return;
	
	// true - Disconn, false - connecting.
	return InterlockedDecrement(&_ioCount) == 0;
}

C_Network::NetworkErrorCode C_Network::Session::ProcessSend(DWORD transferredBytes)
{
	_sendEvent._owner = nullptr;
	_sendEvent._pendingBuffs.clear();
	
	InterlockedExchange8(&_sendFlag, 0);

	if (transferredBytes == 0)
		return C_Network::NetworkErrorCode::SEND_LEN_ZERO;

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
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;

	PostRecv();

	return false;
}

bool C_Network::Session::ProcessDisconnect()
{
	// TODO : �� �κ��� �񵿱�� ó���� �� �����ϵ��� �Ѵ�.
	_disconnEvent._owner = nullptr;

	return false;
}

//class TestLog
//{
//	static void SetData(LONGLONG data)
//	{
//		static bool b = false;
//		
//		if (!b)
//		{
//			b = true;
//			InitializeSRWLock(&_logLock);
//		}
//
//		AcquireSRWLockExclusive(&_logLock);
//		dataSet.insert(data);
//	}
//public:
//	static std::unordered_set<LONGLONG> dataSet;
//	static SRWLOCK _logLock;
//};
// -----------------------------------------------------------------------------------------------------------------------
// Post
void C_Network::Session::PostSend()
{
	//if (!_isConnected)
	//	return;

	if (InterlockedExchange8(&_sendFlag, 1) == 1)
		return;

	_sendEvent.Reset();
	_sendEvent._owner = this;

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
			case 10053:break;

				// ����ڰ� �Ϲ������� ������ ���� ���� ���� ����� ���� �ʵ��� �ϰڴ�.  WSAECONNRESET
			case 10054:break;
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
	_recvEvent._owner = this;

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
	int recvRet = WSARecv(_socket, buf, wsabufSize, nullptr, &flag, reinterpret_cast<LPWSAOVERLAPPED>(& _recvEvent), nullptr);

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
			case 10053:break;
				// ����ڰ� �Ϲ������� ������ ���� ���� ���� ����� ���� �ʵ��� �ϰڴ�. WSAECONNRESET
			case 10054:break;
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
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::Session::PostConnect()
{
	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}

void C_Network::Session::PostDisconnect()
{	TODO_DEFINITION;
	TODO_UPDATE_EX_LIST;
}
// -----------------------------------------------------------------------------------------------------------------------



/*--------------------------------------
			Session Manager
--------------------------------------*/

C_Network::SessionManager::SessionManager(uint maxSessionCnt) : C_Utility::ManagerPool<Session>(maxSessionCnt)
{
	InitializeSRWLock(&_mapLock);
}

C_Network::SessionManager::~SessionManager()
{}

C_Network::Session* C_Network::SessionManager::AddSession(SOCKET sock, SOCKADDR_IN* pSockAddr)
{   
	// accept�� ��� �� á�� ���� ���� �ʱ� ������. �� á�� ��츦 ������ �ʿ�� ����.
	uint idx = GetAvailableIndex();
	
	C_Network::Session* newSession = _elementArr[idx];

	newSession->Init(sock, pSockAddr);

	ULONGLONG sessionId = newSession->GetId();

	{
		SRWLockGuard lockGuard(&_mapLock);
		_idToIndexDic[sessionId] = idx;
		_indexToIdDic[idx] = sessionId;
	}
	InterlockedIncrement(&_curElementCnt);

	return _elementArr[idx];
}

void C_Network::SessionManager::DeleteSession(Session* sessionPtr)
{
	ULONGLONG sessionId = sessionPtr->GetId();

	int arrIdx;
	{
		SRWLockGuard lockGuard(&_mapLock);
		arrIdx = _idToIndexDic[sessionId];
		_idToIndexDic.erase(sessionId);
		_indexToIdDic.erase(arrIdx);
	}
	closesocket(_elementArr[arrIdx]->GetSock());
	
	{
		SRWLockGuard lockGuard(&_indexListLock);
		_availableElementidxList.push(arrIdx);
	}
	InterlockedDecrement(&_curElementCnt);
}

C_Network::Session* C_Network::SessionManager::GetSession(ULONGLONG sessionId)
{
	SRWLockGuard lockGuard(&_mapLock);

	// ���� �ʿ�.
	auto sessionIter = _idToIndexDic.find(sessionId);
	if (sessionIter == _idToIndexDic.end())
		return nullptr;

	return _elementArr[sessionIter->second];//_sessionMap[sessionId];
}


// Client������ �翬������ �ʱ� ������ 
void C_Network::ClientSessionManager::DeleteAllSession()
{
	for (C_Network::Session* session : _elementArr)
	{
		DeleteSession(session);
	}
}
