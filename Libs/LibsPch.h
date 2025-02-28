#pragma once

//#define LAN
//#define ECHO
#include <iostream>

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <thread>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#include "Define.h"

#include "GlobalInstance.h"
#include "TLSInstance.h"

#include "Utils.h"
#include "NetworkUtils.h"

#include "JobQueue.h"
//extern thread_local C_Network::SharedSendBufChunk sendBufChunks;

#include "ErrorCode.h"
#include "Session.h"
#include "CLog.h"
