#include "pch.h"
#include "LobbyMonitor.h"
#include "RoomManager.h"
#include "Room.h"
#include "LobbySession.h"

void C_Utility::LobbyMonitor::MonitoringJob()
{
	_wsetlocale(LC_ALL, L"korean");

	std::vector<std::weak_ptr<C_Network::Room>> readRoomInfos;

	readRoomInfos.clear();

	C_Network::RoomManager::GetInstance().GetRoomsRead(readRoomInfos);
	
	// atomic 정확
	int realAliveRoomCount = C_Network::Room::GetAliveRoomCount();


	
	//system("cls");
	wprintf(L"+-----------------------------------------------------------------------------------+\n");
	wprintf(L"[ Session Manager's Count : %u ]\n", _sessionMgr->GetSessionCnt());

	wprintf(L"[ Real Alive Session Count : %u ] \n\n", C_Network::LobbySession::GetAliveLobbySessionCount());

	wprintf(L"[ Real Alive Room Count : %u ] \n", realAliveRoomCount);

	if (realAliveRoomCount != readRoomInfos.size())
	{
		wprintf(L"!!! realAliveRoomCount [%d] != ReadRoomInfos.size() [%d] \n", realAliveRoomCount, readRoomInfos.size());
	}

	for (auto& weakPtr : readRoomInfos)
	{
		if (!weakPtr.expired())
		{
			RoomPtr room = weakPtr.lock();

			WCHAR* pWchar = static_cast<WCHAR*>(room->GetRoomNamePtr());

			wprintf(L"[ %d번 방	[ %s ]	owner : %llu	  ( %d / %d )	준비 인원 : [%u] \n", room->GetRoomNum(), pWchar, room->GetOwnerId(), room->GetCurUserCnt(), room->GetMaxUserCnt(), room->GetReadyCnt());
		}
	}
	wprintf(L"+-----------------------------------------------------------------------------------+\n");

	wprintf(L"\n\n\n");
	return;
}
