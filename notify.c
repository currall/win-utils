#include "window.h"
#include <stdio.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char *text = (lpCmdLine && lpCmdLine[0] != '\0') ? lpCmdLine : "Default Text";
    const char *windowName = "Notification";

    HWND hwnd = CreateCustomWindow(hInstance, text, windowName, nCmdShow);
    if (!hwnd) {
        return 0;
    }

    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    WindowData *data = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (data) {
        free(data);
    }

    return Msg.wParam;
}
