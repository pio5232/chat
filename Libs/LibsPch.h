#pragma once

#include <iostream>

#include <WinSock2.h>
#include <Windows.h>
#include <thread>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#include "Define.h"

#include "Utils.h"
#include "NetworkUtils.h"

#include "ErrorCode.h" // 컨텐츠 에러 코드.
#include "NetworkErrorCode.h" // 네트워크 에러 코드
#include "Session.h"
#include "CLog.h"
inline void CCrash(const WCHAR* resaon)
{
	wprintf(L"%s\n", resaon);

	int* bp = nullptr;
	*bp = (int)0x12345678;
}
