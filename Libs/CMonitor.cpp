#include "LibsPch.h"
#include "CMonitor.h"
#include "Session.h"

C_Utility::CMonitor::CMonitor() 
{
}

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

void C_Utility::CMonitor::ProcessMonitoring() {
}

void C_Utility::NetMonitor::ProcessMonitoring()
{
	//while (_monitoringFlag)
	//{
	//	printf("[ Current Connect Session Count : %u ]\n", _sessionMgr->GetCurElementCount());

	//	printf("[ Send Completion Count : %u ]\n", InterlockedExchange(&_sendCount, 0));

	//	printf("[ Recv Completion Count : %u ]\n", InterlockedExchange(&_recvCount, 0));

	//	printf("\n");

	//	Sleep(1000);
	//}
	return;
}
