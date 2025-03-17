#include <windows.h>
#include <stdio.h>

#define TRAY_ICON_ID 1001

HICON CreateBatteryIcon(int percentage) {
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 16, 16);
    SelectObject(hMemDC, hBitmap);

    // Clear background with custom color
    COLORREF bg_color = RGB(0,0,0);
    if (percentage >= 95){
        bg_color = RGB(0, 150, 50);
    } else if (percentage <= 25){
        bg_color = RGB(255, 0, 0);
    }
    HBRUSH hBrush = CreateSolidBrush(bg_color); // Black background
    RECT rect = {0, 0, 16, 16};
    FillRect(hMemDC, &rect, hBrush);
    DeleteObject(hBrush);

    // Set text color
    SetTextColor(hMemDC, RGB(255, 255, 255)); // White text

    // Draw text
    char text[4];
    sprintf(text, "%d", percentage);
    SetBkMode(hMemDC, TRANSPARENT);
    DrawText(hMemDC, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    HICON hIcon = CreateIconIndirect(&(ICONINFO){TRUE, 0, 0, hBitmap, hBitmap});

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);
    DeleteObject(hBitmap);

    return hIcon;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static NOTIFYICONDATA nid = {0};

    switch (uMsg) {
    case WM_CREATE: {
        SYSTEM_POWER_STATUS status;
        GetSystemPowerStatus(&status);

        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = TRAY_ICON_ID;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
        nid.uCallbackMessage = WM_APP + 1;

        int batteryPercentage = status.BatteryLifePercent;
        nid.hIcon = CreateBatteryIcon(batteryPercentage);
        sprintf(nid.szTip, "Battery: %d%%", batteryPercentage);

        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    }

    case WM_TIMER: {
        SYSTEM_POWER_STATUS status;
        GetSystemPowerStatus(&status);

        int batteryPercentage = status.BatteryLifePercent;
        DestroyIcon(nid.hIcon);
        nid.hIcon = CreateBatteryIcon(batteryPercentage);
        sprintf(nid.szTip, "Battery: %d%%", batteryPercentage);

        Shell_NotifyIcon(NIM_MODIFY, &nid);
        break;
    }

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "TrayIconClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Tray Icon Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        return 1;
    }
    
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    SetTimer(hwnd, 1, 10000, NULL); // update every 10s

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
