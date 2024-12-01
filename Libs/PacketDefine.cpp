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

serializationBuffer& operator>>(serializationBuffer& serialBuffer, C_Network::RoomInfo& roomInfo)
{
	serialBuffer >> roomInfo.ownerId >> roomInfo.roomNum >> roomInfo.curUserCnt >> roomInfo.maxUserCnt;

	serialBuffer.GetData(reinterpret_cast<char*>(roomInfo.roomName), ROOM_NAME_MAX_LEN * MESSAGE_SIZE);

	return serialBuffer;
	// TODO: 여기에 return 문을 삽입합니다.
}
