This is a my modified version of hawku's tablet driver. I've added some smoothing algorithms.
For more detailed info hold mouse over antichatter settings.
## Download
### https://github.com/Devocub/TabletDriver/releases
### Installing
1) If you have already Driver installed just: close it from tray, and replace files.  

If you have not installed the driver then: install hawku's 0.1.3 version (version doesn't matter) (https://github.com/hawku/TabletDriver/releases) and go to step 1. My version based on 0.1.0 version of driver.

_____

Antichatter feature is meant to prevent cursor chattering/rattling/shaking/trembling when it's almost doesn't moves and/or too high to prevent tablet noise.
Antichatter in it's primary form is useful for tablets which doesn't have any smoothing (including hardware smoothing).
Antichatter requires smoothing filter enabled for work. Latency do affect on antichatter settings.

**Type**: few different types of smoothing and off. Explanation will below.

**Range**: maximum distance after which antichatter eliminates. Motionless pen provides different range of noise based on height over the tablet. cm is a lie, Useful values are roughly from 0.05-0.15 up to 4-10, negative values are useless.

**Strength**: is strength, useful values are from 2-3 up to 1000000. Can be negative.

**Offset**: offset value for Type 2, see source code for formulas. Can be negative.


**Type Off** disables antichatter at all, only smoothing filter left working.
                         
**Type 1** is dumb and simple: if cursor speed is slow (based on Range) then smooth it in Strength times stronger. Offset value doesn't affect on this.
  
**Type 2** is smarter - it's distance based, see code for formulas. 
          
Recommended values for primary idea of antichatter is: Latency 10 ms, Type 1 (or 2), Range 0.15 cm, Strength 20
Some interesting (fun) settings: Latency 40 ms, Type 1, Range 3, Strength 100 | Latency 17 ms, Type 2, Range 10, Strength 100000, Offset -2

![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/6.png)
![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/1.png)
![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/2.png)
![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/3.png)
![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/4.png)
![Alt text](https://raw.githubusercontent.com/Devocub/TabletDriver/master/images/5.png)



# TabletDriver

This is a low latency graphics tablet driver that is meant to be used with rhythm game [osu!](https://osu.ppy.sh/home)

Currently the driver only works when the TabletDriverGUI is running.

The GUI minimizes to system tray / notification area. You can reopen the GUI by double clicking the system tray icon.

**If you have problems with the driver, please read the FAQ:**

**https://github.com/hawku/TabletDriver/wiki/FAQ**

## Download

### http://hwk.fi/TabletDriver/TabletDriverV0.1.0.zip

#

### Supported operating systems:
  - Windows 7 64-bit
  - Windows 8 64-bit
  - Windows 8.1 64-bit
  - Windows 10 64-bit

#

### Supported tablets:
  - Wacom CTL-470
  - Wacom CTL-471
  - Wacom CTL-472
  - Wacom CTL-480
  - Wacom CTH-480
  - Wacom CTL-490
  - XP Pen G430
  - XP Pen G640
  - Huion 420
  - Huion H640P
  - Gaomon S56K
  
### Configured, but not properly tested:
  - Huion H420
  - Wacom CTH-470
  - Wacom CTH-670
  - Wacom CTL-671
  - Wacom CTL-672
  - Wacom CTL-680
  - Wacom CTH-680
  - Wacom CTH-490
  - Wacom PTH-451

#

## Installation

1. You might need to install these libraries, but usually these are already installed:
    * https://www.microsoft.com/net/download/dotnet-framework-runtime
    * https://aka.ms/vs/15/release/vc_redist.x86.exe

2. Unzip the driver to a folder (Shorter path is recommended, for example `C:\Temp\TabletDriver`)
3. Uninstall all other tablet drivers.
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
- https://www.xp-pen.com/upload/download/20180130/XP-PenWin(20180130).zip

**Huion WinUSB driver:**
- https://www.huiontablet.com/drivers/WinDriver/HuionTablet_WinDriver_v13.15.16.zip

#

## Changelog

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
>   Gaomon S56K, XP Pen G430 and G640.
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
