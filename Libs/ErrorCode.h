#pragma once

// Packet ó�� �������� �߻��� ���� �Ǵ� ��Ʈ��ũ�� �ƴ� ����.
enum class ErrorCode : uint16 // Ŭ�󿡼� ��û�� �۾��� �������� �� ó���� �Ǿ����� ���� ����� ���, ����
{
	NONE = 0, // ����

	
	CREATE_ROOM_FAILED,
	MAX_ROOM,
	ACCESS_DESTROYED_ROOM, // ������ room�� ����
	CANNOT_FIND_ROOM, // roomNum���� room�� ã�� ����. 
	ALREADY_EXIST_ROOM, // �����ϸ� �ȵǴµ�.. room�� �����ϴ� ����.

	SESSION_USER_NOT_MAPPED, // UserManager���� �����ϴ� sessionId <-> userIdx �� ���� ������ ����� ���� ���� (�������� ������)
	CANNOT_FIND_PACKET_FUNC, // PacketHandler�� �ش� ��Ŷ�� ���� �Լ��� ó���� �� ����.
	ALLOC_FAILED,	// malloc, new �� �޸� �Ҵ� ����

	DUPLICATED_MEMBER, // Dic�� ����Ϸ��� �ߴµ� �̹� �����ΰ��� ����� �Ǿ� ����.
	NOT_FOUND, // ����� ����.

	ACCESS_DELETE_MEMBER, // ������ ����� ����.

	POST_UI_UPDATE_FAILED, // UITaskManager�� postMessage�� �����ߴ�.
};