#pragma once
#include <unordered_map>

#define WM_USER_UPDATE (WM_USER + 301)

enum UIHandle : USHORT// UI 종류
{
	// BUTTON
	CONN_BUTTON = 0,
	ENTER_ROOM_BUTTON,
	LOG_IN_BUTTON,
	ROOM_REFRESH_BUTTON, // 방 새로고침 버튼
	CREATE_ROOM_BUTTON, // 방 만들기.

	// RO_EDIT - LABEL을 의미, 아마 사용하게 되는 일은 없을 것임.
	RO_IP_EDIT,
	RO_PORT_EDIT,
	RO_CONN_EDIT,
	RO_ROOM_INFO_EDIT,
	RO_CHATTING_EDIT,
	RO_USER_ID_EDIT,

	// EDIT
	IP_EDIT,
	PORT_EDIT,
	INPUT_CHATTING_EDIT, // 채팅 입력 창.
	ID_EDIT,
	PW_EDIT,

	// LIST VIEW
	ROOM_INFO_LISTVIEW,
	//CHATTING_LISTVIEW, // 실제 사용 채팅창.

	// COMBOBOX
	HANDLE_MAX_CNT,

	// LIST BOX
	CHATTING_LISTBOX,
	CHATTING_GROUP_LISTBOX, // 인원 현황 창

	// DIALOG
	CHATTING_DIALOG,
};

enum UIHandleType // UI 타입
{
	NOT_FOUND = 0,
	BUTTON,
	EDIT,
	RO_EDIT,
	LISTVIEW,
	LISTBOX,
	COMBOBOX,
	DIALOG,
};

enum class TaskType : USHORT // 작업 타입
{
	CLEAR = 0, // INIT
	WRITE, // TEXT
	ADD_ITEM, // LIST_VIEW ADD ITEM
	CREATE_DIALOG, // DIALOG 생성
	SHOW_DIALOG, // DIALOG 생성 (MODALESS)
};
class UITaskManager
{
public:
	using UIInfo = std::pair<UIHandle, UIHandleType>; // UI의 종류와 어떤 UI인지를 알려주는 타입.

	UITaskManager();
	~UITaskManager()
	{
		delete[] _enumToHandleDic;
	}
	void SetMain(HWND mainHwnd) { _mainHwnd = mainHwnd; }
	void RegisterHandle(UIHandle enumHandle, HWND handle, UIHandleType handleType);
	const UIInfo& GetHandleInfo(HWND handle)
	{
		return _handleToTypeDic[handle];
	}
	const HWND GetHandle(UIHandle enumHandle) const
	{
		return _enumToHandleDic[enumHandle];
	}

	ErrorCode PostUpdateUI(uint wMsg, WPARAM wParam, LPARAM lParam); // mainUI에 보냄.
	ErrorCode DirectPostUpdateUI(UIHandle handleUI, uint wMsg, WPARAM wParam, LPARAM lParam); // 내 ui조작
private:
	HWND _mainHwnd;

	HWND* _enumToHandleDic; // enumHandle -> handle
	std::unordered_map<HWND, UIInfo> _handleToTypeDic; // handle -> enumHandle / UIType

};

