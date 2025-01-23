#include "LibsPch.h"
#include "NetworkBase.h"
#include "GlobalQueue.h"
#include "JobQueue.h"
	/*-----------------------
		  ServerBase
	-----------------------*/

C_Network::ServerBase::ServerBase(const NetAddress& netAddr, uint maxSessionCnt) : _netAddr(netAddr), _iocpHandle(nullptr),_listenSock(INVALID_SOCKET)
{
	_sessionMgr = std::make_unique<ServerSessionManager>(maxSessionCnt);
	_logger = std::make_unique<C_Utility::FileLogger>();
	
	//_monitor = nullptr;// std::make_unique<C_Utility::NetMonitor>(_sessionMgr);

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

	_workerThreads.reserve(concurrentThreadCnt * 2);

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

	LINGER linger{};
	//linger.l_linger = 0;
	//linger.l_onoff = 0;
	DWORD setLingerRet = setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	if (setLingerRet == SOCKET_ERROR)
	{
		printf("Set Linger Failed GetLastError : %d", GetLastError());
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::SET_SOCK_OPT_FAILED;
	}
	
	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_netAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		printf("GetLastError : %d", GetLastError());
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::BIND_FAILED;

	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		TODO_LOG_ERROR;
		return C_Network::NetworkErrorCode::LISTEN_FAILED;
	}

	if(nullptr != _monitor)
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

	if(nullptr != _monitor)
	_monitor->End();

	_logger->EndLogging();

	if (_fileLogThread.joinable())
		_fileLogThread.join();
	
	TODO_LOG_SUCCESS;

	return C_Network::NetworkErrorCode::NONE;
}


void C_Network::ServerBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	SharedSession session = iocpEvent->_owner;

	// ��ȣ ���� ( �� ���� ��ȯ ������ �߻��� �� �־� NetworkBase���� Session�� ������ ���·� ����Ѵ�. )
	switch (iocpEvent->_type)
	{
	case IocpEventType::Accept:
	{
		ProcessAccept(session);
		break;
	}

	case IocpEventType::Connect:
	{
		ProcessConnect(session);
		//OnConnected(sessionPtr->GetNetAddr().GetSockAddr(), sessionPtr->GetId());
		break;
	}

	case IocpEventType::Recv:
	{
		ProcessRecv(session, transferredBytes);

		break;
	}

	case IocpEventType::Send:
	{
		ProcessSend(session, transferredBytes);

		break;
	}
	default:break;
	}
	if (session->CheckDisconnect())
		ProcessDisconnect(session, transferredBytes);
}


bool C_Network::ServerBase::ProcessIO(DWORD timeOut)
{
	IocpEvent* iocpEvent;
	DWORD transferredBytes;
	ULONG_PTR key;
	// Session*�� Key�� ����ؼ� ������ ���µ� ����� �� ������.
	// ���� �۾� (IocpEvent)�� �ڽ��� ������ (Session*)�� ��ϵǾ� �ֱ� ������ Key�� ������� �ʴ´�.

	iocpEvent = nullptr;
	transferredBytes = 0;

	key = 0;

	bool gqcsRet = GetQueuedCompletionStatus(_iocpHandle, &transferredBytes,
		&key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOut);

	// if ret == false => pOverlapped = nullptr;
	if (!iocpEvent)
	{
		DWORD wsaErr = WSAGetLastError();

		if (wsaErr == WSA_WAIT_TIMEOUT)
			return true;

		TODO_LOG_ERROR;
		PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
		return false;
	}

	Dispatch(iocpEvent, transferredBytes);
	
	return true;
}

void C_Network::ServerBase::ProcessJob()
{
	while (1)
	{
		uint64 tick = GetTickCount64();

		if (tick > LExecuteTimeTick)
			break;

		SharedJobQueue jobQueue = GGlobalQueue->Pop();

		if (nullptr == jobQueue)
			break;

		jobQueue->Execute();
	}
}
void C_Network::ServerBase::WorkerThread()
{
	while (1)
	{
		LExecuteTimeTick = GetTickCount64() + JOB_PROC_TICK;

		if (false == ProcessIO())
			break;

		ProcessJob();
	}
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
	SharedSession session = _sessionMgr->GetSession(sessionId);

	if (!session)
		return;
	
	session->Send(buffer);
}

void C_Network::ServerBase::ProcessAccept(SharedSession session, DWORD transferredBytes)
{
	TODO_UPDATE_EX_LIST; // �񵿱� ��ȯ�� �� �ڵ� ���� �ʿ�.

	session->ProcessAccept();
}

void C_Network::ServerBase::ProcessConnect(SharedSession session, DWORD transferredBytes)
{
	TODO_UPDATE_EX_LIST;// �񵿱� ��ȯ�� �� �ڵ� ���� �ʿ�.

	session->ProcessConnect();
}

C_Network::NetworkErrorCode C_Network::ServerBase::ProcessRecv(SharedSession session, DWORD transferredBytes)
{
	session->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		printf("���� ����\n");
		return C_Network::NetworkErrorCode::NONE; // ���� ����� �Ǵ�.
	}
	
	if (!session->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		//sessionPtr->CheckDisconnect();
		TODO_LOG_ERROR;
		printf("Buffer Size Overflow\n");
		return C_Network::NetworkErrorCode::RECV_BUF_OVERFLOW;
	}

	uint dataSize = session->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = session->_recvBuffer.GetUseSize();

		// packetheader���� ���� ����
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		session->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;
		
		session->_recvBuffer.MoveFront(sizeof(header));

		if (!session->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			TODO_LOG_ERROR; C_Utility::Log(L"OnRecv Dequeue Error\n");

			return C_Network::NetworkErrorCode::RECV_BUF_DEQUE_FAILED;
		}
		
		tempBuffer.MoveRearPos(header.size);
		
		OnRecv(tempBuffer, session->GetId(), header.type);

		//_monitor->IncRecvCount();
	}

	session->PostRecv();

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ServerBase::ProcessSend(SharedSession session, DWORD transferredBytes)
{
	C_Network::NetworkErrorCode ret = session->ProcessSend(transferredBytes);
	
	if (C_Network::NetworkErrorCode::SEND_LEN_ZERO == ret)
		ProcessDisconnect(session);
		//CCrash(L"Send 0 Byte");

	//_monitor->IncSendCount();

	return ret;
	//return sessionPtr->ProcessSend(transferredBytes);
}

bool C_Network::ServerBase::ProcessDisconnect(SharedSession session, DWORD transferredBytes)
{
	session->ProcessDisconnect();

	OnDisconnected(session->GetId());

	_sessionMgr->DeleteSession(session);

	C_Utility::Log(L"Process Disconn"); // TODO : �� �ڼ��ϰ�

	return true;
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
		if (!OnConnectionRequest(clientInfo)) // Client�� ������ ������ �� ���� ������ ���� ��
		{
			TODO_DEFINITION // ������ ������ �ش� ip�� �������� �α� ���
		}

		SharedSession newSession = _sessionMgr->CreateSession(clientSock, &clientInfo, _iocpHandle);
				
		if (nullptr == newSession)
			continue;

		newSession->ProcessAccept();

		OnConnected(clientInfo, newSession->GetId());
	}
}

