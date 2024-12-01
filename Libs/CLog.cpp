#include "LibsPch.h"
#include "CLog.h"

#define LOG_EXPORT

void C_Utility::Log(const WCHAR* szString)
{
	wprintf(L"%s \n", szString);
}
