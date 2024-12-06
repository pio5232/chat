#include "LibsPch.h"
#include "PacketDefine.h"
#include "CSerializationBuffer.h"

serializationBuffer& operator<< (serializationBuffer& serialBuffer, C_Network::PacketHeader& packetHeader)
{
	serialBuffer << packetHeader.size << packetHeader.type;

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


// ------------------------------  operator >>  (»©³»±â), PacketHeader »¬ ÇÊ¿ä ¾ø´Ù. --------------------------------


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

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::MakeRoomRequestPacket& makeRoomResponsePacket)
{
	serialBuffer.GetData(reinterpret_cast<char*>(makeRoomResponsePacket.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}


serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo)
{
	serialBuffer >> roomInfo.ownerId >> roomInfo.roomNum >> roomInfo.curUserCnt >> roomInfo.maxUserCnt;

	serialBuffer.GetData(reinterpret_cast<char*>(roomInfo.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
}
