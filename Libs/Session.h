#pragma once

#include <queue>
#include <stack>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace C_Network
{
	/*----------------------
			IocpEvent
	----------------------*/
	enum class IocpEventType : unsigned char
	{
		Accept,
		Connect,
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
		 
		SharedSession _owner;

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

	struct ConnectEvent : public IocpEvent
	{
	public:
		ConnectEvent() : IocpEvent(IocpEventType::Connect) {}
	};


	/*------------------------------
				Session 
	------------------------------*/
	struct Session : public std::enable_shared_from_this<Session>
	{
	public:
		Session(SOCKET sock,const SOCKADDR_IN* pSockAddr);
		~Session();

		void Send(SharedSendBuffer sendBuf);

		//bool ProcessRecv(DWORD transferredBytes);
		C_Network::NetworkErrorCode ProcessSend(DWORD transferredBytes);
		bool ProcessConnect();
		bool ProcessAccept();
		bool ProcessDisconnect();

		const NetAddress& GetNetAddr() const { return _targetNetAddr; }
		SOCKET GetSock() const { return _socket; }
		ULONGLONG GetId () const { return _sessionId; }

		// Regist.
		void PostSend();
		void PostRecv();
		void PostConnect();
		void PostAccept();

		void Disconnect();

		bool CheckDisconnect();

	private:

		SRWLOCK _sendBufferLock;
		SOCKET _socket;
		NetAddress _targetNetAddr;
		ULONGLONG _sessionId;

		std::queue<SharedSendBuffer> _sendBufferQ;

		volatile char _sendFlag; // Use - 1, unUse - 0
		volatile char _isDisconn;
	public:
	    volatile ULONG _ioCount;
	public:
		C_Utility::CRingBuffer _recvBuffer;
		RecvEvent _recvEvent;
		SendEvent _sendEvent;
		ConnectEvent _connectEvent;

	};


	class SessionManager
	{
	public:
		SessionManager(uint maxSessionCnt) : _sessionCnt(0), _maxSessionCnt(maxSessionCnt) { InitializeSRWLock(&_lock); _sessionToIdDic.reserve(maxSessionCnt); _idToSessionDic.reserve(maxSessionCnt); }
		virtual ~SessionManager() = 0;

		SharedSession CreateSession(SOCKET sock, SOCKADDR_IN* pSockAddr, HANDLE iocpHandle);
		void DeleteSession(SharedSession session);

		SharedSession GetSession(ULONGLONG sessionId);
		uint GetSessionCnt() { return _sessionCnt; }

	protected:
		SRWLOCK _lock;
		std::unordered_map<SharedSession, ULONGLONG> _sessionToIdDic; // Session -> sessionId
		std::unordered_map<ULONGLONG, SharedSession> _idToSessionDic; // sessionId -> Session
		std::atomic<uint> _sessionCnt;
		const uint _maxSessionCnt;
	};

	class ServerSessionManager : public SessionManager
	{
	public: ServerSessionManager(uint maxSessionCnt) : SessionManager(maxSessionCnt) {}
		  ~ServerSessionManager() {}
	};

	class ClientSessionManager : public SessionManager
	{
	public: ClientSessionManager() : SessionManager(1) {}
		  ~ClientSessionManager() {}
	};

}
