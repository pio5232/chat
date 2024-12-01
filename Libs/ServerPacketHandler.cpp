#include "LibsPch.h"
#include "ServerPacketHandler.h"
#include "ChattingClient.h"
// -------------------------------------------------------
//					ServerPacketHandler
// -------------------------------------------------------


//ErrorCode C_Network::ChattingServerPacketHandler::ProcessChatToUserPacket(C_Utility::CSerializationBuffer& buffer)
//{
//	//uint16 messageLen;
//
//	//buffer >> messageLen;
//
//	//// TODO : NEW -> POOL
//	//ChatUserResponsePacket* chatPacket = static_cast<ChatUserResponsePacket*>(malloc(sizeof(ChatUserResponsePacket) + messageLen));
//
//	//if (!chatPacket)
//	//	return C_Network::NetworkErrorCode::MESSAGE_SEND_FAILED;
//
//	//chatPacket->size = messageLen + sizeof(chatPacket->messageLen);
//	//chatPacket->messageLen = messageLen;
//	//buffer.GetData(chatPacket->payLoad, messageLen);
//
//	//SharedSendBuffer sendBuffer = MakePacket(CHAT_TO_USER_RESPONSE_PACKET, *chatPacket);
//
//	//C_Network::NetworkErrorCode result = _owner->SendToRoom(sendBuffer, -1);
//
//	//// TODO : 정상 처리
//	
//	//return result;
//	return C_Network::NetworkErrorCode::NONE;
//}
//
//ErrorCode C_Network::ChattingServerPacketHandler::ProcessChatToRoomPacket(C_Utility::CSerializationBuffer& buffer)
//{
//	return ErrorCode();
//}
//
//ErrorCode C_Network::ChattingServerPacketHandler::ProcessChatNotifyPacket(C_Utility::CSerializationBuffer& buffer)
//{
//	return ErrorCode();
//}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessRoomListResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    C_Utility::Log(L"RoomList Recv");

    RoomListResponsePacket roomListResponsePacket;
    
    roomListResponsePacket.roomCnt;


    buffer >> roomListResponsePacket.roomCnt;
    
    ErrorCode uiInitRet = _uiTaskManager->DirectPostUpdateUI(UIHandle::ROOM_INFO_LISTBOX, LB_RESETCONTENT, 0, 0);

    if (ErrorCode::NONE != uiInitRet)
    {
        C_Utility::Log(L"Room List Initializing is Failed");
        return ErrorCode::POST_UI_UPDATE_FAILED;
    }
    for (int i = 0; i < roomListResponsePacket.roomCnt; i++)
    {
        // TODO : new 안 하게 바꾸기
        RoomInfo* roomInfo = new RoomInfo;
        
        buffer >> *roomInfo;

        ErrorCode roomUpdateRet = _uiTaskManager->PostUpdateUI(WM_USER_UPDATE, MAKEWPARAM(TaskType::ADD_ITEM, UIHandle::ROOM_INFO_LISTBOX), reinterpret_cast<LPARAM>(roomInfo));

        if (roomUpdateRet != ErrorCode::NONE)
        {
            C_Utility::Log(L"");

        }
    }
    return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessLogInResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    LogInResponsePacket responsePacket;
 
    buffer >> responsePacket;
    
    // TODO : new 안 하게 바꾸기tring(static_cast<ULONGLONG>(responsePacket.userId));

    std::wstring message = L"User Id : " + std::to_wstring(responsePacket.userId);

    WCHAR* wstr = new WCHAR[message.length() + 1]{};

    wcscpy_s(wstr, message.length()+1, message.c_str());

    C_Utility::Log(L"LogInPacket Recv\n");

    ErrorCode ret = _uiTaskManager->PostUpdateUI(WM_USER_UPDATE, MAKEWPARAM(TaskType::WRITE, UIHandle::RO_USER_ID_EDIT), reinterpret_cast<LPARAM>(wstr));
   
    if (ret != ErrorCode::NONE)
    {
        C_Utility::Log(L"LogIn Update UI Failed\n");
        return ret;
    }
    else
        C_Utility::Log(L"LogInUI Update Send \n");

    RoomListRequestPacket roomListRequestPacket;

    C_Network::SharedSendBuffer roomListRequestSendBuffer = MakePacket<PacketHeader>(roomListRequestPacket);
    
    _owner->Send(roomListRequestSendBuffer);

    C_Utility::Log(L"RoomListRequestPacket Send");

    return ret;
}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessChatToRoomResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessChatToUserResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessEnterRoomResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    return ErrorCode::NONE;
}

ErrorCode C_Network::ChattingServerPacketHandler::ProcessLeaveRoomResponsePacket(C_Utility::CSerializationBuffer& buffer)
{
    return ErrorCode::NONE;
}
