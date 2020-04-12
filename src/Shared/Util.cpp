#include "Util.h"

std::string StringConvertFromUTF16(LPCWSTR str)
{
	std::string stdstr;
	int length_utf8 = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);

	if (length_utf8 != 0)
	{
		char* str_utf8 = new char[length_utf8];
		
		if (WideCharToMultiByte(CP_UTF8, 0, str, -1, str_utf8, length_utf8, nullptr, nullptr) != 0)
		{
			stdstr = str_utf8;
		}

		delete[] str_utf8;
	}
		
	return stdstr;
}

std::wstring WStringConvertFromUTF8(const char * str)
{
	std::wstring wstr;
	int length_utf16 = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);

	if (length_utf16 != 0)
	{
		WCHAR* str_utf16 = new WCHAR[length_utf16];

		if (MultiByteToWideChar(CP_UTF8, 0, str, -1, str_utf16, length_utf16) != 0)
		{
			wstr = str_utf16;
		}

		delete[] str_utf16;
	}

	return wstr;
}

//This is only needed for std::error_code.message(), thanks to it being in the local ANSI codepage instead of UTF-8
std::wstring WStringConvertFromLocalEncoding(const char* str)
{
    std::wstring wstr;
    int length_utf16 = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);

    if (length_utf16 != 0)
    {
        WCHAR* str_utf16 = new WCHAR[length_utf16];

        if (MultiByteToWideChar(CP_ACP, 0, str, -1, str_utf16, length_utf16) != 0)
        {
            wstr = str_utf16;
        }

        delete[] str_utf16;
    }

    return wstr;
}

void OffsetTransformFromSelf(vr::HmdMatrix34_t& matrix, float offset_right, float offset_up, float offset_forward)
{
	matrix.m[0][3] += offset_right * matrix.m[0][0];
	matrix.m[1][3] += offset_right * matrix.m[1][0];
	matrix.m[2][3] += offset_right * matrix.m[2][0];

	matrix.m[0][3] += offset_up * matrix.m[0][1];
	matrix.m[1][3] += offset_up * matrix.m[1][1];
	matrix.m[2][3] += offset_up * matrix.m[2][1];

	matrix.m[0][3] += offset_forward * matrix.m[0][2];
	matrix.m[1][3] += offset_forward * matrix.m[1][2];
	matrix.m[2][3] += offset_forward * matrix.m[2][2];
}

void OffsetTransformFromSelf(Matrix4& matrix, float offset_right, float offset_up, float offset_forward)
{
    matrix[12] += offset_right * matrix[0];
    matrix[13] += offset_right * matrix[1];
    matrix[14] += offset_right * matrix[2];

    matrix[12] += offset_up * matrix[4];
    matrix[13] += offset_up * matrix[5];
    matrix[14] += offset_up * matrix[6];

    matrix[12] += offset_forward * matrix[8];
    matrix[13] += offset_forward * matrix[9];
    matrix[14] += offset_forward * matrix[10];
}

vr::TrackedDeviceIndex_t GetFirstVRTracker()
{
    //Get the first generic tracker
    for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
    {
        if (vr::VRSystem()->GetTrackedDeviceClass(i) == vr::TrackedDeviceClass_GenericTracker)
        {
            return i;
        }
    }

    return vr::k_unTrackedDeviceIndexInvalid;
}

DEVMODE GetDevmodeForDisplayID(int display_id)
{
    if (display_id == -1)
        display_id = 0;

    DEVMODE mode = {0};
    DISPLAY_DEVICE DispDev = {0};

    DispDev.cb = sizeof(DISPLAY_DEVICE);

    if (EnumDisplayDevices(nullptr, display_id, &DispDev, 0))
    {
        mode.dmSize = sizeof(DEVMODE);

        if (EnumDisplaySettings(DispDev.DeviceName, ENUM_CURRENT_SETTINGS, &mode) == FALSE)
        {
            mode.dmSize = 0;    //Reset dmSize to 0 if the call failed
        }
    }

    return mode;
}

