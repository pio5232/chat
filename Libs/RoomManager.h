#pragma once

#include "Room.h"

namespace C_Network
{
	class RoomManager : public C_Utility::ManagerPool<Room, class ServerBase* , uint16 >
	{
	public:
		RoomManager(class ServerBase* owner, uint16 maxRoomCount, uint16 maxRoomUserCnt);
		~RoomManager();

		ErrorCode SendToAllUser(SharedSendBuffer& sendBuffer);
		ErrorCode SendToRoom(uint16 roomNum, SharedSendBuffer& sendBuffer);
		ErrorCode SendToUserRoomInfo(ULONGLONG sessionId, SharedSendBuffer& sendBuffer);
		
	private:
		const uint16 _maxRoomUserCnt; // room당 최대 user 수
		std::unordered_map<uint16, Room*> _usingRoomDic;
		ServerBase* _owner;
	};
}