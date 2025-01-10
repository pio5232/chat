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
	enum
	{
		JOB_PROC_TICK = 64,
	};
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

	private:
		bool ProcessIO(DWORD timeOut = INFINITE);
		void ProcessJob(); // 할 일이 없는 경우 글로벌 Job 처리

		void ProcessAccept(SharedSession session, DWORD transferredBytes = 0);
		void ProcessConnect(SharedSession session, DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessRecv(SharedSession session, DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessSend(SharedSession session, DWORD transferredBytes = 0);
		bool ProcessDisconnect(SharedSession session, DWORD transferredBytes = 0);

	private:
		
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();

		void AcceptThread();

		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;
		std::thread _fileLogThread;

	protected:
		const NetAddress _netAddr; 
		std::unique_ptr<C_Utility::CMonitor> _monitor = nullptr;
		std::unique_ptr<ServerSessionManager> _sessionMgr;
	private:
		std::unique_ptr<C_Utility::FileLogger> _logger;

		SOCKET _listenSock;
		std::thread _acceptThread;
		// NetServer -> listen EndPoint
		// NetClient -> dest EndPoint
	};

}