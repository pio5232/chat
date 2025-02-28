#pragma once
#include <unordered_map>
#include <iostream>
#include <functional>
#include <thread>
#include "Session.h"
#include "PacketDefine.h"
#include "CMonitor.h"
namespace C_Network
{
	enum
	{
		JOB_PROC_TICK = 64,
	};
	/*-----------------------
			ServerBase
	-----------------------*/ 

	class ServerBase : public std::enable_shared_from_this<ServerBase>
	{
	public:
		ServerBase(const NetAddress& netAddr, uint maxSessionCnt, C_Network::SessionCreator createFunc);
		virtual ~ServerBase() = 0;

		ErrorCode Begin(bool isWorkerAlone = false);
		ErrorCode End();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnError(int errCode, WCHAR* cause);

		void Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer);
		const NetAddress GetNetAddr() const { return _targetNetAddr; }

		void DeleteSession(SessionPtr session) { _sessionMgr->DeleteSession(session); }
	protected:
		SOCKET GetListenSock() { return _listenSock; }
	private:
		bool ProcessIO(DWORD timeOut = INFINITE);
		void ProcessJob(); // 할 일이 없는 경우 글로벌 Job 처리

	private:
		
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();

		void AcceptThread();

		HANDLE _iocpHandle;
		std::vector<std::thread> _workerThreads;
		std::thread _fileLogThread;

	protected:
		const NetAddress _targetNetAddr; 
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