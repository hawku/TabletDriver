# TabletDriver

This is a low latency graphics tablet driver that is meant to be used with rhythm game [osu!](https://osu.ppy.sh/home)

Currently the driver only works when the TabletDriverGUI is running.

The GUI minimizes to system tray / notification area. You can reopen the GUI by double clicking the system tray icon.

**If you have problems with the driver, please read the FAQ:**

**https://github.com/hawku/TabletDriver/wiki/FAQ**

## Download

### http://hwk.fi/TabletDriver/TabletDriverV0.2.3.zip

#

### Supported operating systems:
  - Windows 10 64-bit
  - Windows 8.1 64-bit
  - Windows 8 64-bit
  - Windows 7 64-bit (Multiple monitors do not work in absolute mode)

#

### Supported tablets:
  - Wacom CTE-440
  - Wacom CTL-470
  - Wacom CTH-470
  - Wacom CTL-471
  - Wacom CTL-472
  - Wacom CTL-480
  - Wacom CTH-480
  - Wacom CTL-490
  - Wacom CTH-490
  - Wacom CTL-4100 USB
  - Wacom CTL-4100 Bluetooth
  - Wacom CTL-671
  - Wacom CTL-672
  - Wacom CTL-680
  - Wacom CTH-680
  - Wacom CTL-690
  - Wacom CTH-690
  - Wacom PTH-660
  - Wacom PTH-451
  - Wacom PTH-850
  - XP-Pen G430 (New 2017+ "Model B". Old G430 is not supported)
  - XP-Pen G430S
  - XP-Pen G540 Pro (G540 is not supported)
  - XP-Pen G640
  - XP-Pen G640S
  - XP-Pen Deco 01
  - XP-Pen Deco 01 v2
  - Huion 420
  - Huion H420
  - Huion H430P
  - Huion H640P
  - Huion osu!tablet
  - Gaomon S56K
  - VEIKK S640
  
### Configured, but not properly tested:
> - Huion New 1060 Plus
> - Huion Inspiroy Q11K
  - Other Wacom tablets that might work: https://github.com/hawku/TabletDriver/blob/master/TabletDriverService/config/wacom.cfg

#

## Installation

1. You might need to install these libraries, but usually these are already installed:
    * https://www.microsoft.com/net/download/dotnet-framework-runtime
    * https://aka.ms/vs/15/release/vc_redist.x86.exe

2. Unzip the driver to a folder (Shorter path is recommended, for example `C:\Temp\TabletDriver`)
3. Uninstall all other tablet drivers. If you have problems with uninstalling the Wacom drivers, check the GitHub issue [#1](https://github.com/hawku/TabletDriver/issues/1)
4. Run `install_vmulti_driver.bat`. It might need a restart if there is another vmulti driver installed.
5. If you have Huion or Gaomon tablet, you need to run `install_huion_64.bat`, which is in the `driver_huion` directory.
6. Start the TabletDriverGUI.exe

## Updating to a new version

1. Unzip the new version
2. Start the TabletDriverGUI.exe

## Uninstallation

1. Uncheck the "Run at Windows startup" option in the GUI.
2. Run `remove_vmulti_driver.bat`
3. Run `remove_huion_64.bat`, which is in the `driver_huion` directory.

#

## VMulti and Huion driver binaries

If you want to compile the code and don't want to install anything from the TabletDriver binary package, you will need extract the missing drivers from these installation packages:

**VMulti driver:**
- https://www.xp-pen.com/upload/download/20181019/osuWin(20181019).zip

**Huion WinUSB driver:**
- https://www.huiontablet.com/drivers/WinDriver/HuionTablet_WinDriver_v14.7.60.zip

#

## Changelog

>**v0.2.3:**
> - Fixed "TabletDriverService.exe has stopped working" error when a tablet is not connected.
> - Relative mouse mode can now have different sensitivity on X and Y axis.
> - Added SendInput output mode to GUI

>**v0.2.2:**
> - XP-Pen G430S configuration by [frodriguez96](https://github.com/frodriguez96) and [riley-badour](https://github.com/riley-badour)
> - Huion New 1060 Plus configuration by [riley-badour](https://github.com/riley-badour)
> - Huion Inspiroy Q11K configuration by [octoberU](https://github.com/octoberU)
> - Wacom CTL-690 and CTH-690 configurations moved to tablet.cfg and DetectMask fixed.
> - Few more Wacom tablets can now be used while the official drivers are installed (check tablet.cfg)
> - Tablet button support for Wacom CTH-480, CTL-490, CTL-4100, Huion H430P and H640P.
> - Custom tablet data format configuration (check tablet.cfg for examples)
> - Pen and tablet button mapping to a mouse buttons and keyboard keys.
> - Pen button mapping to scroll.
> - Windows Ink pressure settings and test canvas.
> - Smoothing filter now also smooths out pen pressure.
> - Smoothing can be set to only smooth out when pen buttons are down.
> - Anti-smoothing compensation value is now in milliseconds (check tooltip for example values)
> - Ability start the TabletDriverService.exe without GUI (`tools\RunServiceOnly.bat`)

>**v0.2.1:**
> - XP-Pen G640S and VEIKK S640 configurations by [frodriguez96](https://github.com/frodriguez96)
> - Original XP-Pen, Huion and VEIKK driver processes will now be killed when the TabletDriverGUI starts.
> - Fixed cursor jumping to the corner when driver restarts or tablet disconnects.
> - Removed device list from driver startup. Might help with the problems that the driver restart is causing other USB devices to glitch out.
> - Relative mode position reset is now decided by when the last tablet movement was received (default 100 ms, `RelativeResetTime` command).
> - Added ability to disable automatic restart.
> - Added command name autocomplete to the console input (tab) and commands tab (control + space).
> - Added gravity filter (`GravityFilter` command). It's just for fun and testing. Works only when pen buttons are pressed.
> - Added 32-bit VMulti and Huion drivers to the driver zip file.

>**v0.2:**
> - **Improved the noise reduction filter and added the settings to the GUI.**
> - **Added an anti-smoothing filter which reduces the input latency on tablets that use hardware smoothing.**
> - Wacom CTE-440 support by [Poliwrath](https://github.com/Poliwrath)
> - Wacom PTH-660 support by [Implojin](https://github.com/Implojin)
> - XP-Pen Deco 01 v2 support by [Itsyuka](https://github.com/Itsyuka)
> - Modified Huion 420 and H420 tablet area size to match with the 2000 LPI resolution.
> - Updated Huion drivers to the latest version.
> - Moved the filter settings to filters tab in the GUI.
> - Added `Measure` command. Measures distances between clicked points.
> - Added an ability to "draw" tablet area by clicking two points with the pen.
> - Added "Restart driver" option to the notification icon menu.
> - Added 1000 Hz smoothing filter rate option to the GUI.
> - Added few tablets to wacom.cfg
> - Major code refactoring.

>**v0.1.5:**
> - New tablet configurations: Wacom CTL-4100 (USB only model), XP-Pen G540 Pro, XP-Pen Deco 01 and Huion osu!tablet
>   Thanks to /u/THEqrunt for capturing the XP-Pen Deco 01 USB data.
> - Added `ResetDistance` command, it controls the relative mode position reset distance.
> - Code refactoring.

>**v0.1.4:**
> - Modified the Wacom CTL-471 full area size (147.20 x 92.25 mm to 152 x 95 mm)
> - New tablet configurations: Wacom PTH-850 and Huion H430P
>   The PTH-850 configuration is made by [mojobojo](https://github.com/mojobojo)
> - Regenerated the wacom.cfg with new parameters, so it now includes PTH-450/650/850 and PTK-450/650

>**v0.1.3:**
> - Added left handed mode / tablet invert option.
> - Added Wacom driver device support for the CTL-471 and 472
> - Noise reduction filter improvement (`Noise` command)

>**v0.1.2:**
> - Added experimental support for leaving the Wacom drivers installed on the system.
>   Supported tablets: CTL-470, CTL-480, CTH-480, CTL-4100
> - Added `disable_wacom_drivers.bat` and `enable_wacom_drivers.bat` to the `tools` folder.
>   These scripts are used to disable and enable Wacom drivers when using the experimental Wacom driver support.
> - Added driver restart button.

>**v0.1.1:**
> - Added support for Wacom CTL-4100 (USB and Bluetooth)
> - Added settings import / export to the main menu.
> - Added Wacom backup reader to the Wacom area tool.
> - Added tablet benchmark tools to the console output context menu (Right click).
> - Moved the `config.xml` to the `config` folder.
> - Added noise reduction filter (`Noise` command, not in the GUI)
> - Code refactoring

>**v0.1.0:**
> - Added `Bench` / `Benchmark` command.
> - Added `-hide` GUI command line parameter. GUI will start as minimized when you run `TabletDriverGUI.exe -hide` 
> - Added an option to run the TabletDriverGUI at Windows startup.

>**v0.0.18:**
> - Added TabletDriverService.exe multi-instance prevention.
> - Added yet another Wacom 490 tip click fix.
>   `KeepTipDown` command sets how long the pen tip button output should be kept down after the pen tip is released.

>**v0.0.17:**
> - Fixed driver crashing when used with the Huion or Gaomon tablets.

>**v0.0.16:**
> - Added smoothing filter rate selector. Use a lower rate if you have filter performance problems.
> - Added TabletDriverService.exe process priority warning when the priority isn't set to High.
> - Desktop size settings are now available to everyone.
>   Previously shown only when the developer mode was enabled.
>   Automatic size should be used, but if you have problems with the screen mapping:
>   https://github.com/hawku/TabletDriver/issues/4
> - First few tablet position packets are now ignored to prevent the cursor jumping to a wrong position when driver is started.

>**v0.0.15:**
> - Added more debug information to startuplog.txt. It now includes a list of connected input devices.
> - Added debug tools to the console output context menu (Right click).
> - Removed ClickPressure workaround from Wacom 490.
>   You can re-enable that by adding `ClickPressure 500` to Commands tab.
> - Updated the wacom.cfg

>**v0.0.14:**
> - Fix for the console "Copy all" function.

>**v0.0.13:**
> - Added a direction indicator to the tablet area and moved the aspect ratio text to the middle of the area.
> - TabletDriverService process priority class is now set to "High" (suggestion by /u/Devocub)
> - TabletDriverService stability and error handling improvements (thanks to https://github.com/freakode)
> - New tablet configurations: Huion H420 (made by /u/uyghti)
> - Automatically generated Wacom tablet configurations (config\wacom.cfg)

>**v0.0.12:**
> - Added multi-instance prevention. Old TabletDriverGUI.exe should pop up if you try to open another one.
> - New tablet configurations: Wacom CTL-680 and CTH-680

>**v0.0.11:**
> - Fix for DPI scaling problems. Screen mapping were wrong when the monitor DPI scaling wasn't 100%
> - Added a Wacom area tool. It should work with Wacom Intuos and Bamboo tablets (470->490)
> - Added startup debug log

>**v0.0.10:**
> - New tablet configurations: Wacom CTH-470, CTH-670, PTH-451
> - Fix for the smoothing filter. The filter didn't turn on when the settings were applied.
> - Fix for the Huion H640P clicking problem and also added better data validation for Huion 420,
>   Gaomon S56K, XP-Pen G430 and G640.
> - Modified click detection on CTL-490 and CTH-490 (tablet.cfg ClickPressure).

>**v0.0.9:**
> - Yet another fix for the clicking problem... Maybe this time it will work?

>**v0.0.8:**
> - Another fix for pen tip clicking. Improved the tablet data validation.

>**v0.0.7:**
> - Added aspect ratio text to screen and tablet area.
> - Workaround for pen tip click detection. Some tablets don't send correct button data, so the pen tip click is now detected from the pressure data

>**v0.0.6:**
> - Improved smoothing filter latency calculation

>**v0.0.5:**
> - Added Windows Ink mode with pressure sensitivity
> - Added relative mouse mode
> - Added tablet area rotation
> - Added optional smoothing filter

>**v0.0.4:**
> - Fixed a number conversion bug in the tablet area detection.
