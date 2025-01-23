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


void C_Utility::ChatMonitor::MonitoringJob()
{
	_wsetlocale(LC_ALL, L"korean");

	std::vector<std::weak_ptr<C_Network::Room>> readRoomInfos;

	readRoomInfos.clear();

	_roomMgr->GetRoomsRead(readRoomInfos);
		
	printf("[ Current User Count : %u ] \n", _userMgr->GetUserCount() );
		
	for (auto& weakPtr : readRoomInfos)
	{
		if (!weakPtr.expired())
		{
			SharedRoom room = weakPtr.lock();
			
			WCHAR* pWchar = static_cast<WCHAR*>(room->GetRoomNamePtr());

			wprintf(L"[ %d¹ø ¹æ	[ %s ]	owner : %lld	  ( %d / %d ) \n", room->GetRoomNum(), pWchar, room->GetOwnerId(), room->GetCurUserCnt(), room->GetMaxUserCnt());
		}
	}

	return;
}
