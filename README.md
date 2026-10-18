# cleanmyram

tiny ram auto cleaning tool for windows. lives in ur tray area and quietly asks windows to stop being so greedy with memory.

basically its just a small wrapper around the `SetProcessWorkingSetSize` function to say to ur windows "hey so this hungry processes eating too much resources, can u pls take some memory from them and reallocate it to where it is needed more". no fancy ui, no bloat, no telemetry. one exe and thats all

originally created for the fyall <3

## what it actually does

every N seconds the app walks through all running processes with toolhelp snapshot and trims their working sets. windows then pushes cold pages from ram into the page file and u get some free memory back. thats the whole magic, nothing more complicated.

## features

- tray icon only, no taskbar window, app starts minimized and silent
- auto clean on timer: **15 sec**, **60 sec (recommended)** or **5 min**
- "clean ram right now" button if u dont wanna wait
- "launch at windows startup" checkbox right in the tray menu
- interval and startup flag are saved in registry, so settings survive reboot
- warns u with a special icon in tray if u running it without admin rights

## about admin rights

tool runs fine without elevation, but protected system processes simply won't open and their memory pages cannot be optimized. so if u want the full effect - relaunch as administrator, the tray icon will show u a little warning about it.

## about intervals

im strongly recommend to use 60 sec interval of auto memory cleaning, bcs if clean memory every 15 sec - windows will asking page file too much times p/min. 5 min is fine too if u just want some background maintenance without touching anything.

## building

just a regular visual studio c++ project, no external dependencies at all, pure winapi.

1. open `CleanMyRam.slnx` in visual studio (2022 or newer, toolset v145)
2. pick `Release` / `x64`
3. build, get ur `CleanMyRam.exe`

project is compiled with c++20 and unicode charset. everything else is already configured in the vcxproj, so nothing to tweak manually.

## project structure

```
main.cpp       - winmain, tray icon, popup menu, message loop
cmr_core.h     - identifiers, delays, function declarations
cmr_core.cpp   - cleaner loop, registry settings, startup, admin check
resources.rc   - binds icon files to resource ids
icon.ico       - normal app icon
warn.ico       - shown when running without admin rights
```

## settings in registry

u dont have to touch it manually, menu does all of it, but just for reference:

- `HKCU\Software\cleanmyram\interval` - auto clean interval in milliseconds
- `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\cleanmyram` - startup entry with `--startup` flag

`--startup` flag is how the app knows it was launched by windows on boot and skips the welcome message box.

## menu

- `clean ram right now` - immediate cleaning, shows how many processes were optimized
- `change intervals` - 15 sec / 60 sec / 5 min
- `launch at windows startup` - toggle autostart
- `helpful info` - short description right inside the app
- `exit application` - quit

## disclaimer

trimming working sets is safe, windows just re-reads cold pages from page file when it needs them again. but still, its a tweak, not a miracle. if u have tons of ram maybe u dont need this at all lol

made by @fucksynth <3