int GetMonitorRefreshRate(int display_id)
{
    DEVMODE mode = GetDevmodeForDisplayID(display_id);

    if ( (mode.dmSize != 0) && (mode.dmFields & DM_DISPLAYFREQUENCY) ) //Something would be wrong if that field isn't supported, but let's check anyways
    {
        return mode.dmDisplayFrequency;
    }

    return 60;	//Fallback value
}

void CenterRectToMonitor(LPRECT prc)
{
    HMONITOR    hmonitor;
    MONITORINFO mi;
    RECT        rc;
    int         w = prc->right  - prc->left;
    int         h = prc->bottom - prc->top;

    //Get the nearest monitor to the passed rect
    hmonitor = ::MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);

    //Get monitor rect
    mi.cbSize = sizeof(mi);
    ::GetMonitorInfo(hmonitor, &mi);

    rc = mi.rcMonitor;

    //Center the passed rect to the monitor rect 
    prc->left   = rc.left + (rc.right  - rc.left - w) / 2;
    prc->top    = rc.top  + (rc.bottom - rc.top  - h) / 2;
    prc->right  = prc->left + w;
    prc->bottom = prc->top  + h;
}

void CenterWindowToMonitor(HWND hwnd, bool use_cursor_pos)
{
    RECT rc;
    ::GetWindowRect(hwnd, &rc);

    HMONITOR    hmonitor;
    MONITORINFO mi;
    RECT rcm;
    int w = rc.right  - rc.left;
    int h = rc.bottom - rc.top;

    if (use_cursor_pos) //Cursor position is used to determine the screen to center on
    {
        POINT mouse_pos = {0};
        ::GetCursorPos(&mouse_pos); 
        RECT mouse_rc;
        mouse_rc.left   = mouse_pos.x;
        mouse_rc.right  = mouse_pos.x;
        mouse_rc.top    = mouse_pos.y;
        mouse_rc.bottom = mouse_pos.y;

        hmonitor = ::MonitorFromRect(&mouse_rc, MONITOR_DEFAULTTONEAREST);
    }
    else
    {
        //Get the nearest monitor to the passed rect
        hmonitor = ::MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    }

    //Get monitor rect
    mi.cbSize = sizeof(mi);
    ::GetMonitorInfo(hmonitor, &mi);

    rcm = mi.rcMonitor;

    //Center the passed rect to the monitor rect 
    rc.left   = rcm.left + (rcm.right  - rcm.left - w) / 2;
    rc.top    = rcm.top  + (rcm.bottom - rcm.top  - h) / 2;
    rc.right  = rc.left + w;
    rc.bottom = rc.top  + h;

    ::SetWindowPos(hwnd, nullptr, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void ForceScreenRefresh()
{
    //This is a hacky workaround for occasionally not getting a full desktop image after resetting duplication until a screen change occurs
    //For secondary screens that could possibly not happen until manual user interaction, so instead we force the desktop to redraw itself
    //Unproblematic, but proper fix would be welcome too
    if (HWND shell_window = ::GetShellWindow())
        ::SendMessage(shell_window, WM_SETTINGCHANGE, 0, 0); 
}

bool IsProcessElevated() 
{
    bool ret = false;
    HANDLE handle_token = nullptr;
    if (::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &handle_token) ) 
    {
        TOKEN_ELEVATION elevation;
        DWORD cb_size = sizeof(TOKEN_ELEVATION);

        if (::GetTokenInformation(handle_token, TokenElevation, &elevation, sizeof(elevation), &cb_size) ) 
        {
            ret = elevation.TokenIsElevated;
        }
    }

    if (handle_token) 
    {
        CloseHandle(handle_token);
    }

    return ret;
}

bool FileExists(LPCTSTR path)
{
    DWORD attrib = GetFileAttributes(path);

    return ((attrib != INVALID_FILE_ATTRIBUTES) && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

void StopProcessByWindowClass(LPCTSTR class_name)
{
    //Try to close it gracefully first so it can save the config
    if (HWND window_handle = ::FindWindow(class_name, nullptr))
    {
        ::PostMessage(window_handle, WM_QUIT, 0, 0);
    }

    ULONGLONG start_tick = ::GetTickCount64();

    while ( (::FindWindow(class_name, nullptr) != nullptr) && (::GetTickCount64() - start_tick < 3000) ) //Wait 3 seconds max
    {
        Sleep(5); //Should be usually quick though, so don't wait around too long
    }

    //Still running? Time to kill it
    if (HWND window_handle = ::FindWindow(class_name, nullptr))
    {
        DWORD pid;
        ::GetWindowThreadProcessId(window_handle, &pid);

        HANDLE phandle;
        phandle = ::OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, pid);

        if (phandle != nullptr)
        {
            ::TerminateProcess(phandle, 0);
            ::WaitForSingleObject(phandle, INFINITE);
            ::CloseHandle(phandle);
        }
    }
}

//This ain't pretty, but GetKeyNameText() works with scancodes, which are not exactly the same and the output strings aren't that nice either (and always localized)
//Those duplicate lines are optimized away to the same address by any sane compiler, nothing to worry about.
const char* g_VK_name[256] = 
{
    "[None]",
    "Left Mouse",
    "Right Mouse",
    "Control Break",
    "Middle Mouse",
    "X1 Mouse",
    "X2 Mouse",
    "[Undefined] (7)",
    "Backspace",
    "Tab",
    "[Reserved] (10)",
    "[Reserved] (11)",
    "Clear",
    "Enter",
    "[Undefined] (14)",
    "[Undefined] (15)",
    "Shift",
    "Ctrl",
    "Alt",
    "Pause",
    "Caps Lock",
    "IME Kana",
    "[Undefined] (22)",
    "IME Junja",
    "IME Final",
    "IME Kanji",
    "[Undefined] (26)",
    "Esc",
    "IME Convert",
    "IME Non Convert",
    "IME Accept",
    "IME Mode Change",
    "Space",
    "Page Up",
    "Page Down",
    "End",
    "Home",
    "Left Arrow",
    "Up Arrow",
    "Right Arrow",
    "Down Arrow",
    "Select",
    "Print",
    "Execute",
    "Print-Screen",
    "Insert",
    "Delete",
    "Help",
    "0",  //0x30 - 0x5A are ASCII equivalent, but we want iterate this array directly for listing too
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "[Undefined] (58)",
    "[Undefined] (59)",
    "[Undefined] (60)",
    "[Undefined] (61)",
    "[Undefined] (62)",
    "[Undefined] (63)",
    "[Undefined] (64)",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "Left Windows",
    "Right Windows",
    "Context Menu",
    "[Reserved] (94)",
    "Sleep",
    "Numpad 0",
    "Numpad 1",
    "Numpad 2",
    "Numpad 3",
    "Numpad 4",
    "Numpad 5",
    "Numpad 6",
    "Numpad 7",
    "Numpad 8",
    "Numpad 9",
    "Numpad Multiply",
    "Numpad Add",
    "Separator",
    "Numpad Subtract",
    "Numpad Decimal",
    "Numpad Divide",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "[Unassigned] (136)",
    "[Unassigned] (137)",
    "[Unassigned] (138)",
    "[Unassigned] (139)",
    "[Unassigned] (140)",
    "[Unassigned] (141)",
    "[Unassigned] (142)",
    "[Unassigned] (143)",
    "Num Lock",
    "Scroll Lock",
    "OEM 1",
    "OEM 2",
    "OEM 3",
    "OEM 4",
    "OEM 5",
    "[Unassigned] (151)",
    "[Unassigned] (152)",
    "[Unassigned] (153)",
    "[Unassigned] (154)",
    "[Unassigned] (155)",
    "[Unassigned] (156)",
    "[Unassigned] (157)",
    "[Unassigned] (158)",
    "[Unassigned] (159)",
    "Left Shift",
    "Right Shift",
    "Left Ctrl",
    "Right Ctrl",
    "Left Alt",
    "Right Alt",
    "Browser Back",
    "Browser Forward",
    "Browser Refresh",
    "Browser Stop",
    "Browser Search",
    "Browser Favorites",
    "Browser Home",
    "Volume Mute",
    "Volume Down",
    "Volume Up",
    "Media Next",
    "Media Previous",
    "Media Stop",
    "Media Play/Pause",
    "Launch Mail",
    "Select Media",
    "Launch Application 1",
    "Launch Application 2",
    "[Reserved] (184)",
    "[Reserved] (185)",
    "[Layout-Specific 1] (186)",
    "+",
    ",",
    "-",
    ".",
    "[Layout-Specific 2] (191)",
    "[Layout-Specific 3] (192)",
    "[Reserved] (193)",
    "[Reserved] (194)",
    "[Reserved] (195)",
    "[Reserved] (196)",
    "[Reserved] (197)",
    "[Reserved] (198)",
    "[Reserved] (199)",
    "[Reserved] (200)",
    "[Reserved] (201)",
    "[Reserved] (202)",
    "[Reserved] (203)",
    "[Reserved] (204)",
    "[Reserved] (205)",
    "[Reserved] (206)",
    "[Reserved] (207)",
    "[Reserved] (208)",
    "[Reserved] (209)",
    "[Reserved] (210)",
    "[Reserved] (211)",
    "[Reserved] (212)",
    "[Reserved] (213)",
    "[Reserved] (214)",
    "[Reserved] (215)",
    "[Unassigned] (216)",
    "[Unassigned] (217)",
    "[Unassigned] (218)",
    "[Layout-Specific 4] (219)",
    "[Layout-Specific 5] (220)",
    "[Layout-Specific 6] (221)",
    "[Layout-Specific 7] (222)",
    "[Layout-Specific 8] (223)",
    "[Reserved] (224)",
    "[Reserved] (225)",
    "[Layout-Specific 102] (226)", //Big jump, but that's VK_OEM_102, so dunno
    "OEM 6",
    "OEM 7",
    "IME Process",
    "OEM 8",
    "Unicode Packet",
    "[Unassigned] (232)",
    "OEM 9",
    "OEM 10",
    "OEM 11",
    "OEM 12",
    "OEM 13",
    "OEM 14",
    "OEM 15",
    "OEM 16",
    "OEM 17",
    "OEM 18",
    "OEM 19",
    "OEM 20",
    "OEM 21",
    "Attn",
    "CrSel",
    "ExSel",
    "Erase EOF",
    "Play",
    "Zoom",
    "NoName",
    "PA1",
    "OEM Clear",
    "[Unassigned] (255)",
};

//Attempt at making a list of indicies to sort the key codes in a way an end-user would make expect them in, leaving the obscure stuff at the end.
const unsigned char g_VK_name_order_list[256] = 
{ 0, 1, 2, 4, 5, 6, 27, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 44, 145, 19, 8, 9, 13, 20, 16, 17, 18, 160, 161, 162, 163, 164, 165, 
91, 92, 93, 32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 
89, 90, 187, 189, 190, 188, 45, 46, 36, 35, 33, 34, 37, 38, 39, 40, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 144, 107, 109, 106, 111, 110, 167, 166,
168, 169, 170, 171, 172, 173, 175, 174, 176, 177, 179, 178, 180, 181, 182, 183, 186, 191, 192, 219, 220, 221, 222, 223, 226, 146, 147, 148, 149, 150, 227,
228, 230, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 21, 23, 25, 28, 29,
30, 31, 229, 3, 95, 12, 41, 42, 43, 47, 108, 246, 247, 248, 249, 250, 251, 252, 253, 254, 231, 7, 14, 15, 22, 26, 58, 59, 60, 61, 62, 63, 64, 24, 136, 137,
138, 139, 140, 141, 142, 143, 151, 152, 153, 154, 155, 156, 157, 158, 159, 216, 217, 218, 232, 255, 10, 11, 94, 184, 185, 193, 194, 195, 196, 197, 198, 199,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 224, 225 };

const char* GetStringForKeyCode(unsigned char keycode)
{
    return g_VK_name[keycode];
}

unsigned char GetKeyCodeForListID(unsigned char list_id)
{
    return g_VK_name_order_list[list_id];
}