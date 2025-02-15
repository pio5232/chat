#include "LibsPch.h"
#include "NetworkBase.h"
#include "GlobalQueue.h"
#include "JobQueue.h"
#include "ws2tcpip.h"
#include "ClientPacketHandler.h"

#pragma comment (lib, "ws2_32.lib")
	/*-----------------------
		  ServerBase
	-----------------------*/

C_Network::ServerBase::ServerBase(const NetAddress& netAddr, uint maxSessionCnt) : _targetNetAddr(netAddr), _iocpHandle(nullptr),_listenSock(INVALID_SOCKET)
{
	_sessionMgr = std::make_unique<ServerSessionManager>(maxSessionCnt);
	_logger = std::make_unique<C_Utility::FileLogger>();
	
	//_monitor = nullptr;// std::make_unique<C_Utility::NetMonitor>(_sessionMgr);

	_fileLogThread = std::thread([&]() {_logger->Save(); });
}

C_Network::ServerBase::~ServerBase() 
{
	WSACleanup();
}

C_Network::NetworkErrorCode C_Network::ServerBase::Begin(bool isWorkerAlone)
{
	WSAData wsa;

	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		return C_Network::NetworkErrorCode::WSA_START_UP_ERROR;
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
		return C_Network::NetworkErrorCode::CREATE_SOCKET_FAILED;
	}

	LINGER linger{};
	//linger.l_linger = 0;
	//linger.l_onoff = 0;
	DWORD setLingerRet = setsockopt(_listenSock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	if (setLingerRet == SOCKET_ERROR)
	{
		printf("Set Linger Failed GetLastError : %d", GetLastError());
		
		return C_Network::NetworkErrorCode::SET_SOCK_OPT_FAILED;
	}
	
	DWORD bindRet = bind(_listenSock, (SOCKADDR*)&_targetNetAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (bindRet == SOCKET_ERROR)
	{
		printf("GetLastError : %d", GetLastError());

		return C_Network::NetworkErrorCode::BIND_FAILED;

	}

	DWORD listenRet = ::listen(_listenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
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
	
	return C_Network::NetworkErrorCode::NONE;
}


void C_Network::ServerBase::Dispatch(C_Network::IocpEvent* iocpEvent, DWORD transferredBytes)
{
	SharedSession session = iocpEvent->_owner;

	// 상호 참조 ( 이 경우는 순환 참조가 발생할 수 있어 NetworkBase에서 Session을 가지는 형태로 사용한다. )
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
	return true;
}

void C_Network::ServerBase::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
}

void C_Network::ServerBase::OnDisconnected(ULONGLONG sessionId)
{
}

void C_Network::ServerBase::OnError(int errCode, WCHAR* cause)
{
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
	session->ProcessAccept();
}

void C_Network::ServerBase::ProcessConnect(SharedSession session, DWORD transferredBytes)
{
	session->ProcessConnect();
}

C_Network::NetworkErrorCode C_Network::ServerBase::ProcessRecv(SharedSession session, DWORD transferredBytes)
{
	session->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		printf("정상 종료\n");
		return C_Network::NetworkErrorCode::NONE; // 정상 종료로 판단.
	}
	
	if (!session->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
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

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		session->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;
		
		session->_recvBuffer.MoveFront(sizeof(header));

		if (!session->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			wprintf(L"OnRecv Dequeue Error\n");

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

	wprintf(L"Process Disconn");

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
			printf("Accept thread return\n");
			break;
  		}
		if (!OnConnectionRequest(clientInfo)) // Client가 서버에 접속할 수 없는 이유가 있을 때
		{
		}

		SharedSession newSession = _sessionMgr->CreateSession(clientSock, &clientInfo, _iocpHandle);
				
		if (nullptr == newSession)
			continue;

		newSession->ProcessAccept();

		OnConnected(clientInfo, newSession->GetId());

	}
}

C_Network::LanServer::LanServer(const NetAddress& netAddr, uint maxSessionCnt) : ServerBase(netAddr, maxSessionCnt), _callback(nullptr)
{
	// LAN 전용 Monitor 등록.
	_monitor = std::make_unique<C_Utility::NetMonitor>(_sessionMgr.get());
	
	_packetHandler = std::make_unique<LanClientPacketHandler>(_sessionMgr.get(), this);
}

C_Network::LanServer::~LanServer()
{
	_sessionMgr = nullptr;
}

bool C_Network::LanServer::OnConnectionRequest(const SOCKADDR_IN& clientInfo)
{
	return false;
}

