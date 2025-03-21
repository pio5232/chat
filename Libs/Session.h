#pragma once

#include <queue>
#include <stack>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace C_Network
{
	using SessionCreator = std::function<SessionPtr()>;

	/*----------------------
			IocpEvent
	----------------------*/
	enum class IocpEventType : unsigned char
	{
		Recv,
		Send,
		EventMax,
		// PreRecv, 0 byte Recv
	};
	
	struct IocpEvent : private OVERLAPPED
	{
	public:
		IocpEvent(IocpEventType type);
		void Reset();
		 
		SessionPtr _owner;

		const IocpEventType _type;
	};


	struct RecvEvent : public IocpEvent
	{
	public:
		RecvEvent() : IocpEvent(IocpEventType::Recv) {}
	};

	struct SendEvent : public IocpEvent
	{
	public:
		SendEvent() : IocpEvent(IocpEventType::Send) { _pendingBuffs.reserve(10); }
		std::vector<SharedSendBuffer> _pendingBuffs;
	};


	/*------------------------------
				Session 
	------------------------------*/

	struct Session : public std::enable_shared_from_this<Session>
	{
	public:
		Session();
		~Session();

		void Init(SOCKET sock, const SOCKADDR_IN* pSockAddr, std::weak_ptr<class ServerBase> owner = std::weak_ptr<class ServerBase>());
		void Send(SharedSendBuffer sendBuf);
	
		bool ProcessConnect();

		const NetAddress& GetNetAddr() const { return _targetNetAddr; }
		SOCKET GetSock() const { return _socket; }
		ULONGLONG GetSessionId () const { return _sessionId; }

		ErrorCode ProcessRecv(DWORD transferredBytes);
		ErrorCode ProcessSend(DWORD transferredBytes);
		// Regist.
		void PostSend();
		void PostRecv();

		void Disconnect();

		virtual void OnConnected() = 0;
		virtual void OnDisconnected() = 0;
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) = 0;

		SessionPtr GetSessionPtr() { return std::static_pointer_cast<Session>(shared_from_this()); }
		std::shared_ptr<class ServerBase> GetServer();

		void UpdateHeartbeat(ULONGLONG now);

		void CheckHeartbeatTimeout(ULONGLONG now);
	private:
		std::weak_ptr<class ServerBase> _ownerServer;
		SRWLOCK _sendBufferLock;
		SOCKET _socket;
		NetAddress _targetNetAddr;
		ULONGLONG _sessionId;

		std::queue<SharedSendBuffer> _sendBufferQ;

		volatile char _sendFlag; // Use - 1, unUse - 0
		volatile char _isDisconn;
		
		ULONGLONG _lastTimeStamp;
	public:
		C_Utility::CRingBuffer _recvBuffer;
		RecvEvent _recvEvent;
		SendEvent _sendEvent;

	};

	class SessionManager
	{
	public:
		SessionManager(uint maxSessionCnt, SessionCreator creator) : _sessionCnt(0), _maxSessionCnt(maxSessionCnt), _createFunc(creator) { InitializeSRWLock(&_lock); }
		virtual ~SessionManager() = 0;

		SessionPtr CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr, HANDLE iocpHandle, std::shared_ptr<ServerBase> owner);
		void DeleteSession(SessionPtr session);

		uint GetSessionCnt() { return _sessionCnt; }
		void SetMaxCount(uint maxSessionCnt);

		void CheckHeartbeatTimeOut(ULONGLONG now);

	protected:
		SRWLOCK _lock;
		std::set<SessionPtr> _sessionSet;

		std::atomic<uint> _sessionCnt;
		uint _maxSessionCnt;

		SessionCreator _createFunc;
	};

	class ServerSessionManager : public SessionManager
	{
	public: ServerSessionManager(uint maxSessionCnt, SessionCreator creator) : SessionManager(maxSessionCnt, creator) {}
		  ~ServerSessionManager() {}
	};

	class ClientSessionManager : public SessionManager
	{
	public: ClientSessionManager(SessionCreator creator) : SessionManager(1, creator) {}
		  ~ClientSessionManager() {}
	};

}
