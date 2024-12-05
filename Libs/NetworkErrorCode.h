#pragma once

namespace C_Network
{
	enum class NetworkErrorCode : uint16
	{
		NONE = 0, // 정상 처리.
		WSA_START_UP_ERROR,

		// SERVER ERROR CODE
		CREATE_COMPLETION_PORT_FAILED = 100, // 
		CREATE_SOCKET_FAILED,
		LISTEN_FAILED,
		BIND_FAILED,
		//

		// CLIENT ERROR CODE
		CLIENT_NOT_INITIALIZED,
		CLIENT_CONNECT_FAILED,
		CLIENT_NOT_CONNECTED,

		SET_NET_ADDR_FAILED, // NET ADDR 설정 실패.
		// COMMON
		SET_SOCK_OPT_FAILED,

		RECV_BUF_OVERFLOW = 200, // 수신 버퍼가 증가시킬 수 있는 크기보다 크게 transferredBytes가 올 때 발생함.
		RECV_BUF_DEQUE_FAILED, // 절대 일어나면 안되는 에러. 발생했을 때는 링버퍼 설계를 확인해야함.

		SEND_LEN_ZERO = 300, // 0 Byte Send

		//

		CANNOT_FIND_PACKET_FUNC = 1000, // PacketHandler에 해당 패킷을 처리할 수 있는 함수가 등록되지 않음.
	};
}