void C_Network::LanServer::OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId)
{
	WCHAR ipWstr[IP_STRING_LEN];

	// 주소체계, &IN_ADDR
	InetNtopW(AF_INET, &clientInfo.sin_addr, ipWstr, sizeof(ipWstr) / sizeof(WCHAR));

	wprintf(L"LAN SERVER ONCONNECTED IP : %s   Port : %d\n", ipWstr, ntohs(clientInfo.sin_port));
}

void C_Network::LanServer::OnDisconnected(ULONGLONG sessionId)
{
	wprintf(L"Session Id [%lld] OnDisconnected\n ");
}

void C_Network::LanServer::OnError(int errCode, WCHAR* cause)
{
}

void C_Network::LanServer::OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type)
{
	printf("OnRecv - Session ID [%lld], packetType : %u\n", sessionId, type);
	if (_packetHandler->ProcessPacket(sessionId, type, buffer) == ErrorCode::CANNOT_FIND_PACKET_FUNC)
		printf("[ !!! OnRecv Failed.. !!! ] \n");
}

// ----------------- ClientBase ----------------------

C_Network::ClientBase::ClientBase(const NetAddress& targetNetAddr) : _iocpHandle(nullptr), _clientSession(nullptr),
_targetNetAddr(targetNetAddr)
{
	WSAData wsa;

	int iRet = WSAStartup(MAKEWORD(2, 2), &wsa);

	if (iRet)
	{
		printf("[WSAStartUp Error !!]\n");
		return;
	}

	SYSTEM_INFO sys;
	GetSystemInfo(&sys);

	uint concurrentThreadCnt = 1;

	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, concurrentThreadCnt);

	if (_iocpHandle == NULL)
	{
		printf("[Create IOCP Handle is Failed!!]\n");
		return;
	}


	_workerThread = std::thread([this]() {this->WorkerThread(); });

	SOCKET clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSock == INVALID_SOCKET)
	{
		printf("ClientSocket is INVALID\n");
		return;
	}

	_clientSession = std::make_shared<C_Network::Session>(clientSock, &_targetNetAddr.GetSockAddr());

	LINGER linger{};
	//linger.l_linger = 0;
	//linger.l_onoff = 0;
	DWORD setLingerRet = setsockopt(_clientSession->GetSock(), SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));
	if (setLingerRet == SOCKET_ERROR)
	{
		printf("Set Linger Failed GetLastError : %d", GetLastError());

		return;
	}
}

C_Network::ClientBase::~ClientBase() 
{
	CloseHandle(_iocpHandle);

	if (_workerThread.joinable())
		_workerThread.join();

	WSACleanup();
}

bool C_Network::ClientBase::Connect()
{
	if (_clientSession->GetSock() == INVALID_SOCKET)
	{
		printf("[Socket is Invalid]");
		return false;
	}

	int connectRet = connect(_clientSession->GetSock(), (sockaddr*)&_targetNetAddr.GetSockAddr(), sizeof(SOCKADDR_IN));

	if (connectRet)
	{
		printf("[Client Connect Error - %d", GetLastError());;
		return false;
	}

	HANDLE RegistIocpRet = CreateIoCompletionPort((HANDLE)_clientSession->GetSock(), _iocpHandle, NULL, NULL);

	if (nullptr == RegistIocpRet)
	{
		printf("[Regist Session Handle Failed]\n");
		return false;
	}
	_clientSession->PostRecv();

	OnEnterJoinServer();

	return true;
}

bool C_Network::ClientBase::Disconnect()
{
	OnLeaveServer();

	_clientSession->Disconnect();

	return false;
}

void C_Network::ClientBase::OnEnterJoinServer()
{
}

void C_Network::ClientBase::OnLeaveServer()
{
}

void C_Network::ClientBase::OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type)
{
}

void C_Network::ClientBase::Send(C_Network::SharedSendBuffer buffer)
{
	if (!_clientSession)
	{
		printf("[Send Error - ClientSession is null !!!!!]");
		return;
	}
	_clientSession->Send(buffer);
}

bool C_Network::ClientBase::ProcessIO(DWORD timeOut)
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

		printf("IocpEvent is NULL\n");
		if (wsaErr == WSA_WAIT_TIMEOUT)
			return true;

		PostQueuedCompletionStatus(_iocpHandle, 0, 0, nullptr);
		return false;
	}

	Dispatch(iocpEvent, transferredBytes);

	return true;
}

