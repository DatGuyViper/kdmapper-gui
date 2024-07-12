#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <filesystem>
#include "kdmapper.hpp"

HANDLE iqvw64e_device_handle;
HWND g_hWndLog;

#define IDC_CHECK_FREE          101
#define IDC_CHECK_MDL           102
#define IDC_CHECK_IND_PAGES     103
#define IDC_CHECK_PASS_ALLOC_PTR 104
#define IDC_BUTTON_SELECT_FILE  105
#define IDC_BUTTON_MAP_DRIVER   106
#define IDC_EDIT_FILE_PATH      107
#define IDC_EDIT_LOG            108

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void LogMessage(const wchar_t* message);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
void ShowErrorMessage(const wchar_t* message);
void CreateControls(HWND hWnd);
void HandleCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void SelectDriverFile(HWND hWnd);
void MapDriver(HWND hWnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"KDMapperWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"KDMapper GUI",
        WS_OVERLAPPEDWINDOW,

        CW_USEDEFAULT, CW_USEDEFAULT, 620, 645,

        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateControls(hWnd);
        break;

    case WM_COMMAND:
        HandleCommand(hWnd, wParam, lParam);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hWnd) {
    CreateWindow(L"BUTTON", L"Free Pool Memory", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 20, 200, 30, hWnd, (HMENU)IDC_CHECK_FREE, NULL, NULL);

    CreateWindow(L"BUTTON", L"MDL Memory Usage", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 60, 200, 30, hWnd, (HMENU)IDC_CHECK_MDL, NULL, NULL);

    CreateWindow(L"BUTTON", L"Allocate Independent Pages", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 100, 200, 30, hWnd, (HMENU)IDC_CHECK_IND_PAGES, NULL, NULL);

    CreateWindow(L"BUTTON", L"Pass Allocation Ptr as First Param", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        20, 140, 250, 30, hWnd, (HMENU)IDC_CHECK_PASS_ALLOC_PTR, NULL, NULL);

    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        20, 180, 460, 30, hWnd, (HMENU)IDC_EDIT_FILE_PATH, NULL, NULL);

    CreateWindow(L"BUTTON", L"Select Driver", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        490, 180, 100, 30, hWnd, (HMENU)IDC_BUTTON_SELECT_FILE, NULL, NULL);

    g_hWndLog = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL |
        ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | ES_NOHIDESEL,
        20, 220, 560, 320, hWnd, (HMENU)IDC_EDIT_LOG, GetModuleHandle(NULL), NULL);

    CreateWindow(L"BUTTON", L"Map Driver", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        20, 550, 100, 30, hWnd, (HMENU)IDC_BUTTON_MAP_DRIVER, NULL, NULL);

    HFONT hFontTitle = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    SendMessage(g_hWndLog, WM_SETFONT, (WPARAM)hFontTitle, TRUE);
}

void HandleCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
    case IDC_BUTTON_SELECT_FILE:
        SelectDriverFile(hWnd);
        break;

    case IDC_BUTTON_MAP_DRIVER:
        MapDriver(hWnd);
        break;

    case IDC_CHECK_FREE:
        if (HIWORD(wParam) == BN_CLICKED) {
            LogMessage(L"Free Pool Memory option selected.\r\n");
        }
        break;

    case IDC_CHECK_MDL:
        if (HIWORD(wParam) == BN_CLICKED) {
            LogMessage(L"MDL Memory Usage option selected.\r\n");
        }
        break;

    case IDC_CHECK_IND_PAGES:
        if (HIWORD(wParam) == BN_CLICKED) {
            LogMessage(L"Allocate Independent Pages option selected.\r\n");
        }
        break;

    case IDC_CHECK_PASS_ALLOC_PTR:
        if (HIWORD(wParam) == BN_CLICKED) {
            LogMessage(L"Pass Allocation Ptr as First Param option selected.\r\n");
        }
        break;

    default:
        break;
    }
}

void SelectDriverFile(HWND hWnd) {
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"Driver (*.sys)\0*.SYS\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFileName);
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        SetDlgItemText(hWnd, IDC_EDIT_FILE_PATH, szFileName);
        wchar_t logMessage[MAX_PATH + 50];
        swprintf_s(logMessage, L"Driver selected: %s\r\n", szFileName);
        LogMessage(logMessage);
    }
}

void MapDriver(HWND hWnd) {
    wchar_t szFileName[MAX_PATH];
    GetDlgItemText(hWnd, IDC_EDIT_FILE_PATH, szFileName, MAX_PATH);

    bool free = IsDlgButtonChecked(hWnd, IDC_CHECK_FREE) == BST_CHECKED;
    bool mdlMode = IsDlgButtonChecked(hWnd, IDC_CHECK_MDL) == BST_CHECKED;
    bool indPagesMode = IsDlgButtonChecked(hWnd, IDC_CHECK_IND_PAGES) == BST_CHECKED;
    bool passAllocationPtr = IsDlgButtonChecked(hWnd, IDC_CHECK_PASS_ALLOC_PTR) == BST_CHECKED;

    std::filesystem::path driverPath(szFileName);

    if (!std::filesystem::exists(driverPath)) {
        ShowErrorMessage(L"File doesn't exist.");
        return;
    }

    iqvw64e_device_handle = intel_driver::Load();
    if (iqvw64e_device_handle == INVALID_HANDLE_VALUE) {
        ShowErrorMessage(L"Failed to load driver.");
        return;
    }

    std::vector<uint8_t> raw_image;
    if (!utils::ReadFileToMemory(driverPath, &raw_image)) {
        ShowErrorMessage(L"Failed to read driver.");
        intel_driver::Unload(iqvw64e_device_handle);
        return;
    }
    kdmapper::AllocationMode mode = kdmapper::AllocationMode::AllocatePool;
    if (mdlMode && indPagesMode) {
        ShowErrorMessage(L"Too many allocation modes selected.");
        intel_driver::Unload(iqvw64e_device_handle);
        return;
    }
    else if (mdlMode) {
        mode = kdmapper::AllocationMode::AllocateMdl;
    }
    else if (indPagesMode) {
        mode = kdmapper::AllocationMode::AllocateIndependentPages;
    }

    NTSTATUS exitCode = 0;
    if (!kdmapper::MapDriver(iqvw64e_device_handle, raw_image.data(), 0, 0, free, true, mode, passAllocationPtr, nullptr, &exitCode)) {
        ShowErrorMessage(L"Failed to map driver.");
    }

    if (!intel_driver::Unload(iqvw64e_device_handle)) {
        LogMessage(L"Warning: Failed to fully unload driver.");
    }
    LogMessage(L"Driver mapped successfully.\r\n");
}

void LogMessage(const wchar_t* message) {
    SendMessage(g_hWndLog, EM_SETSEL, -1, -1);
    SendMessage(g_hWndLog, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(message));
    SendMessage(g_hWndLog, EM_SCROLLCARET, 0, 0);
}

void ShowErrorMessage(const wchar_t* message) {
    MessageBox(NULL, message, L"Error", MB_ICONERROR | MB_OK);
}
