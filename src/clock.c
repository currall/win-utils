#include "window.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>

void UpdateTime(HWND hwnd) {
    char timeBuffer[256];
    time_t rawTime;
    struct tm *timeInfo;

    while (1) {
        time(&rawTime);
        timeInfo = localtime(&rawTime);

        strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", timeInfo);
        UpdateWindowText(hwnd, timeBuffer);

        Sleep(1000); // 1 second
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char *windowName = "Clock";
    HWND hwnd = CreateCustomWindow(hInstance, "00:00:00", windowName, nCmdShow,160,60);
    if (!hwnd) {
        return 0;
    }

    WindowData *data = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (data) {
        SetFontBasedOnWindowWidth(hwnd, data);
    }

    // Create a thread to update the time
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateTime, hwnd, 0, NULL);
    if (!hThread) {
        MessageBox(NULL, "Failed to create thread", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    CloseHandle(hThread);

    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    if (data) {
        free(data);
    }

    return Msg.wParam;
}
