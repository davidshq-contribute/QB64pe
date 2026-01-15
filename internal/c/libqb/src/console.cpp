//----------------------------------------------------------------------------------------------------------------------
// QB64-PE Console Support
// Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "console.h"
#include "error_handle.h"
#include "qbs.h"

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

// External console state from libqb.cpp
extern int32_t console;
extern int32_t console_active;
extern int32_t console_child;
extern int32_t console_image;

// Console input state - these need to be accessible from mouse.cpp
extern int32_t consolekey;
extern int32_t consolemousex;
extern int32_t consolemousey;
extern int32_t consolebutton;

int32_t func__console() {
    if (is_error_pending())
        return -1;
    return console_image;
}

void sub__console(int32_t onoff) { // on=1 off=2
    if (!console)
        return; // command does nothing if console unavailable
    if (onoff == 1) {
        // turn on
        if (!console_active) {
#ifdef QB64_WINDOWS
            if (console_child) {
                ShowWindow(GetConsoleWindow(), SW_SHOWNOACTIVATE);
            }
#endif
            console_active = 1;
        }
    } else {
        // turn off
        if (console_active) {
#ifdef QB64_WINDOWS
            if (console_child) {
                ShowWindow(GetConsoleWindow(), SW_HIDE);
            }
#endif
            console_active = 0;
        }
    }
}

void sub__consoletitle(qbs *s) {
#ifdef QB64_WINDOWS
    char *title;
    title = (char *)malloc(s->len + 1);
    title[s->len] = '\0'; // add NULL terminator
    memcpy(title, s->chr, s->len);
    if (console) {
        if (console_active) {
            SetConsoleTitleA(title);
            Sleep(40);
        }
    }
    free(title);
#endif
}

void sub__consolefont(qbs *FontName, int32_t FontSize) {
#ifdef QB64_WINDOWS
#    if WINVER >= 0x0600 // this block is not compatible with XP
    SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
    HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
    static int OneTimePause;
    if (!OneTimePause) { // a slight delay so the console can be properly created and registered with Windows, before we try to change fonts with it.
        Sleep(500);
        OneTimePause = 1; // after the first pause, the console should be created, so we don't need any more delays in the future.
    }
    CONSOLE_FONT_INFOEX info = {0};
    info.cbSize = sizeof(info);
    info.dwFontSize.Y = FontSize; // leave X as zero
    info.FontWeight = FW_NORMAL;
    if (FontName->len > 0) { // if we don't pass a font name, don't change the existing one.
        const size_t cSize = FontName->len;
        wchar_t *wc = new wchar_t[32];
        mbstowcs(wc, (char *)FontName->chr, cSize);
        wcscpy(info.FaceName, wc);
        delete[] wc;
    }

    SetCurrentConsoleFontEx(cl_conout, NULL, &info);
#    endif
#endif
}

void sub__console_cursor(int32_t visible, int32_t cursorsize, int32_t passed) {
#ifdef QB64_WINDOWS
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;

    GetConsoleCursorInfo(consoleHandle, &info); // get the original info, so we reuse it, unless the user called for a change.

    if (visible == 1)
        info.bVisible = TRUE; // cursor is set to show
    if (visible == 2)
        info.bVisible = FALSE; // set to hide
    if (passed && cursorsize >= 0 && cursorsize <= 100)
        info.dwSize = cursorsize; // the user passed the cursor size, of a suitable size

    SetConsoleCursorInfo(consoleHandle, &info);
#endif
}

int32_t func__getconsoleinput() {
#ifdef QB64_WINDOWS
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD irInputRecord;
    DWORD dwEventsRead, fdwMode, dwMode;
    CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;

    GetConsoleMode(hStdin, (LPDWORD)&dwMode);
    fdwMode = ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hStdin, fdwMode);
    fdwMode = dwMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdin, fdwMode);

    DWORD numEvents = 0;
    GetNumberOfConsoleInputEvents(hStdin, &numEvents);
    if (numEvents) {
        ReadConsoleInputA(hStdin, &irInputRecord, 1, &dwEventsRead);
        switch (irInputRecord.EventType) {
        case KEY_EVENT: // keyboard input
            consolekey = irInputRecord.Event.KeyEvent.wVirtualScanCode;
            if (!irInputRecord.Event.KeyEvent.bKeyDown)
                consolekey = -consolekey; // positive/negative return of scan codes.
            return 1;
        case MOUSE_EVENT: // mouse input
            consolemousex = irInputRecord.Event.MouseEvent.dwMousePosition.X + 1;
            consolemousey = irInputRecord.Event.MouseEvent.dwMousePosition.Y - cl_bufinfo.srWindow.Top + 1;
            consolebutton = irInputRecord.Event.MouseEvent.dwButtonState; // button state for all buttons
            // SetConsoleMode(hStdin, dwMode);
            return 2;
        }
    }
#endif
    return 0; // no or unhandled input
}

int32_t func__cinp(int32_t toggle, int32_t passed) {
#ifdef QB64_WINDOWS
    int32_t temp = consolekey;
    consolekey = 0; // reset the console key, now that we've read it
    if (passed == 0)
        toggle = 1; // default return of positive/negative scan code values
    if (toggle) {
        return temp;
    } else {
        if (temp >= 0)
            return temp;
        return -temp + 128;
    }
#else
    return 0;
#endif
}
