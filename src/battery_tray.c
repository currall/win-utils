#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "window.h"

#define TRAY_ICON_ID 1001
#define ID_TRAY_EXIT 1002
#define ID_TRAY_LOCK 1003

static HWND g_batteryWindowHwnd = NULL;
static HINSTANCE g_hGlobalInstance = NULL;

typedef struct { // track locked status, and time locked
    BOOL locked;
    time_t lockTime;
} APP_STATE;

HICON CreateBatteryIcon(int percentage, int charging, BOOL locked) {
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 16, 16);
    SelectObject(hMemDC, hBitmap);

    COLORREF bg_color = RGB(0, 0, 0);
    COLORREF fg_color = RGB(255, 255, 255);

    if (locked) {
        fg_color = RGB(120, 120, 255);
    } else if (charging) {
        fg_color = RGB(0, 200, 100);
    } else {
        if (percentage <= 6) {
            bg_color = RGB(255, 0, 0);
            fg_color = RGB(0, 0, 0);
        } else if (percentage <= 20) {
            fg_color = RGB(255, 50, 0);
        } else if (percentage <= 30) {
            fg_color = RGB(255, 160, 40);
        }
    }

    HBRUSH hBrush = CreateSolidBrush(bg_color);
    RECT rect = {0, 0, 16, 16};
    FillRect(hMemDC, &rect, hBrush);
    DeleteObject(hBrush);

    SetTextColor(hMemDC, fg_color);
    SetBkMode(hMemDC, TRANSPARENT);
    char text[4];
    if (percentage == 100)
        sprintf(text, ":D");
    else
        sprintf(text, "%d", percentage);
    DrawText(hMemDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    HICON hIcon = CreateIconIndirect(&(ICONINFO){TRUE, 0, 0, hBitmap, hBitmap});

	// clean up
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);
    DeleteObject(hBitmap);

    return hIcon;
}

void ShowContextMenu(HWND hwnd) {
    APP_STATE *state = (APP_STATE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING | (state->locked ? MF_CHECKED : MF_UNCHECKED), ID_TRAY_LOCK, "Lock");
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quit");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

void UpdateTrayIcon(HWND hwnd, NOTIFYICONDATA *nid) {
    APP_STATE *state = (APP_STATE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    SYSTEM_POWER_STATUS status;
    GetSystemPowerStatus(&status);

    DestroyIcon(nid->hIcon);
    nid->hIcon = CreateBatteryIcon(status.BatteryLifePercent, status.ACLineStatus, state->locked);

    if (state->locked) {
        struct tm *tm_info = localtime(&state->lockTime);
        char timeStr[16];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", tm_info);
        sprintf(nid->szTip, "Locked at %s", timeStr);
    } else {
        sprintf(nid->szTip, "Battery: %d%%", status.BatteryLifePercent);
    }

    Shell_NotifyIcon(NIM_MODIFY, nid);
}

void ShowBatteryWindow() {
    // If window already exists and is valid, just bring it to the front
    if (g_batteryWindowHwnd && IsWindow(g_batteryWindowHwnd)) {
        SetForegroundWindow(g_batteryWindowHwnd);
        return;
    }

    // Get current battery status
    SYSTEM_POWER_STATUS status;
    GetSystemPowerStatus(&status);

    char batteryText[256];
    const char* chargeStatus;
    const char* lifeTime;
    char lifeTimeStr[100];

    if (status.ACLineStatus == 1) {
        chargeStatus = "Charging";
    } else if (status.ACLineStatus == 0) {
        chargeStatus = "Discharging";
    } else {
        chargeStatus = "Unknown";
    }

    if (status.BatteryLifeTime != (DWORD)-1) {
         sprintf(lifeTimeStr, "Time Remaining: %ld min", status.BatteryLifeTime / 60);
         lifeTime = lifeTimeStr;
    } else {
        lifeTime = "Time Remaining: (Unknown)";
    }

    // Format the text for the window
    sprintf(batteryText, "Battery: %d%%\n%s\n%s", 
        status.BatteryLifePercent, 
        chargeStatus,
        (status.ACLineStatus == 1) ? "" : lifeTime); // Only show time remaining if discharging

    // Create the custom window
    g_batteryWindowHwnd = CreateCustomWindow(
        g_hGlobalInstance,   // Use the stored global instance
        batteryText,         // The text to display
        "Battery Status",    // The window title (when not hidden)
        SW_SHOWNORMAL,
        300,                 // Initial width
        200                  // Initial height
    );
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static NOTIFYICONDATA nid = {0};

    switch (uMsg) {
    case WM_CREATE: { // create process
        APP_STATE *state = (APP_STATE *)malloc(sizeof(APP_STATE));
        state->locked = FALSE;
        state->lockTime = 0;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);

        SYSTEM_POWER_STATUS status;
        GetSystemPowerStatus(&status);

        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = TRAY_ICON_ID;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_APP + 1;

        nid.hIcon = CreateBatteryIcon(status.BatteryLifePercent, status.ACLineStatus, state->locked);
        sprintf(nid.szTip, "Battery: %d%%", status.BatteryLifePercent);

        Shell_NotifyIcon(NIM_ADD, &nid);
        SetTimer(hwnd, 1, 500, NULL);
        break;
    }

    case WM_TIMER: { // update icon if not locked
        APP_STATE *state = (APP_STATE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!state->locked) {
            UpdateTrayIcon(hwnd, &nid);
        }
        break;
    }

    case WM_APP + 1: // right click
        if (lParam == WM_RBUTTONUP) {
            ShowContextMenu(hwnd);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) { // exit button
            PostQuitMessage(0);
        } else if (LOWORD(wParam) == ID_TRAY_LOCK) { // lock button
            APP_STATE *state = (APP_STATE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            state->locked = !state->locked;
            if (state->locked) {
                time(&state->lockTime);
            }
            UpdateTrayIcon(hwnd, &nid);
        }
        break;

    case WM_DESTROY: { // on close
        Shell_NotifyIcon(NIM_DELETE, &nid);
        APP_STATE *state = (APP_STATE *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (state) free(state);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
	g_hGlobalInstance = hInstance;
	
	const char CLASS_NAME[] = "TrayIconClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "Tray Icon Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 1;

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
