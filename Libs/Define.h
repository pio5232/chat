#pragma once

using MemoryGuard = unsigned int;
//using uint16 = unsigned short;

using uint64 = unsigned long long;
using uint16 = unsigned short;
using uint = unsigned int;

enum : uint
{
	MESSAGE_SIZE = sizeof(WCHAR),
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

using SharedSession = std::shared_ptr<class C_Network::Session>;
using SharedJob = std::shared_ptr<class C_Utility::Job>;
using SharedJobQueue = std::shared_ptr<class C_Utility::JobQueue>;
using SharedUser = std::shared_ptr<class C_Network::User>;
using SharedRoom = std::shared_ptr<class C_Network::Room>;



//using SharedIocpBase = std::shared_ptr<class C_Network::IocpObjBase>;

#define TODO_TLS_LOG_ERROR
#define TODO_TLS_LOG_SUCCESS

#define TODO_LOG_SUCCESS
#define TODO_LOG_ERROR
#define TODO_LOG

#define TODO_UPDATE_EX_LIST// 나중에 비동기로 수정해야 하는 코드. (CONNECT, ACCEPT, New->Mempool 수정 등)..
#define TODO_DEFINITION // 사용하기위해 정의해야하는 코드.


//#define TODO_LOG_ERROR_WSA(x) printf("[%s WsaGetLastError - %d] \n",#x, WSAGetLastError()) 
