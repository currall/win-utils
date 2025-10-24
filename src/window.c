#include "window.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#define FONT_PADDING_FRACTION 0.85f 
#define ASPECT_RATIO_APPROX 0.5f 
#define CLASS_NAME "myWindowClass"

int CalculateFontSize(int windowWidth, int windowHeight, const char* text)
{
    int heightConstraint = (int)((float)windowHeight * FONT_PADDING_FRACTION);
    float textLen = (float)strlen(text);
    if (textLen < 1.0f) textLen = 1.0f; // prevent division by zero
    
    int widthConstraint = (int)((float)windowWidth * FONT_PADDING_FRACTION / (textLen * ASPECT_RATIO_APPROX));
    int finalFontSize = min(heightConstraint, widthConstraint);
    
    if (finalFontSize < 10) finalFontSize = 10;
    return -finalFontSize; 
}

void SetFontBasedOnWindowWidth(HWND hwnd, WindowData *data)
{
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    int fontSize = CalculateFontSize(rect.right, rect.bottom, data->text); 

    if (data && data->hFont) {
        DeleteObject(data->hFont);
    }

    data->hFont = CreateFont(
        fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");

    InvalidateRect(hwnd, NULL, TRUE);
}

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

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            if (data && data->hFont) {
                DeleteObject(data->hFont);
            }
            PostQuitMessage(0);
            break;

        case WM_SIZE: {
            SetFontBasedOnWindowWidth(hwnd, data);
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
		
		case WM_KEYDOWN:
            if (wParam == VK_SPACE) {
                // Get the current window style
                LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);

                if (style & WS_POPUP) {
                    // Title bar is hidden (WS_POPUP is set), so SHOW it.
                    // Remove WS_POPUP and add standard window styles.
                    style &= ~WS_POPUP;
                    style |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX);
                } else {
                    // Title bar is visible, so HIDE it.
                    // Remove standard styles and add WS_POPUP.
                    style &= ~(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU);
                    style |= (WS_POPUP | WS_SIZEBOX); // Keep WS_SIZEBOX
                }

                // Apply the new style
                SetWindowLongPtr(hwnd, GWL_STYLE, style);

                // Force the window to redraw its frame/non-client area
                SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            }
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND CreateCustomWindow(HINSTANCE hInstance, const char *text, const char *windowName, int nCmdShow, int width, int height)
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
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, data);

    if (!hwnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        free(data);
        return NULL;
    }

    SetFontBasedOnWindowWidth(hwnd, data);

    ShowWindow(hwnd, nCmdShow);
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    UpdateWindow(hwnd);

    return hwnd;
}

void UpdateWindowText(HWND hwnd, const char *newText)
{
    WindowData *data = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (data) {
        strncpy(data->text, newText, sizeof(data->text) - 1);
        data->text[sizeof(data->text) - 1] = '\0';
        InvalidateRect(hwnd, NULL, TRUE);
    }
}
