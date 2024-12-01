#pragma once
#include <unordered_map>
#include "RoomManager.h"
#include "Session.h"
#include "PacketDefine.h"
#include <iostream>
#include "CMonitor.h"
#include "UserManager.h"
namespace C_Network
{
	/*-----------------------
			ServerBase
	-----------------------*/ 
	class ServerBase
	{
	public:
		ServerBase(const NetAddress& netAddr, uint maxSessionCnt);
		virtual ~ServerBase() = 0;

		NetworkErrorCode Begin();
		NetworkErrorCode End();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId); 
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type);

		void Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer);
		const NetAddress GetNetAddr() const { return _netAddr; }
		bool CanConnect() { return !_sessionMgr->IsFull(); } // 현재는 세션이 초과되었는지만 확인.

	private:
		void ProcessAccept(Session* sessionPtr, DWORD transferredBytes = 0);
		void ProcessConnect(Session* sessionPtr, DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessRecv(Session* sessionPtr, DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessSend(Session* sessionPtr, DWORD transferredBytes = 0);
		bool ProcessDisconnect(Session* sessionPtr, DWORD transferredBytes = 0);

	private:
		
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();

		void AcceptThread();

		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;
		std::thread _fileLogThread;

	protected:
		Session* CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr);
		const NetAddress _netAddr; 
	private:
		std::unique_ptr<ServerSessionManager> _sessionMgr;
		std::unique_ptr<C_Utility::FileLogger> _logger;
		std::unique_ptr<C_Utility::NetMonitor> _monitor;

		SOCKET _listenSock;
		std::thread _acceptThread;
		// NetServer -> listen EndPoint
		// NetClient -> dest EndPoint
	};

	/*-----------------------
			ClientBase
	-----------------------*/
	class ClientBase
	{
	public:
		// Dummy Or 1 Client
		// Dummy에서는 연결 재연결을 반복하는 형태로 만들지 않았음.
		ClientBase(const NetAddress& targetEndPoint, uint maxSessionCnt);
		virtual ~ClientBase() = 0;

		C_Network::NetworkErrorCode Init(); // 소켓 초기화
		C_Network::NetworkErrorCode Begin(); 
		C_Network::NetworkErrorCode End(); 

		// sessionId가 0이 될 일은 없다.
		void Send(C_Network::SharedSendBuffer buffer, ULONGLONG sessionId = 0);

		C_Network::NetworkErrorCode Connect(); // 대상에 연결
		C_Network::NetworkErrorCode Disconnect(); // 연결 끊기.

		virtual C_Network::NetworkErrorCode OnEnterServer();
		virtual C_Network::NetworkErrorCode OnLeave();
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type); // 서버에서 온 메시지를 처리.

	private:
		C_Network::NetworkErrorCode ProcessRecv(Session* sessionPtr, DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessSend(Session* sessionPtr, DWORD transferredBytes = 0);

		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();

		// Server와 마찬가지로 클라이언트도 세션 배열을 사용해 소켓을 돌려쓰도록 한다 -> 초기화 필요.
	private:
		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;

		std::unique_ptr<C_Utility::NetMonitor> _monitor;
		const NetAddress _targetEndPoint;
	protected:
		std::unique_ptr<ClientSessionManager> _sessionMgr;
		std::unique_ptr<UserManager> _userMgr;


	};
}