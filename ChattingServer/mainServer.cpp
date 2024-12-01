#include "pch.h"
#include "TlsProfiler.h"
#include <crtdbg.h>
#include "CCrashDump.h"
#include <thread>
#include <chrono>
#include "ChattingServer.h"
#include <conio.h>

using namespace C_Network;
ChattingServer server(NetAddress(std::wstring(L"192.168.0.10"),ServerPort),6000);

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	if (server.Begin() != C_Network::NetworkErrorCode::NONE)
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

	if (server.End() != C_Network::NetworkErrorCode::NONE)
	{
		printf("End : 에러 발생\n");
		return 0;
	}
}
