#include "LibsPch.h"
#include "UITaskManager.h"

UITaskManager::UITaskManager() : _mainHwnd(nullptr)
{
	_enumToHandleDic = new HWND[HANDLE_MAX_CNT];

	for (int i = 0; i < HANDLE_MAX_CNT; i++)
	{
		_enumToHandleDic[i] = nullptr;
	}	
}

void UITaskManager::RegisterHandle(const UIHandle enumHandle, HWND handle, UIHandleType handleType)
{
	// CreateWindow가 false -> handle이 null인 상황 또는 이미 해당 UI 핸들이 등록되어 있는 상황엔 실패 리턴
	if (!handle || _enumToHandleDic[enumHandle] != nullptr)
		CCrash(L"Handle Register Err\n");

	_enumToHandleDic[enumHandle] = handle;

	if (_handleToTypeDic[handle].second != NOT_FOUND)
		CCrash(L"HandleType is already Alloced");

	_handleToTypeDic[handle].first = enumHandle;
	_handleToTypeDic[handle].second = handleType;
}

// wParam Hiword -> handleUI, wParam Loword -> 작업 타입, lParma -> 이용할 작업물
// main으로 보냄
ErrorCode UITaskManager::PostUpdateUI(uint uMsg, WPARAM wParam, LPARAM lParam)
{
	//std::cout << "PostUpdateUI " << handle <<"  msg " <<uMsg << "\n";

	bool bRet = PostMessage(_mainHwnd, uMsg, wParam, lParam);
	
	if (!bRet) // PostMessage => 비동기로 메시지큐에 전달. SendMessage => 동기로 전달이 완료될 때까지 블락. 비동기로 전송하도록 한다.
	{
		wprintf(L"PostUpdateUI Failed [GetLastError - %d]\n", GetLastError());

		return ErrorCode::POST_UI_UPDATE_FAILED;
	}

	return ErrorCode::NONE;
}

ErrorCode UITaskManager::DirectPostUpdateUI(UIHandle handleUI, uint wMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = GetHandle(handleUI);

	bool bRet = PostMessage(hWnd, wMsg, wParam, lParam);

	if (!bRet) 
	{
		wprintf(L"DirectPostUpdateUI Failed [GetLastError - %d]\n", GetLastError());
		
		return ErrorCode::POST_UI_UPDATE_FAILED;
	}

	return ErrorCode::NONE;
}
