#pragma once

using MemoryGuard = unsigned int;
//using uint16 = unsigned short;
using uint16 = unsigned short;
using uint = unsigned int;

namespace C_Utility
{
	class CSerializationBuffer;
}
namespace C_Network
{
	//using SharedIocpBase = std::shared_ptr<class IocpObjBase>;
	//using SharedSession = std::shared_ptr<class Session>;

	using SharedSendBufChunk = std::shared_ptr<class SendBufferChunk>;
	//using SharedSendBuffer = std::shared_ptr<class SendBuffer>;
	using SharedSendBuffer = std::shared_ptr<class C_Utility::CSerializationBuffer>;
}

enum : uint
{
	MESSAGE_SIZE = sizeof(WCHAR),
	USER_NAME_MAX_LEN = 20,
	MESSAGE_MAX_LEN = 30,
	ROOM_NAME_MAX_LEN = 20 + 1,
}
//using SharedIocpBase = std::shared_ptr<class C_Network::IocpObjBase>;

#define TODO_TLS_LOG_ERROR
#define TODO_TLS_LOG_SUCCESS

#define TODO_LOG_SUCCESS
#define TODO_LOG_ERROR
#define TODO_LOG

#define TODO_UPDATE_EX_LIST// 나중에 비동기로 수정해야 하는 코드. (CONNECT, ACCEPT, New->Mempool 수정 등)..
#define TODO_DEFINITION // 사용하기위해 정의해야하는 코드.


//#define TODO_LOG_ERROR_WSA(x) printf("[%s WsaGetLastError - %d] \n",#x, WSAGetLastError())
