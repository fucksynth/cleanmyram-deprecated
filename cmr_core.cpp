#include "cmr_core.h"
#include <tlhelp32.h>
#include <thread>
#include <chrono>

std::atomic<int> current_interval = delay_60s;
bool is_running = true;

const TCHAR* run_key_path = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
const TCHAR* app_key_path = _T("Software\\cleanmyram");
const TCHAR* app_name_reg = _T("cleanmyram");

int optimize_process_memory() {
    int optimized_count = 0;
    HANDLE h_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (h_snapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(h_snapshot, &pe)) {
        do {
            if (pe.th32ProcessID == 0) continue;
            HANDLE h_proc = OpenProcess(PROCESS_SET_QUOTA, FALSE, pe.th32ProcessID);
            if (h_proc != NULL) {
                if (SetProcessWorkingSetSize(h_proc, (SIZE_T)-1, (SIZE_T)-1)) {
                    optimized_count++;
                }
                CloseHandle(h_proc);
            }
        } while (Process32Next(h_snapshot, &pe));
    }
    CloseHandle(h_snapshot);
    return optimized_count;
}

void run_cleaner_loop() {
    int last_check_time = 0;
    while (is_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!is_running) break;

        last_check_time += 100;
        if (last_check_time >= current_interval) {
            optimize_process_memory();
            last_check_time = 0;
        }
    }
}

bool is_startup_enabled() {
    HKEY h_key;
    bool enabled = false;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, run_key_path, 0, KEY_READ, &h_key) == ERROR_SUCCESS) {
        if (RegQueryValueEx(h_key, app_name_reg, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            enabled = true;
        }
        RegCloseKey(h_key);
    }
    return enabled;
}

void set_startup_state(bool enable) {
    HKEY h_key;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, run_key_path, 0, KEY_WRITE, &h_key) == ERROR_SUCCESS) {
        if (enable) {
            TCHAR path[MAX_PATH];
            GetModuleFileName(NULL, path, MAX_PATH);

            TCHAR command_line[MAX_PATH + 16];
            _stprintf_s(command_line, _countof(command_line), _T("\"%s\" --startup"), path);

            RegSetValueEx(h_key, app_name_reg, 0, REG_SZ, (BYTE*)command_line, (DWORD)(_tcslen(command_line) + 1) * sizeof(TCHAR));
        }
        else {
            RegDeleteValue(h_key, app_name_reg);
        }
        RegCloseKey(h_key);
    }
}

void save_settings_to_reg() {
    HKEY h_key;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, app_key_path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &h_key, NULL) == ERROR_SUCCESS) {
        int interval_val = current_interval;
        RegSetValueEx(h_key, _T("interval"), 0, REG_DWORD, (BYTE*)&interval_val, sizeof(DWORD));
        RegCloseKey(h_key);
    }
}

void load_settings_from_reg() {
    HKEY h_key;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, app_key_path, 0, KEY_READ, &h_key) == ERROR_SUCCESS) {
        DWORD interval_val = delay_60s;
        DWORD size = sizeof(DWORD);
        if (RegQueryValueEx(h_key, _T("interval"), NULL, NULL, (BYTE*)&interval_val, &size) == ERROR_SUCCESS) {
            current_interval = (int)interval_val;
        }
        RegCloseKey(h_key);
    }
}

// checks if the current process tkn has elevated admin privileges
bool is_run_as_admin() {
    BOOL is_admin = FALSE;
    PSID administrators_group = NULL;
    SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administrators_group)) {
        CheckTokenMembership(NULL, administrators_group, &is_admin);
        FreeSid(administrators_group);
    }
    return is_admin == TRUE;
}
