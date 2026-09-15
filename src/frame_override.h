#ifndef NODUS_FRAME_OVERRIDE_H
#define NODUS_FRAME_OVERRIDE_H

/*
 * Nodus custom window frame
 * -------------------------
 * Forced into the build before src/main.c.
 * It keeps the existing native C dashboard intact while replacing the
 * standard Windows caption/title bar with the dark Nodus header.
 */

#include <windows.h>
#include <string.h>

static WNDPROC g_nodus_original_proc = NULL;

static int nodus_tab_width(int index) {
    if (index == 0) return 94;
    if (index == 4) return 106;
    return 98;
}

static int nodus_hit_tab(int x, int y) {
    int i;
    int tx = 246;
    if (y < 24 || y > 78) return 0;

    for (i = 0; i < 6; i++) {
        int tw = nodus_tab_width(i);
        if (x >= tx - 8 && x <= tx + tw && y >= 24 && y <= 78) {
            return 1;
        }
        tx += tw + 9;
    }
    return 0;
}

static int nodus_hit_search(int x, int y, int w) {
    int sx = w - 430;
    return x >= sx && x <= sx + 205 && y >= 23 && y <= 61;
}

static int nodus_hit_control(int x, int y, int w) {
    if (y < 9 || y > 68) return 0;
    if (x >= w - 34) return 3;
    if (x >= w - 68) return 2;
    if (x >= w - 102) return 1;
    return 0;
}

static void nodus_draw_text(HDC dc, HFONT font, COLORREF color,
                            int x, int y, const char *text) {
    HFONT old = (HFONT)SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    TextOutA(dc, x, y, text, (int)strlen(text));
    SelectObject(dc, old);
}

static void nodus_draw_overlay(HWND hwnd) {
    RECT cr;
    RECT r;
    HDC dc;
    HBRUSH bg;
    HBRUSH lineBrush;
    HFONT fontSmall;
    HFONT fontTiny;
    SYSTEMTIME st;
    char buf[64];
    int w;

    if (!GetClientRect(hwnd, &cr)) return;
    w = cr.right;
    if (w < 800) return;

    dc = GetDC(hwnd);
    if (!dc) return;

    r.left = w - 452;
    r.top = 7;
    r.right = w - 2;
    r.bottom = 88;
    bg = CreateSolidBrush(RGB(12,10,9));
    FillRect(dc, &r, bg);
    DeleteObject(bg);

    fontSmall = CreateFontA(
        -13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas"
    );
    fontTiny = CreateFontA(
        -11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas"
    );

    r.left = w - 430;
    r.top = 24;
    r.right = w - 225;
    r.bottom = 59;
    lineBrush = CreateSolidBrush(RGB(82,66,49));
    FrameRect(dc, &r, lineBrush);
    DeleteObject(lineBrush);
    nodus_draw_text(dc, fontTiny, RGB(119,105,91), w - 414, 36,
                    "Search agents, tasks...");

    GetLocalTime(&st);
    wsprintfA(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
    nodus_draw_text(dc, fontSmall, RGB(229,216,194), w - 210, 26, buf);
    wsprintfA(buf, "%02d/%02d/%04d", st.wDay, st.wMonth, st.wYear);
    nodus_draw_text(dc, fontTiny, RGB(126,116,104), w - 210, 46, buf);

    nodus_draw_text(dc, fontSmall, RGB(136,126,114), w - 91, 31, "_");
    nodus_draw_text(dc, fontTiny, RGB(136,126,114), w - 58, 33, "[]");
    nodus_draw_text(dc, fontSmall, RGB(232,221,202), w - 25, 31, "X");

    DeleteObject(fontSmall);
    DeleteObject(fontTiny);
    ReleaseDC(hwnd, dc);
}

static LRESULT CALLBACK NodusFrameProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_NCHITTEST: {
            POINT pt;
            RECT cr;
            int left, right, top, bottom;
            const int grip = 7;

            pt.x = (short)LOWORD(lp);
            pt.y = (short)HIWORD(lp);
            ScreenToClient(hwnd, &pt);
            GetClientRect(hwnd, &cr);

            left = pt.x < grip;
            right = pt.x >= cr.right - grip;
            top = pt.y < grip;
            bottom = pt.y >= cr.bottom - grip;

            if (top && left) return HTTOPLEFT;
            if (top && right) return HTTOPRIGHT;
            if (bottom && left) return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;
            if (left) return HTLEFT;
            if (right) return HTRIGHT;
            if (top) return HTTOP;
            if (bottom) return HTBOTTOM;

            if (nodus_hit_tab(pt.x, pt.y)) return HTCLIENT;
            if (nodus_hit_search(pt.x, pt.y, cr.right)) return HTCLIENT;
            if (nodus_hit_control(pt.x, pt.y, cr.right)) return HTCLIENT;

            if (pt.y >= 0 && pt.y < 96) return HTCAPTION;
            return HTCLIENT;
        }

        case WM_LBUTTONDOWN: {
            RECT cr;
            int x = (short)LOWORD(lp);
            int y = (short)HIWORD(lp);
            int control;

            GetClientRect(hwnd, &cr);
            control = nodus_hit_control(x, y, cr.right);

            if (control == 3) {
                PostMessageA(hwnd, WM_CLOSE, 0, 0);
                return 0;
            }
            if (control == 2) {
                ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
                return 0;
            }
            if (control == 1) {
                ShowWindow(hwnd, SW_MINIMIZE);
                return 0;
            }
            break;
        }

        case WM_PAINT: {
            LRESULT result = CallWindowProcA(g_nodus_original_proc, hwnd, msg, wp, lp);
            nodus_draw_overlay(hwnd);
            return result;
        }

        case WM_TIMER: {
            LRESULT result = CallWindowProcA(g_nodus_original_proc, hwnd, msg, wp, lp);
            nodus_draw_overlay(hwnd);
            return result;
        }
    }

    return CallWindowProcA(g_nodus_original_proc, hwnd, msg, wp, lp);
}

static HWND WINAPI NodusCreateWindowExA(
    DWORD dwExStyle,
    LPCSTR lpClassName,
    LPCSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam
) {
    HWND hwnd;
    DWORD style = dwStyle;

    if (lpClassName && lstrcmpA(lpClassName, "NodusNativeWindow") == 0) {
        style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX |
                WS_MAXIMIZEBOX | WS_SYSMENU;
    }

    hwnd = CreateWindowExA(
        dwExStyle, lpClassName, lpWindowName, style,
        X, Y, nWidth, nHeight,
        hWndParent, hMenu, hInstance, lpParam
    );

    if (hwnd && lpClassName &&
        lstrcmpA(lpClassName, "NodusNativeWindow") == 0) {
        g_nodus_original_proc = (WNDPROC)SetWindowLongPtrA(
            hwnd, GWLP_WNDPROC, (LONG_PTR)NodusFrameProc
        );
    }

    return hwnd;
}

#define CreateWindowExA NodusCreateWindowExA

#endif