C_Network::NetworkErrorCode C_Network::ClientBase::ProcessRecv(DWORD transferredBytes)
{
	_clientSession->_recvEvent._owner = nullptr;

	if (transferredBytes == 0)
	{
		printf("정상 종료\n");
		return C_Network::NetworkErrorCode::NONE; // 정상 종료로 판단.
	}

	if (!_clientSession->_recvBuffer.MoveRearRetBool(transferredBytes))
	{
		printf("Buffer Size Overflow\n");
		return C_Network::NetworkErrorCode::RECV_BUF_OVERFLOW;
	}

	uint dataSize = _clientSession->_recvBuffer.GetUseSize();

	uint processingLen = 0;

	C_Utility::CSerializationBuffer tempBuffer(3000);

	while (1)
	{
		tempBuffer.Clear();

		int bufferSize = _clientSession->_recvBuffer.GetUseSize();

		// packetheader보다 작은 상태
		if (bufferSize < sizeof(PacketHeader))
			break;

		PacketHeader header;

		_clientSession->_recvBuffer.PeekRetBool(reinterpret_cast<char*>(&header), sizeof(PacketHeader));

		if (bufferSize < (sizeof(PacketHeader) + header.size))
			break;

		_clientSession->_recvBuffer.MoveFront(sizeof(header));

		if (!_clientSession->_recvBuffer.DequeueRetBool(tempBuffer.GetRearPtr(), header.size))
		{
			wprintf(L"OnRecv Dequeue Error\n");

			return C_Network::NetworkErrorCode::RECV_BUF_DEQUE_FAILED;
		}

		tempBuffer.MoveRearPos(header.size);

		OnRecv(tempBuffer, header.type);

		//_monitor->IncRecvCount();
	}

	_clientSession->PostRecv();

	return C_Network::NetworkErrorCode::NONE;
}

C_Network::NetworkErrorCode C_Network::ClientBase::ProcessSend(DWORD transferredBytes)
{
	C_Network::NetworkErrorCode ret = _clientSession->ProcessSend(transferredBytes);

	if (C_Network::NetworkErrorCode::SEND_LEN_ZERO == ret)
		Disconnect();


	return ret;
}

void C_Network::ClientBase::Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes)
{
	switch (iocpEvent->_type)
	{
	case IocpEventType::Recv:
	{
		ProcessRecv(transferredBytes);

		break;
	}

	case IocpEventType::Send:
	{
		ProcessSend(transferredBytes);

		break;
	}
	default:break;
	}
	if (_clientSession->CheckDisconnect())
		Disconnect();
}

void C_Network::ClientBase::WorkerThread()
{
	while (1)
	{
		if (false == ProcessIO())
			break;
	}

}

//C_Network::LanClient::LanClient(const NetAddress& targetNetAddr, const NetAddress& ownerNetAddr, uint16 myRoomNum)
//	: ClientBase(targetNetAddr), _ownerNetAddr(ownerNetAddr), _myRoomNumber(myRoomNum)
//{
//}
//
//C_Network::LanClient::~LanClient()
//{
//}
//
//void C_Network::LanClient::OnEnterJoinServer()
//{
//	C_Network::GameServerInfoNotifyPacket packet;
//
//	C_Network::SharedSendBuffer buffer = C_Network::ChattingClientPacketHandler::MakeSendBuffer((sizeof(packet)));
//
//	const std::wstring ip = _ownerNetAddr.GetIpAddress();
//	uint16 port = _ownerNetAddr.GetPort();
//
//	wcscpy_s(packet.ipStr, IP_STRING_LEN, ip.c_str());
//
//	*buffer << packet.size << packet.type;
//
//	buffer->PutData(reinterpret_cast<const char*>(packet.ipStr), IP_STRING_LEN * sizeof(WCHAR));
//	
//	*buffer << port << _myRoomNumber;
//
//	printf("ClientJoined Success\n");
//
//	wprintf(L"size : [ %d ]\n", packet.size);
//	wprintf(L"type : [ %d ]\n", packet.type);
//	wprintf(L"ip: [ %s ]\n", packet.ipStr);
//	wprintf(L"port : [ %d ]\n", port);
//	wprintf(L"Room : [ %d ]\n", _myRoomNumber);
//
//	Send(buffer);
//}
//
//void C_Network::LanClient::OnLeaveServer()
//{
//	printf("Client Leave\n");
//}
//
//void C_Network::LanClient::OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type)
//{
//	printf("\t\tClient OnRecv - packetType : %u\n", type);
//
//}
