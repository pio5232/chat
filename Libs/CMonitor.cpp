#include "LibsPch.h"
#include "CMonitor.h"

C_Utility::CMonitor::~CMonitor() 
{
}

void C_Utility::CMonitor::Begin()
{
	
	_monitoringFlag = true;
	
	_monitorThread = std::thread([&]() {this->ProcessMonitoring(); });

	return;
}

void C_Utility::CMonitor::End()
{
	_monitoringFlag = false;

	if (_monitorThread.joinable())
		_monitorThread.join();

	return;
}

void C_Utility::CMonitor::ProcessMonitoring() 
{
	while (_monitoringFlag)
	{
		MonitoringJob();

		printf("\n");

		Sleep(1000);
	}
}
//
//void C_Utility::NetMonitor::MonitoringJob()
//{
//	while (_monitoringFlag)
//	{
//		printf("[ Current Connect Session Count : %u ]\n", _sessionMgr->GetSessionCnt());
//
//		//printf("[ Send Completion Count : %u ]\n", InterlockedExchange(&_sendCount, 0));
//
//		//printf("[ Recv Completion Count : %u ]\n", InterlockedExchange(&_recvCount, 0));
//
//		printf("\n");
//
//		Sleep(1000);
//	}
//	return;
//}

