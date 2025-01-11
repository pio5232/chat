#include "LibsPch.h"
#include "PacketDefine.h"
#include "CSerializationBuffer.h"

serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::PacketHeader& packetHeader)
{
	serialBuffer << packetHeader.size << packetHeader.type;

	return serialBuffer;
}

serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo)
{
	serialBuffer << roomInfo.ownerId << roomInfo.roomNum << roomInfo.curUserCnt << roomInfo.maxUserCnt;

	serialBuffer.PutData(reinterpret_cast<char*>(roomInfo.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo)
{
	serialBuffer >> roomInfo.ownerId >> roomInfo.roomNum >> roomInfo.curUserCnt >> roomInfo.maxUserCnt;

	serialBuffer.GetData(reinterpret_cast<char*>(roomInfo.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}


serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInPacket)
{
	serialBuffer << logInPacket.size << logInPacket.type << logInPacket.logInId << logInPacket.logInPw;

	return serialBuffer;
}

serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket)
{
	serialBuffer << logInResponsePacket.size << logInResponsePacket.type << logInResponsePacket.userId;

	return serialBuffer;
}

serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket)
{
	serialBuffer << makeRoomRequestPacket.size << makeRoomRequestPacket.type;

	serialBuffer.PutData(reinterpret_cast<char*>(makeRoomRequestPacket.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}

serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket)
{
	serialBuffer << makeRoomResponsePacket.size << makeRoomResponsePacket.type << makeRoomResponsePacket.isMade;

	return serialBuffer;
}

serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::EnterRoomResponsePacket& EnterRoomResponsePacket)
{
	serialBuffer << EnterRoomResponsePacket.size << EnterRoomResponsePacket.type << EnterRoomResponsePacket.bAllow << EnterRoomResponsePacket.idCnt;

	return serialBuffer;
	// TODO: 여기에 return 문을 삽입합니다.
}


serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket)
{
	serialBuffer << enterRoomNotifyPacket.size << enterRoomNotifyPacket.type << enterRoomNotifyPacket.enterUserId;

	return serialBuffer;
	// TODO: 여기에 return 문을 삽입합니다.
}

MOD2025 serializationBuffer& operator<<(serializationBuffer& serialBuffer, C_Network::LeaveRoomNotifyPacket& leaveRoomNotifyPacket)
{
	serialBuffer << leaveRoomNotifyPacket.size << leaveRoomNotifyPacket.type << leaveRoomNotifyPacket.leaveUserId;

	return serialBuffer;
}


// ------------------------------  operator >>  (빼내기), PacketHeader 뺄 필요 없다. --------------------------------


serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::LogInRequestPacket& logInRequestPacket)
{
	serialBuffer >> logInRequestPacket.logInId >> logInRequestPacket.logInPw;

	return serialBuffer;
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::LogInResponsePacket& logInResponsePacket)
{
	serialBuffer >> logInResponsePacket.userId;

	return serialBuffer;
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomRequestPacket)
{
	serialBuffer.GetData(reinterpret_cast<char*>(makeRoomRequestPacket.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::MakeRoomResponsePacket& makeRoomResponsePacket)
{
	serialBuffer >> makeRoomResponsePacket.isMade;

	return serialBuffer;
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::EnterRoomNotifyPacket& enterRoomNotifyPacket)
{
	serialBuffer >> enterRoomNotifyPacket.enterUserId;

	return serialBuffer;
	// TODO: 여기에 return 문을 삽입합니다.
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::EnterRoomRequestPacket& enterRoomRequestPacket)
{
	serialBuffer >> enterRoomRequestPacket.roomNum;

	serialBuffer.GetData(reinterpret_cast<char*>(enterRoomRequestPacket.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
	// TODO: 여기에 return 문을 삽입합니다.
}

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::LeaveRoomRequestPacket& leaveRoomRequestPacket)
{
	serialBuffer >> leaveRoomRequestPacket.roomNum;

	serialBuffer.GetData(reinterpret_cast<char*>(leaveRoomRequestPacket.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}

