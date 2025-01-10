#pragma once

namespace C_Utility
{
	class CMonitor
	{
	public: CMonitor() {}
		virtual ~CMonitor() = 0;

		virtual void ProcessMonitoring();
		virtual void MonitoringJob() abstract = 0; // 상속받은 클래스에서 모니터링 할 작업을 한다.
		void Begin();
		void End();

	protected:
		std::thread _monitorThread;
		volatile bool _monitoringFlag = false;

	};

	class NetMonitor : public CMonitor
	{
	public:
		NetMonitor(class C_Network::SessionManager* sessionMgr) : _sessionMgr(sessionMgr) {}

		virtual void MonitoringJob() override;

		ULONG IncSendCount() { return InterlockedIncrement(&_sendCount); }
		ULONG IncRecvCount() { return InterlockedIncrement(&_recvCount); }
	private:		
		const class C_Network::SessionManager* const _sessionMgr;

		volatile ULONG _sendCount = 0; // 1초에 몇 번씩 send 완료통지처리 / recv 완료통지 처리 체크,
		volatile ULONG _recvCount = 0;
	};

	//class ChatMonitor : public CMonitor
	//{
	////public:
	////	ChatMonitor(class C_Network::RoomManager* roomMgr, class C_Network::UserManager* userMgr) : _roomMgr(roomMgr), _userMgr(userMgr) {}
	////	virtual void MonitoringJob() override;
	////private:
	////	const class C_Network::RoomManager* const _roomMgr;
	////	const class C_Network::UserManager* const _userMgr;
	//};

}
