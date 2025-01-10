#include "LibsPch.h"
#include "CMonitor.h"
#include "RoomManager.h"
#include "UserManager.h"

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

void C_Utility::NetMonitor::MonitoringJob()
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


//void C_Utility::ChatMonitor::MonitoringJob()
//{
	//printf("[ Current User Count : %u ] \n", _userMgr->GetCurElementCount());

	//C_Network::RoomManager* rm = new C_Network::RoomManager(nullptr, 1, 2, nullptr);

	//{
	//	SRWLockGuard lockGuard(const_cast<SRWLOCK*>(&_roomMgr->_lock));

	//	for (auto& pair : _roomMgr->_roomMap)
	//	{
	//		C_Network::Room* room = pair.second;
	//		printf("[ Room Num : %u / ÀÎ¿ø : (%u / %u) / Owner Id : %llu / Title : %s ]\n", room->GetRoomNum(), room->GetCurUserCnt(), room->GetMaxUserCnt(), room->GetOwnerId(), room->GetRoomNamePtr());
	//	}
	//}
	//return;
//}
