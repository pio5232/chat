#include "LibsPch.h"
#include "Utils.h"

//namespace C_Utility
//{
//	//template<typename T, typename... Args>
//	//ManagerPool<T, Args...>::~ManagerPool()
//	//{
//	//	for (T* elementPtr : _elementArr)
//	//	{
//	//		delete elementPtr;
//	//	}
//	//	_elementArr.clear();
//	//}
//}

void ExecuteProcess(const std::wstring& path, const std::wstring& args)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	std::wstring commandLine = path + L" " + args;
	bool ret = CreateProcess(NULL, &commandLine[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);

	if (!ret)
	{
		printf("Create Process Failed [%d]\n", GetLastError());
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

}

Vector3::Vector3() : x(0), y(0), z(0)
{
}

Vector3::Vector3(float inX, float inY, float inZ) : x(inX), y(inY), z(inZ)
{
}
