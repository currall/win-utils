#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>

typedef struct {
    char text[256]; // stores display text
    HFONT hFont;    // stores font
} WindowData;

HWND CreateCustomWindow(HINSTANCE hInstance, const char *text, const char *windowName, int nCmdShow);

#endif 
