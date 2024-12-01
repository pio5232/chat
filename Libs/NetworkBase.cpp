#include "LibsPch.h"
#include "NetworkBase.h"


	/*-----------------------
		  ServerBase
	-----------------------*/

C_Network::ServerBase::ServerBase(const NetAddress& netAddr, uint maxSessionCnt) : _netAddr(netAddr), _iocpHandle(nullptr),_listenSock(INVALID_SOCKET)
{
	_sessionMgr = std::make_unique<ServerSessionManager>(maxSessionCnt);
	_logger = std::make_unique<C_Utility::FileLogger>();
	_monitor = std::make_unique<C_Utility::NetMonitor>(_sessionMgr.get());

	_fileLogThread = std::thread([&]() {_logger->Save(); })
	// Print Success and ConcurrentCnt
	TODO_LOG_SUCCESS;
}

C_Network::ServerBase::~ServerBase() 
{
	WSACleanup();
}

C_Network::NetworkErrorCode C_Network::ServerBase::Begin()
{
	WSAData wsa;

	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::WSA_START_UP_ERROR;
	}

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);

	uint concurrentThreadCnt = sys.dwNumberOfProcessors;

	concurrentThreadCnt = concurrentThreadCnt * 0.7;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;


	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		TODO_LOG_ERROR;
		return 	C_Network::NetworkErrorCode::CREATE_COMPLETION_PORT_FAILED;
	}

	_workerThreads.reserve(10); //(concurrentThreadCnt * 2);

	for (int i = 0; i < _workerThreads.capacity(); i++)
	{
		_workerThreads.push_back(std::thread([this]() {this->WorkerThread(); }));
	}

	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::CREATE_SOCKET_FAILED;
	}

	LINGER linger;
	linger.l_linger = 0;
	linger.l_onoff = 0;
	setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::BIND_FAILED;

	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::LISTEN_FAILED;
	}

	_monitor->Begin();
	// TODO_
	_acceptThread = std::thread([this]() { this->AcceptThread(); });

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ServerBase::End()
{
	closesocket(_listenSock);
	_listenSock = INVALID_SOCKET;

	if (_acceptThread.joinable())
		_acceptThread.join();

	CloseHandle(_iocpHandle);

	for (auto& t : _workerThreads)
	{
		if (t.joinable())
			t.join();
	}

	_monitor->End();

	_logger->EndLogging();

	if (_fileLogThread.joinable())
		_fileLogThread.join();
	
	TODO_LOG_SUCCESS;

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::Session* C_Network::ServerBase::CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr)
{
	TODO_UPDATE_EX_LIST;
	
	Session* newSession = _sessionMgr->AddSession(sock, pSockAddr);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), _iocpHandle, reinterpret_cast<ULONG_PTR>(newSession), 0);

	return newSession;
}


void C_Network::ServerBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	Session* session = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
	switch (iocpEvent->_type)
	{
	case IocpEventType::Accept:
		ProcessAccept(session);
		break;

	case IocpEventType::Connect:
		ProcessConnect(session);
		//OnConnected(sessionPtr->GetNetAddr().GetSockAddr(), sessionPtr->GetId());
		break;

	case IocpEventType::Recv:
		
		ProcessRecv(session, transferredBytes);

		if (session->CanDisconnect()) // Disconn
		{
			OnDisconnected(session->GetId());

			_sessionMgr->DeleteSession(session);
		}
		break;

	case IocpEventType::Send:
		
		ProcessSend(session, transferredBytes);
		
		if (session->CanDisconnect()) // Disconn
		{
			OnDisconnected(session->GetId());

			_sessionMgr->DeleteSession(session);
		}
		
		break;

	case IocpEventType::Disconnect: 
		ProcessDisconnect(session, transferredBytes);
		//OnDisconnected(sessionPtr->GetId());
		break;
	default:break;
	}
}

void C_Network::ServerBase::WorkerThread()
{
	IocpEvent *iocpEvent;
	DWORD transferredBytes;

	Session* pSession;

	while (1)
	{
		iocpEvent = nullptr;
		transferredBytes = 0;
		pSession = nullptr;

		bool gqcsRet = GetQueuedCompletionStatus(_iocpHandle, &transferredBytes, 
			reinterpret_cast<PULONG_PTR>(&pSession), reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE);

		// if ret == false => pOverlapped = nullptr;
		if(!iocpEvent)
		{
			TODO_LOG_ERROR;
			PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
			break;
		}

		Dispatch(iocpEvent, transferredBytes);
	}
	printf("Worker End\n");
}

bool C_Network::ServerBase::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	TODO_UPDATE_EX_LIST;
	return true;
}

void C_Network::ServerBase::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::ServerBase::OnDisconnected(ULONGLONG sessionId)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::ServerBase::OnError(int errCode, WCHAR* cause)
{
	TODO_UPDATE_EX_LIST;
}

void C_Network::ServerBase::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
}

void C_Network::ServerBase::Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer)
{
	Session* session = _sessionMgr->GetSession(sessionId);

	if (!session)
		return;
	
	session->Send(buffer);
}

