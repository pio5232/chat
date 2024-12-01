#include "LibsPch.h"
#include "User.h"

#include <sstream>

C_Network::User::User()
{
}

void C_Network::User::InitInfo(ULONGLONG userId, ULONGLONG sessionId)
{
	_winCnt = 0;
	_loseCnt = 0;

	static ULONGLONG nickGenerator = 1;
	nickGenerator++;

	std::wstringstream stream;

	stream << L"자동생성" << nickGenerator;

	wcscpy_s(_nickName, stream.str().c_str());

	_userId = userId;
	_sessionId = sessionId;
}
