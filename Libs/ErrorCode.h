#pragma once

// Packet 처리 과정에서 발생한 에러 또는 네트워크가 아닌 에러.
enum class ErrorCode : uint16 // 클라에서 요청한 작업이 서버에서 잘 처리가 되었는지 진행 결과를 출력, 전송
{
	NONE = 0, // 정상

	
	CREATE_ROOM_FAILED,
	MAX_ROOM,
	ACCESS_DESTROYED_ROOM, // 삭제된 room에 접근
	CANNOT_FIND_ROOM, // roomNum으로 room을 찾지 못함. 
	ALREADY_EXIST_ROOM, // 존재하면 안되는데.. room이 존재하는 상태.

	SESSION_USER_NOT_MAPPED, // UserManager에서 관리하는 sessionId <-> userIdx 에 대한 맵핑이 제대로 되지 않음 (여러가지 이유로)
	CANNOT_FIND_PACKET_FUNC, // PacketHandler가 해당 패킷에 대한 함수를 처리할 수 없음.
	ALLOC_FAILED,	// malloc, new 등 메모리 할당 실패

	DUPLICATED_MEMBER, // Dic에 등록하려고 했는데 이미 무엇인가가 등록이 되어 있음.
	NOT_FOUND, // 멤버가 없음.

	ACCESS_DELETE_MEMBER, // 삭제된 멤버에 접근.

	POST_UI_UPDATE_FAILED, // UITaskManager에 postMessage가 실패했다.
};