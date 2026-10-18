#ifndef cmr_core_h
#define cmr_core_h

#include <windows.h>
#include <tchar.h>
#include <atomic>

// custom message for tray interactions
#define wm_trayicon (WM_USER + 1)

// custom menu identifier numbers
#define id_menu_clean_now 1001
#define id_menu_int_15s   1002
#define id_menu_int_60s   1003
#define id_menu_int_5m    1004
#define id_menu_exit      1005
#define id_menu_info      1006
#define id_menu_startup   1007

// custom resource identifiers for embedded icons
#define idi_app_icon      2001 
#define idi_warn_icon     2002 

// timer delay values in milliseconds
#define delay_15s 15000
#define delay_60s 60000
#define delay_5m  300000

// internal structural configurations
extern std::atomic<int> current_interval;
extern bool is_running;

// basic framework logic functions
int optimize_process_memory();
void run_cleaner_loop();

// registry management functions
bool is_startup_enabled();
void set_startup_state(bool enable);
void save_settings_to_reg();
void load_settings_from_reg();

// security and privilege helper functions
bool is_run_as_admin();

#endif
