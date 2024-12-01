#pragma once

namespace C_Utility
{
	class CMonitor
	{
	public: CMonitor();
		virtual ~CMonitor() = 0;

		virtual void ProcessMonitoring();
		void Begin();
		void End();

	protected:
		std::thread _monitorThread;
		volatile bool _monitoringFlag = false;
	};

	class NetMonitor : public CMonitor
	{
	public:
		NetMonitor(class C_Network::SessionManager* sessionMgr) : _sessionMgr(sessionMgr){}
		virtual void ProcessMonitoring() override;

		ULONG IncSendCount() { return InterlockedIncrement(&_sendCount); }
		ULONG IncRecvCount() { return InterlockedIncrement(&_recvCount); }
	private:
		const class C_Network::SessionManager* const _sessionMgr;
		volatile ULONG _sendCount = 0; // 1초에 몇 번씩 send 완료통지처리 / recv 완료통지 처리 체크,
		volatile ULONG _recvCount = 0;
	};
}
