#pragma once
#include "CMonitor.h"

namespace C_Utility
{
	class LobbyMonitor : public C_Utility::CMonitor
	{
	public:
		LobbyMonitor(class C_Network::SessionManager* sessionMgr) : _sessionMgr(sessionMgr) {}
		virtual void MonitoringJob() override;
	private:
		class C_Network::SessionManager* _sessionMgr;
	};
}