#include "pch.h"
#include "TlsProfiler.h"
#include "Define.h"
#include <crtdbg.h>
#include "CCrashDump.h"
#include <thread>
#include <chrono>
#include "LobbyServer.h"
#include <conio.h>
#include "NetworkBase.h"
#include "PacketHandler.h"
#include "Session.h"
#include "RoomManager.h"
#include "BufferMaker.h"
#include "LobbySession.h"
using namespace C_Network;

class EchoPacket
{
public:
	uint16 size = 0;
	ULONGLONG data;
};

class EchoSession : public C_Network::Session
{
public:
	EchoSession()
	{
	}
	virtual void OnConnected() override {}
	virtual void OnDisconnected() override {}
	virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) override 
	{
		EchoPacket packet;

		packet.size = 8;
		buffer >> packet.data;

		SharedSendBuffer sendBuffer = C_Network::BufferMaker::MakeSendBuffer(sizeof(packet));

		*sendBuffer << packet.size << packet.data;

		Send(sendBuffer);

		return;
	}

};

	class ChattingServer : public ServerBase
	{
	public:
		ChattingServer(const NetAddress& netAddr, uint maxSessionCnt, SessionCreator creator) : ServerBase(netAddr, maxSessionCnt, creator) {}
		~ChattingServer() {}

		virtual bool OnConnectionRequest(const SOCKADDR_IN& clientInfo) { return true;  }
		virtual void OnError(int errCode, WCHAR* cause) {}

#ifdef LAN
		std::unique_ptr<LanServer> _lanServer;
#endif // LAN

	};

//ChattingServer server(NetAddress(std::wstring(L"127.0.0.1"),ServerPort),6000);

int main() 
{
	// 파일 읽어서 세팅 
	RoomManager::GetInstance().Init(100,4);

	//std::shared_ptr<ChattingServer> server = std::make_shared<ChattingServer>(NetAddress(std::wstring(L"127.0.0.1"), ServerPort), 9000, []() { return std::static_pointer_cast<Session>(std::make_shared<EchoSession>()); });
	std::shared_ptr<LobbyServer> server = std::make_shared<LobbyServer>(NetAddress(std::wstring(L"127.0.0.1"), ServerPort), 6000,
		[]() { return std::static_pointer_cast<Session>(std::make_shared<LobbySession>()); });

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	if (server->Begin() != ErrorCode::NONE)
	{
		printf("Begin : 에러 발생\n");
		return 0;
	}

	while (1)
	{
		if (_kbhit())
		{
			char c = _getch();
			if (c == 'q' || c == 'Q')
				break;
		}
	}

	if (server->End() != ErrorCode::NONE)
	{
		printf("End : 에러 발생\n");
		return 0;
	}
}