void C_Network::ServerBase::ProcessAccept(Session* sessionPtr, DWORD transferredBytes)
{
	// TODO : Session* newSession = CreateSession(clientSock, &clientInfo); 

	sessionPtr->ProcessAccept();
}

void C_Network::ServerBase::ProcessConnect(Session* sessionPtr, DWORD transferredBytes)
{
	// TODO : Session* newSession = CreateSession(clientSock, &clientInfo);

	sessionPtr->ProcessConnect();
}

C_Network::NetworkErrorCode C_Network::ServerBase::ProcessRecv(Session* sessionPtr, DWORD transferredBytes)
{
	sessionPtr->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		return C_Network::NetworkErrorCode::NONE; // 정상 종료로 판단.
	}
	
	if (!sessionPtr->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		//sessionPtr->CheckDisconnect();
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::RECV_BUF_OVERFLOW;
	}

	uint dataSize = sessionPtr->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = sessionPtr->_recvBuffer.GetUseSize();

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		sessionPtr->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;
		
		sessionPtr->_recvBuffer.MoveFront(sizeof(header));

		if (!sessionPtr->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			TODO_LOG_ERROR; printf("OnRecv Dequeue Error\n");
			TODO_UPDATE_EX_LIST; 
			return C_Network::NetworkErrorCode::RECV_BUF_DEQUE_FAILED;
		}
		
		tempBuffer.MoveRearPos(header.size);
		
		OnRecv(tempBuffer, sessionPtr->GetId(), header.type);

		_monitor->IncRecvCount();
	}

	sessionPtr->PostRecv();

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ServerBase::ProcessSend(Session* sessionPtr, DWORD transferredBytes)
{
	C_Network::NetworkErrorCode ret = sessionPtr->ProcessSend(transferredBytes);
	
	if (C_Network::NetworkErrorCode::SEND_LEN_ZERO == ret)
		CCrash(L"Send 0 Byte");

	_monitor->IncSendCount();

	return ret;
	//return sessionPtr->ProcessSend(transferredBytes);
}

bool C_Network::ServerBase::ProcessDisconnect(Session* sessionPtr, DWORD transferredBytes)
{
	bool bRet = sessionPtr->ProcessDisconnect();
	
	if (bRet)
		// or !bRet에 대한 로직 생성
		TODO_UPDATE_EX_LIST;

	//DeleteSession(sessionPtr);

	return bRet;
}

void C_Network::ServerBase::AcceptThread()
{
	while (1)
	{
		SOCKADDR_IN clientInfo;
		int infoSize = sizeof(clientInfo);
		SOCKET clientSock = accept(_listenSock, (SOCKADDR*)&clientInfo, &infoSize);

		if (clientSock == INVALID_SOCKET)
		{
			TODO_LOG; // EXIT LOG
			printf("Accept thread return\n");
			break;
  		}
		if (!OnConnectionRequest(clientInfo)) // Client가 서버에 접속할 수 없는 이유가 있을 때
		{
			TODO_DEFINITION // 거절한 이유와 해당 ip는 무엇인지 로그 기록
		}
		if (!CanConnect()) // Server의 상황이 Connect할만하지 않으면. 또는 어떠한 이유로 Server가 연결할 수 없으면
		{
			continue;
		}

		Session* newSession = CreateSession(clientSock, &clientInfo);
				
		newSession->ProcessAccept();

		OnConnected(clientInfo, newSession->GetId());
	}
}


	/*-----------------------
			ClientBase
	-----------------------*/
C_Network::ClientBase::ClientBase(const NetAddress& targetEndPoint, uint maxSessionCnt) : _iocpHandle(INVALID_HANDLE_VALUE), _targetEndPoint(targetEndPoint)
{
	_sessionMgr = std::make_unique<ClientSessionManager>(maxSessionCnt);
	_monitor = std::make_unique<C_Utility::NetMonitor>(_sessionMgr.get());
	_userMgr = std::make_unique<UserManager>(maxSessionCnt);
}

C_Network::ClientBase::~ClientBase()
{
}

C_Network::NetworkErrorCode C_Network::ClientBase::Init()
{
	WSAData wsa;

	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::WSA_START_UP_ERROR;
	}

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);

	uint concurrentThreadCnt = sys.dwNumberOfProcessors;

	concurrentThreadCnt = concurrentThreadCnt * 0.6;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;

	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		TODO_LOG_ERROR;
		return 	C_Network::NetworkErrorCode::CREATE_COMPLETION_PORT_FAILED;
	}

	if (_sessionMgr->GetMaxElemenetCount() == 1)
		_workerThreads.reserve(1); // Only 1 Client
	else
		_workerThreads.reserve(concurrentThreadCnt * 2); // DummyClient

	for (int i = 0; i < _workerThreads.capacity(); i++)
	{
		_workerThreads.push_back(std::thread([this]() {this->WorkerThread(); }));
	}
	
	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ClientBase::Begin()
{
	_monitor->Begin();
	 
	C_Network::NetworkErrorCode ret = Connect();
	
	return ret;
}

