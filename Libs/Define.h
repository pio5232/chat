#pragma once

using MemoryGuard = unsigned int;
//using uint16 = unsigned short;

using uint64 = unsigned long long;
using uint16 = unsigned short;
using uint = unsigned int;

enum : uint
{
	MESSAGE_SIZE = sizeof(WCHAR),
	IP_STRING_LEN = 22,
	USER_NAME_MAX_LEN = 20,
	MESSAGE_MAX_LEN = 30,
	ROOM_NAME_MAX_LEN = 20 + 1,
};

namespace C_Utility
{
	class CSerializationBuffer;
	class Job;
	class JobQueue;
}

namespace C_Network
{
	//using SharedIocpBase = std::shared_ptr<class IocpObjBase>;
	class Session;
	class Room;
	class User;
	using SharedSendBufChunk = std::shared_ptr<class SendBufferChunk>;
	//using SharedSendBuffer = std::shared_ptr<class SendBuffer>;
	using SharedSendBuffer = std::shared_ptr<class C_Utility::CSerializationBuffer>;

	struct RoomInfo
	{
		static uint16 GetSize() { return sizeof(ownerId) + sizeof(roomNum) + sizeof(curUserCnt) + sizeof(maxUserCnt) + sizeof(roomName); }
		ULONGLONG ownerId;
		uint16 roomNum;
		uint16 curUserCnt;
		uint16 maxUserCnt;
		WCHAR roomName[ROOM_NAME_MAX_LEN]{};
	};
}
const ULONGLONG xorTokenKey = 0x0123456789ABCDEF;

using SharedSession = std::shared_ptr<class C_Network::Session>;
using SharedJob = std::shared_ptr<class C_Utility::Job>;
using SharedJobQueue = std::shared_ptr<class C_Utility::JobQueue>;
using SharedUser = std::shared_ptr<class C_Network::User>;
using SharedRoom = std::shared_ptr<class C_Network::Room>;


