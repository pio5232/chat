#include "pch.h"
#include "LobbyMonitor.h"
#include "RoomManager.h"

void C_Utility::LobbyMonitor::MonitoringJob()
{
	_wsetlocale(LC_ALL, L"korean");

	std::vector<std::weak_ptr<C_Network::Room>> readRoomInfos;

	readRoomInfos.clear();

	C_Network::RoomManager::GetInstance().GetRoomsRead(readRoomInfos);

	//system("cls");

	wprintf(L"[ Current Session Count : %u ] \n", _sessionMgr->GetSessionCnt());

	for (auto& weakPtr : readRoomInfos)
	{
		if (!weakPtr.expired())
		{
			RoomPtr room = weakPtr.lock();

			WCHAR* pWchar = static_cast<WCHAR*>(room->GetRoomNamePtr());

			wprintf(L"[ %d번 방	[ %s ]	owner : %llU	  ( %d / %d )	준비 인원 : [%u] \n", room->GetRoomNum(), pWchar, room->GetOwnerId(), room->GetCurUserCnt(), room->GetMaxUserCnt(), room->GetReadyCnt());
		}
	}

	wprintf(L"\n\n\n");
	return;
}
