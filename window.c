#include "window.h"
#include <stdlib.h>
#include <string.h>

#define CLASS_NAME "myWindowClass"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    WindowData *data;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT *createStruct = (CREATESTRUCT *)lParam;
        data = (WindowData *)createStruct->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
    } else {
        data = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (msg) {
        case WM_LBUTTONDOWN:
            ReleaseCapture();
            SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
            break;

        case WM_CLOSE: // handles closing program
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY: // closes program
            if (data && data->hFont) {
                DeleteObject(data->hFont);
            }
            PostQuitMessage(0);
            break;

        case WM_SIZE: { // resizes text if window resized
            int width = LOWORD(lParam);
            int fontSize = width / 8;

            if (data && data->hFont) {
                DeleteObject(data->hFont);
            }

            if (data) {
                data->hFont = CreateFont(
                    fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            if (data) {
                SelectObject(hdc, data->hFont);
                SetTextColor(hdc, RGB(0, 0, 0));
                SetBkMode(hdc, TRANSPARENT);

                DrawText(hdc, data->text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            EndPaint(hwnd, &ps);
            break;
        }

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND CreateCustomWindow(HINSTANCE hInstance, const char *text, const char *windowName, int nCmdShow)
{
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }

    WindowData *data = (WindowData *)malloc(sizeof(WindowData));
    if (!data) {
        MessageBox(NULL, "Memory allocation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return NULL;
    }
    strncpy(data->text, text, sizeof(data->text) - 1);
    data->text[sizeof(data->text) - 1] = '\0';
    data->hFont = NULL;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST, CLASS_NAME, windowName, WS_POPUP | WS_VISIBLE | WS_SIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120, NULL, NULL, hInstance, data);

    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        free(data);
        return NULL;
    }

    ShowWindow(hwnd, nCmdShow);
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    UpdateWindow(hwnd);

    return hwnd;
}
