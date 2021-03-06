/*
** $Id: helloworld.c 793 2010-07-28 03:36:29Z dongjunjie $
**
** Listing 1.1
**
** helloworld.c: Sample program for MiniGUI Programming Guide
**      The first MiniGUI application.
**
** Copyright (C) 2004 ~ 2009 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <string.h>

#include <common.h>
#include <minigui.h>
#include <gdi.h>
#include <window.h>

#ifdef _LANG_ZHCN
#include "helloworld_res_cn.h"
#elif defined _LANG_ZHTW
#include "helloworld_res_tw.h"
#else
#include "helloworld_res_en.h"
#endif

static char welcome_text [512];
static char msg_text [256];
static RECT welcome_rc = {10, 100, 600, 400};
static RECT msg_rc = {10, 100, 600, 400};

static const char* syskey = "";

static int last_key = -1;
static int last_key_count = 0;

static void make_welcome_text (void)
{
    const char* sys_charset = GetSysCharset (TRUE);
    const char* format;

    if (sys_charset == NULL)
        sys_charset = GetSysCharset (FALSE);

    SetRect (&welcome_rc,  10, 10, g_rcScr.right - 10, g_rcScr.bottom / 2 - 10);
    SetRect (&msg_rc, 10, welcome_rc.bottom + 10, g_rcScr.right - 10, g_rcScr.bottom - 20);

    if (strcmp (sys_charset, FONT_CHARSET_GB2312_0) == 0
            || strcmp (sys_charset, FONT_CHARSET_GBK) == 0
            || strcmp (sys_charset, FONT_CHARSET_GB18030_0) == 0) {
        format = "欢迎来到 MiniGUI 的世界! 如果您能看到该文本, 则说明 MiniGUI Version %d.%d.%d 可在该硬件上运行!";
    }
    else if (strcmp (sys_charset, FONT_CHARSET_BIG5) == 0) {
        format = "欢迎来到 MiniGUI 的世界! 如果您能看到该文本, 则说明 MiniGUI Version %d.%d.%d 可在该硬件上运行!";
    }
    else {
        format = "Welcome to the world of ingenic MiniGUI. \n";
    }

   // sprintf (welcome_text, format, MINIGUI_MAJOR_VERSION, MINIGUI_MINOR_VERSION, MINIGUI_MICRO_VERSION);
    strcat(welcome_text,format);
    strcpy (msg_text, HL_ST_NOMES);
}

static int HelloWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    BITMAP bmp;

    syskey = "";

    switch (message) {
        case MSG_CREATE:
            make_welcome_text ();
          //  SetTimer (hWnd, 100, 200);
            break;

//        case MSG_TIMER:
//            sprintf (msg_text, HL_ST_TIMER,
//                            GetTickCount ());
//            InvalidateRect (hWnd, &msg_rc, TRUE);
//            break;
//
        case MSG_LBUTTONDOWN:
            strcpy (msg_text, HL_ST_LBD);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_LBUTTONUP:
            strcpy (msg_text, HL_ST_LBU);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_RBUTTONDOWN:
            strcpy (msg_text, HL_ST_RBD);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_RBUTTONUP:
            strcpy (msg_text, HL_ST_RBU);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_PAINT:
	    LoadBitmap(HDC_SCREEN, &bmp, "./ingenic.bmp");
	    hdc = BeginPaint (hWnd);
	    FillBoxWithBitmap(hdc, 60, 60, 120, 120, &bmp);
            DrawText (hdc, welcome_text, -1, &welcome_rc, DT_LEFT | DT_WORDBREAK);
            //DrawText (hdc, msg_text, -1, &msg_rc, DT_LEFT | DT_WORDBREAK);
            EndPaint (hWnd, hdc);
            return 0;

        case MSG_SYSKEYDOWN:
            syskey = HL_ST_SYS;
        case MSG_KEYDOWN:
            if(last_key == wParam)
                last_key_count++;
            else
            {
                last_key = wParam;
                last_key_count = 1;
            }
            sprintf (msg_text, HL_ST_KEYD,
                            wParam, syskey, last_key_count);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            return 0;

        case MSG_KEYLONGPRESS:
            sprintf (msg_text, HL_ST_KEYLONG, wParam);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_KEYALWAYSPRESS:
            sprintf (msg_text, HL_ST_KEYALWAY, wParam);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            break;

        case MSG_KEYUP:
            sprintf (msg_text, HL_ST_KEYU, wParam);
            InvalidateRect (hWnd, &msg_rc, TRUE);
            return 0;

        case MSG_CLOSE:
	    UnloadBitmap (&bmp);
	    KillTimer (hWnd, 100);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;
    HWND hMainWnd;
    MAINWINCREATE CreateInfo;
    HDC hdc;
    char buf=0;
#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "helloworld" , 0 , 0);
#endif

    CreateInfo.dwStyle =
        WS_VISIBLE | WS_BORDER | WS_CAPTION;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = HL_ST_CAP;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = HelloWinProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;
    CreateInfo.iBkColor = COLOR_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;

    hMainWnd = CreateMainWindow (&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return -1;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);
    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    MainWindowThreadCleanup (hMainWnd);

    return 0;
}

#ifdef _MGRM_THREADS
//#include <dti.c>
#endif

