#include "LibsPch.h"
#include "NetworkBase.h"
#include "GlobalQueue.h"
#include "JobQueue.h"
#include "ws2tcpip.h"

#pragma comment (lib, "ws2_32.lib")
	/*-----------------------
		  ServerBase
	-----------------------*/

C_Network::ServerBase::ServerBase(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator createFunc) : _targetNetAddr(netAddr), _iocpHandle(nullptr),_listenSock(INVALID_SOCKET)
{
	_sessionMgr = std::make_unique<ServerSessionManager>(maxSessionCnt, createFunc);
	_logger = std::make_unique<C_Utility::FileLogger>();
	
	//_monitor = std::make_unique<C_Utility::NetMonitor>(_sessionMgr.get());

	_fileLogThread = std::thread([&]() {_logger->Save(); });
}

C_Network::ServerBase::~ServerBase() 
{
	WSACleanup();
}

ErrorCode C_Network::ServerBase::Begin(bool isWorkerAlone)
{
	WSAData wsa;

	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		return ErrorCode::WSA_START_UP_ERROR;
	}

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);

	uint concurrentThreadCnt;
	
	if (!isWorkerAlone)
		concurrentThreadCnt = sys.dwNumberOfProcessors;
	else
		concurrentThreadCnt = 1;

	concurrentThreadCnt = concurrentThreadCnt * 0.7;
	if (concurrentThreadCnt == 0)
		concurrentThreadCnt = 1;


	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		return 	ErrorCode::CREATE_COMPLETION_PORT_FAILED;
	}

	_workerThreads.reserve(concurrentThreadCnt * 2);

	for (int i = 0; i < _workerThreads.capacity(); i++)
	{
		_workerThreads.push_back(std::thread([this]() {this->WorkerThread(); }));
	}

	_listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock == INVALID_SOCKET)
	{
		return ErrorCode::CREATE_SOCKET_FAILED;
	}

	LINGER linger{};
	//linger.l_linger = 0;
	//linger.l_onoff = 0;
	DWORD setLingerRet = setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	if (setLingerRet == SOCKET_ERROR)
	{
		printf("Set Linger Failed GetLastError : %d", GetLastError());
		
		return ErrorCode::SET_SOCK_OPT_FAILED;
	}
	
	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_targetNetAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		printf("GetLastError : %d", GetLastError());

		return ErrorCode::BIND_FAILED;

	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		return ErrorCode::LISTEN_FAILED;
	}

	if(nullptr != _monitor)
	_monitor->Begin();
	// TODO_
	_acceptThread = std::thread([this]() { this->AcceptThread(); });

	return ErrorCode::NONE;
}

ErrorCode C_Network::ServerBase::End()
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
	
	return ErrorCode::NONE;
}


void C_Network::ServerBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	SessionPtr session = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
	switch (iocpEvent->_type)
	{
	case IocpEventType::Recv:
	{
		ErrorCode errorCode = session->ProcessRecv(transferredBytes);
		
		if (errorCode != ErrorCode::NONE)
		{
			; // LOG
		}

		break;
	}

	case IocpEventType::Send:
	{
		ErrorCode errorCode = session->ProcessSend(transferredBytes);

		if (errorCode != ErrorCode::NONE)
		{
			; // LOG
		}
		break;
	}
	default:break;
	}
}


bool C_Network::ServerBase::ProcessIO(DWORD timeOut)
{
	IocpEvent* iocpEvent;
	DWORD transferredBytes;
	ULONG_PTR key;
	// Session*를 Key로 등록해서 가지는 형태도 사용할 수 있지만.
	// 현재 작업 (IocpEvent)에 자신의 소유자 (Session*)가 등록되어 있기 때문에 Key는 사용하지 않는다.

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

		JobQueuePtr jobQueue = GGlobalQueue->Pop();

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
	return true;
}


void C_Network::ServerBase::OnError(int errCode, WCHAR* cause)
{
}


void C_Network::ServerBase::Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer)
{
	SessionPtr session = _sessionMgr->GetSession(sessionId);

	if (session == nullptr)
		return;
	
	session->Send(buffer);
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
			printf("Accept thread return\n");
			break;
  		}
		if (!OnConnectionRequest(clientInfo)) // Client가 서버에 접속할 수 없는 이유가 있을 때
		{
		}


		SessionPtr newSession = _sessionMgr->CreateSession(clientSock, &clientInfo, _iocpHandle, shared_from_this());
				
		if (nullptr == newSession)
			continue;

		newSession->ProcessConnect();
	}
}