C_Network::NetworkErrorCode C_Network::ClientBase::End()
{
	for (std::thread& t: _workerThreads)
	{
		if (t.joinable())
			t.join();
	}
	_monitor->End();

	WSACleanup();

	return C_Network::NetworkErrorCode::NONE;
}

void C_Network::ClientBase::Send(C_Network::SharedSendBuffer buffer, ULONGLONG sessionId)
{
	if (sessionId == 0)
		_sessionMgr->GetMySessionId(sessionId);

	Session* session = _sessionMgr->GetSession(sessionId);

	if (!session)
		return;

	session->Send(buffer);
}

C_Network::NetworkErrorCode C_Network::ClientBase::Connect()
{
	for (int i = 0; i < _sessionMgr->GetMaxElemenetCount(); i++)
	{
		SOCKET clientSock = socket(AF_INET, SOCK_STREAM, 0);

		LINGER linger = {};

		if (SOCKET_ERROR == setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger)))
		{
			TODO_LOG_ERROR;
			return C_Network::NetworkErrorCode::SET_SOCK_OPT_FAILED;
		}

		if (SOCKET_ERROR == connect(clientSock, (SOCKADDR*)&_targetEndPoint.GetSockAddr(), sizeof(SOCKADDR_IN)))
		{
			TODO_LOG_ERROR;
			return C_Network::NetworkErrorCode::CLIENT_CONNECT_FAILED;
		}

		SOCKADDR_IN clientInfo;
		int sockaddrLen = sizeof(clientInfo);
		if (SOCKET_ERROR == getsockname(clientSock, (SOCKADDR*)&clientInfo, &sockaddrLen))
		{
			TODO_LOG_ERROR;
			return C_Network::NetworkErrorCode::SET_SOCK_OPT_FAILED;
		}

		Session* newSession = _sessionMgr->AddSession(clientSock, &clientInfo);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSock), _iocpHandle, reinterpret_cast<ULONG_PTR>(newSession), 0);

		newSession->PostRecv();

		OnEnterServer();
	}

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ClientBase::Disconnect()
{
	_sessionMgr->DeleteAllSession();

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ClientBase::OnEnterServer()
{
	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ClientBase::OnLeave()
{
	// LEAVE SERVER LOG

	return C_Network::NetworkErrorCode::NONE;
}

void C_Network::ClientBase::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
}

C_Network::NetworkErrorCode C_Network::ClientBase::ProcessRecv(Session* sessionPtr, DWORD transferredBytes)
{
	sessionPtr->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		return C_Network::NetworkErrorCode::NONE; // 정상 종료로 판단.
	}

	if (!sessionPtr->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		//sessionPtr->CheckDisconnect();
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::RECV_BUF_OVERFLOW;
	}

	uint dataSize = sessionPtr->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = sessionPtr->_recvBuffer.GetUseSize();

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		sessionPtr->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;

		sessionPtr->_recvBuffer.MoveFront(sizeof(header));

		if (!sessionPtr->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			TODO_LOG_ERROR; printf("OnRecv Dequeue Error\n");
			TODO_UPDATE_EX_LIST;
			return C_Network::NetworkErrorCode::RECV_BUF_DEQUE_FAILED;
		}

		tempBuffer.MoveRearPos(header.size);

		OnRecv(tempBuffer,sessionPtr->GetId(), header.type); 

		// Monitor Log
		_monitor->IncRecvCount();
	}

	sessionPtr->PostRecv();

	return C_Network::NetworkErrorCode();
}

C_Network::NetworkErrorCode C_Network::ClientBase::ProcessSend(Session* sessionPtr, DWORD transferredBytes)
{
	C_Network::NetworkErrorCode ret = sessionPtr->ProcessSend(transferredBytes);

	if (C_Network::NetworkErrorCode::SEND_LEN_ZERO == ret)
		CCrash(L"Send 0 Byte");

	// Monitor Log
	_monitor->IncSendCount();

	return ret;
}

void C_Network::ClientBase::Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes)
{
	Session* session = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
	switch (iocpEvent->_type)
	{
	case IocpEventType::Recv:

		ProcessRecv(session, transferredBytes);
		break;

	case IocpEventType::Send:

		ProcessSend(session, transferredBytes);
		break;

	default: TODO_LOG;  break;
	}
}

void C_Network::ClientBase::WorkerThread()
{
	IocpEvent* iocpEvent;
	DWORD transferredBytes;

	Session* pSession;

	while (1)
	{
		iocpEvent = nullptr;
		transferredBytes = 0;
		pSession = nullptr;

		bool gqcsRet = GetQueuedCompletionStatus(_iocpHandle, &transferredBytes,
			reinterpret_cast<PULONG_PTR>(&pSession), reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), INFINITE);

		if (!iocpEvent)
		{
			TODO_LOG_ERROR;
			PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
			break;
		}

		Dispatch(iocpEvent, transferredBytes);
	}
	printf("Clietn Worker End\n");

}
