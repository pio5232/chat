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
	// CreateWindow�� false -> handle�� null�� ��Ȳ �Ǵ� �̹� �ش� UI �ڵ��� ��ϵǾ� �ִ� ��Ȳ�� ���� ����
	if (!handle || _enumToHandleDic[enumHandle] != nullptr)
		CCrash(L"Handle Register Err\n");

	_enumToHandleDic[enumHandle] = handle;

	if (_handleToTypeDic[handle].second != NOT_FOUND)
		CCrash(L"HandleType is already Alloced");

	_handleToTypeDic[handle].first = enumHandle;
	_handleToTypeDic[handle].second = handleType;
}

// wParam Hiword -> handleUI, wParam Loword -> �۾� Ÿ��, lParma -> �̿��� �۾���
// main���� ����
ErrorCode UITaskManager::PostUpdateUI(uint uMsg, WPARAM wParam, LPARAM lParam)
{
	//std::cout << "PostUpdateUI " << handle <<"  msg " <<uMsg << "\n";

	bool bRet = PostMessage(_mainHwnd, uMsg, wParam, lParam);
	
	if (!bRet) // PostMessage => �񵿱�� �޽���ť�� ����. SendMessage => ����� ������ �Ϸ�� ������ ���. �񵿱�� �����ϵ��� �Ѵ�.
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
