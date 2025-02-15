#include "LibsPch.h"
#include "CLog.h"

#define LOG_EXPORT

void wprintf(const WCHAR* szString)
{
	wprintf(L"%s \n", szString);
}
