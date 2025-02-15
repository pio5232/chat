#pragma once
#include <unordered_map>
#include "RoomManager.h"
#include "Session.h"
#include "PacketDefine.h"
#include <iostream>
#include "CMonitor.h"
#include "UserManager.h"
#include <functional>
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

		NetworkErrorCode Begin(bool isWorkerAlone = false);
		NetworkErrorCode End();

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId); 
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type);

		void Send(ULONGLONG sessionId, C_Network::SharedSendBuffer buffer);
		const NetAddress GetNetAddr() const { return _targetNetAddr; }
	protected:
		SOCKET GetListenSock() { return _listenSock; }
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

	class LanServer : public ServerBase
	{
	public:
		using Callback = std::function<void(C_Network::SharedSendBuffer, uint16, NetCallbackType)>;
		LanServer(const NetAddress& netAddr, uint maxSessionCnt);
		~LanServer();

		void RegistCallback(Callback&& callback) {_callback = std::move(callback);}
		void ExecuteCallback(C_Network::SharedSendBuffer buffer, uint16 roomNum, NetCallbackType callbackType)
		{
			if (_callback)
				_callback(buffer, roomNum, callbackType);
		}
		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo);
		virtual void OnConnected(const SOCKADDR_IN& clientInfo, ULONGLONG sessionId);
		virtual void OnDisconnected(ULONGLONG sessionId);
		virtual void OnError(int errCode, WCHAR* cause);
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, ULONGLONG sessionId, uint16 type);
	private:
		std::unique_ptr<class LanClientPacketHandler> _packetHandler = nullptr;

		Callback _callback;
	};

	class ClientBase
	{
	public:
		ClientBase(const NetAddress& targetNetAddr);
		virtual ~ClientBase() = 0;

		bool Connect();
		bool Disconnect();
		
		virtual void OnEnterJoinServer();
		virtual void OnLeaveServer();
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type);

		void Send(C_Network::SharedSendBuffer buffer);

		bool ProcessIO(DWORD timeOut = INFINITE);

		C_Network::NetworkErrorCode ProcessRecv(DWORD transferredBytes = 0);
		C_Network::NetworkErrorCode ProcessSend(DWORD transferredBytes = 0);
		
	private:
		void Dispatch(IocpEvent* iocpEvent, DWORD transferredBytes);
		void WorkerThread();

	private:
		SharedSession _clientSession; // Client의 session은 ClientBase 인스턴스가 사라질 때 사라짐.

		const NetAddress _targetNetAddr;

		HANDLE _iocpHandle;
		std::thread _workerThread;
	};

	//class LanClient : public ClientBase
	//{
	//public:
	//	LanClient(const NetAddress& targetNetAddr, const NetAddress& ownerNetAddr, uint16 myRoomNum);
	//	~LanClient();
	//	virtual void OnEnterJoinServer() override;
	//	virtual void OnLeaveServer() override;
	//	virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) override;
	//
	//private:

	//	C_Network::NetAddress _ownerNetAddr;
	//	const uint16 _myRoomNumber;
	//};
}