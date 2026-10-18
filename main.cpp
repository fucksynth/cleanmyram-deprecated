#include "cmr_core.h"
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <thread>

NOTIFYICONDATA nid;
HMENU h_popup_menu;
HMENU h_sub_menu;
std::thread cleaner_thread;

LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (msg == wm_trayicon) {
        if (lparam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);

            CheckMenuItem(h_sub_menu, id_menu_int_15s, MF_BYCOMMAND | (current_interval == delay_15s ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem(h_sub_menu, id_menu_int_60s, MF_BYCOMMAND | (current_interval == delay_60s ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem(h_sub_menu, id_menu_int_5m, MF_BYCOMMAND | (current_interval == delay_5m ? MF_CHECKED : MF_UNCHECKED));

            CheckMenuItem(h_popup_menu, id_menu_startup, MF_BYCOMMAND | (is_startup_enabled() ? MF_CHECKED : MF_UNCHECKED));

            SetForegroundWindow(hwnd);
            TrackPopupMenu(h_popup_menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
        }
        return 0;
    }

    if (msg == WM_COMMAND) {
        int wm_id = LOWORD(wparam);
        switch (wm_id) {
        case id_menu_clean_now: {
            int count = optimize_process_memory();
            TCHAR msg_buf[256];
            _stprintf_s(msg_buf, _countof(msg_buf), _T("successful <3\noptimized %d processes"), count);
            MessageBox(hwnd, msg_buf, _T("cleanmyram"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            break;
        }
        case id_menu_int_15s:
            current_interval = delay_15s;
            save_settings_to_reg();
            break;
        case id_menu_int_60s:
            current_interval = delay_60s;
            save_settings_to_reg();
            break;
        case id_menu_int_5m:
            current_interval = delay_5m;
            save_settings_to_reg();
            break;
        case id_menu_startup: {
            bool next_state = !is_startup_enabled();
            set_startup_state(next_state);
            break;
        }
        case id_menu_info: {
            TCHAR msg_buf[1024];
            _stprintf_s(msg_buf, _countof(msg_buf),
                _T("cleanmyram - ram auto cleaning tool, builded on SetProcessWorkingSetSize function to say your windows 'hey so this hungry processes eating too much resources, can u pls take some memory from them and reallocate it to where it is needed more'\n\nim strongly recommend to use 60 sec interval of auto memory cleaning, bcs if clean memory every 15 sec - windows will asking page file too much times p/min.\n\n\noriginally created for the fyall <3"));
            MessageBox(hwnd, msg_buf, _T("cleanmyram - made by @fucksynth"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            break;
        }
        case id_menu_exit:
            DestroyWindow(hwnd);
            break;
        }
        return 0;
    }

    if (msg == WM_DESTROY) {
        Shell_NotifyIcon(NIM_DELETE, &nid);
        is_running = false;
        if (cleaner_thread.joinable()) {
            cleaner_thread.join();
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(_In_ HINSTANCE h_inst, _In_opt_ HINSTANCE h_prev, _In_ LPSTR lp_cmd, _In_ int n_cmd) {
    load_settings_from_reg();

    const TCHAR* cmd_line_t = GetCommandLine();
    bool hide_welcome_window = (_tcsstr(cmd_line_t, _T("--startup")) != NULL);

    const TCHAR class_name[] = _T("cmr_cleaner_class");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = wndproc;
    wc.hInstance = h_inst;
    wc.lpszClassName = class_name;

    // fixed on 05.13.2026
    HICON h_embedded_icon = (HICON)LoadImage(h_inst, MAKEINTRESOURCE(idi_app_icon), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    wc.hIcon = h_embedded_icon ? h_embedded_icon : LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, class_name, _T("cleanmyram"), 0,
        0, 0, 0, 0, HWND_MESSAGE, NULL, h_inst, NULL);

    if (hwnd == NULL) {
        return 0;
    }

    h_sub_menu = CreatePopupMenu();
    AppendMenu(h_sub_menu, MF_STRING, id_menu_int_15s, _T("15 sec"));
    AppendMenu(h_sub_menu, MF_STRING, id_menu_int_60s, _T("60 sec (recommended)"));
    AppendMenu(h_sub_menu, MF_STRING, id_menu_int_5m, _T("5 min"));

    h_popup_menu = CreatePopupMenu();
    AppendMenu(h_popup_menu, MF_STRING, id_menu_clean_now, _T("clean ram right now"));
    AppendMenu(h_popup_menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(h_popup_menu, MF_POPUP, (UINT_PTR)h_sub_menu, _T("change intervals"));
    AppendMenu(h_popup_menu, MF_STRING, id_menu_startup, _T("launch at windows startup"));
    AppendMenu(h_popup_menu, MF_STRING, id_menu_info, _T("helpful info"));
    AppendMenu(h_popup_menu, MF_SEPARATOR, 0, NULL);
    AppendMenu(h_popup_menu, MF_STRING, id_menu_exit, _T("exit application"));

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = wm_trayicon;

    // fixed on 05.13.2026
    if (is_run_as_admin()) {
        HICON h_custom = (HICON)LoadImage(h_inst, MAKEINTRESOURCE(idi_app_icon), IMAGE_ICON, 16, 16, LR_SHARED);
        nid.hIcon = h_custom ? h_custom : LoadIcon(NULL, IDI_APPLICATION);
        _tcscpy_s(nid.szTip, _T("cleanmyram"));
    }
    else {
        HICON h_warn = (HICON)LoadImage(h_inst, MAKEINTRESOURCE(idi_warn_icon), IMAGE_ICON, 16, 16, LR_SHARED);
        nid.hIcon = h_warn ? h_warn : LoadIcon(NULL, IDI_WARNING);
        _tcscpy_s(nid.szTip, _T("cleanmyram [WARNING: run as admin to clean system processes!]"));
    }

    Shell_NotifyIcon(NIM_ADD, &nid);

    cleaner_thread = std::thread(run_cleaner_loop);

    if (!hide_welcome_window) {
        if (is_run_as_admin()) {
            MessageBox(hwnd, _T("cleanmyram is active and minimized into the tray area. use tray area to change settings"),
                _T("cleanmyram"), MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
        }
        else {
            MessageBox(hwnd, _T("tool started WITHOUT admin rights.\nsome protected background system memory pages cannot be optimized. please relaunch as administrator"),
                _T("cleanmyram "), MB_OK | MB_ICONWARNING | MB_TOPMOST);
        }
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyMenu(h_sub_menu);
    DestroyMenu(h_popup_menu);
    return (int)msg.wParam;
}
