#pragma once
#include <unordered_map>

#define WM_USER_UPDATE (WM_USER + 301)

enum UIHandle : USHORT// UI ����
{
	// BUTTON
	CONN_BUTTON = 0,
	ENTER_ROOM_BUTTON,
	LOG_IN_BUTTON,
	ROOM_REFRESH_BUTTON, // �� ���ΰ�ħ ��ư
	CREATE_ROOM_BUTTON, // �� �����.

	// RO_EDIT - LABEL�� �ǹ�, �Ƹ� ����ϰ� �Ǵ� ���� ���� ����.
	RO_IP_EDIT,
	RO_PORT_EDIT,
	RO_CONN_EDIT,
	RO_ROOM_INFO_EDIT,
	RO_CHATTING_EDIT,
	RO_USER_ID_EDIT,

	// EDIT
	IP_EDIT,
	PORT_EDIT,
	INPUT_CHATTING_EDIT, // ä�� �Է� â.
	ID_EDIT,
	PW_EDIT,

	// LIST VIEW
	ROOM_INFO_LISTVIEW,
	//CHATTING_LISTVIEW, // ���� ��� ä��â.

	// COMBOBOX
	HANDLE_MAX_CNT,

	// LIST BOX
	CHATTING_LISTBOX,
	CHATTING_GROUP_LISTBOX, // �ο� ��Ȳ â

	// DIALOG
	CHATTING_DIALOG,
};

enum UIHandleType // UI Ÿ��
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

enum class TaskType : USHORT // �۾� Ÿ��
{
	CLEAR = 0, // INIT
	WRITE, // TEXT
	ADD_ITEM, // LIST_VIEW ADD ITEM
	CREATE_DIALOG, // DIALOG ����
	SHOW_DIALOG, // DIALOG ���� (MODALESS)
};
class UITaskManager
{
public:
	using UIInfo = std::pair<UIHandle, UIHandleType>; // UI�� ������ � UI������ �˷��ִ� Ÿ��.

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

	ErrorCode PostUpdateUI(uint wMsg, WPARAM wParam, LPARAM lParam); // mainUI�� ����.
	ErrorCode DirectPostUpdateUI(UIHandle handleUI, uint wMsg, WPARAM wParam, LPARAM lParam); // �� ui����
private:
	HWND _mainHwnd;

	HWND* _enumToHandleDic; // enumHandle -> handle
	std::unordered_map<HWND, UIInfo> _handleToTypeDic; // handle -> enumHandle / UIType

};

