# Serial Port Plotter | Thermo plotter |

Originally was an application that displays real time data from serial port.
Has been modified heavily, to act as digital control software for arduino-based reflow station.
(In the archive - software for Windows + Arduino sketch + assembly/installation/setup information)

#### Suitable for both building from scratch or modifying existing commercial equipment.

Component selection is focused on parts accessibility and ease of assembly.
Arduino Mega2560 + 3x Max6675 + 3 K-type thermocouples + 2 SSRs, + optional ILI9486 display & buttons

[DOWNLOAD version 0.63 for Win x64  (7-8-10)](0.63.zip)

[GUIDE about usage\assembly\tips](Manual%20v0.1.docx)

origin Forum topic about self-made soldering stations: [at Amperka Forum](https://forum.amperka.ru/threads/%D0%98%D0%BA-%D0%BF%D0%B0%D1%8F%D0%BB%D1%8C%D0%BD%D0%B0%D1%8F-%D1%81%D1%82%D0%B0%D0%BD%D1%86%D0%B8%D1%8F-%D0%BD%D0%B0-arduino-mega-2560-%D0%B4%D0%BE%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%BA%D0%B0-%D1%81%D0%BA%D0%B5%D1%82%D1%87%D0%B0-ars_v2_lilium_jsn-%D1%87-2.21236/#post-269853)

## 2 control modes:
 - from PC via this application
 - standalone with display & buttons

## Features
 - automatic Soldering following thermal profile
 - acting as lower heater (PCB preheater)
 - editing and saving profiles in station memory and in a file (PC application)
 - displaying soldering graph (PC application)

## Interface Screenshot

![Serial Port Plotter screenshot](res/interface.png)

## Settings Screenshot

![Serial Port Plotter screenshot](res/settings.png)

## How to use the application

Assemble everything according to schematics. 
Upload sketch to Arduino.
Made some tweaks to soldering profiles.


Double click on a channel in the Graph Control panel to hide/show a specific channel.

![File Save Button](res/screen_1.png)

## Send data over the serial port

```c
/* Example: Plot two values */
printf ("$%d %d;", data1, data2);
```

Depending on how much data you want to display, you can adjust the number of data points. For example, if you send data from the serial port of the mbed every 10 ms (100 Hz) and the plotter is set to display 500 points, it will contain information for 5 seconds of data.

The software supports integer and decimal numbers ( float/double )

## Source

Source and .pro file of the Qt Project are available. A standalone .exe is included for the people who do not want to build the source. Search for it at [releases](https://github.com/CieNTi/serial_port_plotter/releases)

## Credits

- [Serial Port Plotter at mbed forums](https://developer.mbed.org/users/borislav/notebook/serial-port-plotter/) by [Borislav K](https://developer.mbed.org/users/borislav/)
- [Line Icon Set](http://www.flaticon.com/packs/line-icon-set) by [Situ Herrera](http://www.flaticon.com/authors/situ-herrera) icon pack
- [Lynny](http://www.1001freedownloads.com/free-vector/lynny-icons-full) icon pack
- [Changelog](http://keepachangelog.com/)
- Base of this software by [CieNTi](https://github.com/CieNTi)
- CSV export by [HackInventOrg](https://github.com/HackInventOrg)

## Changelog

All notable changes to this project will be documented below this line.
This project adheres to [Semantic Versioning](http://semver.org/).


## [Thermoplotter 0.56b] - 2023-03-20
 
### Added
- Lots of bugfixes and improvements. 
- Able to save\load profiles from file on pc.


## [1.3.0.A] - 2020-06-15
### Info
 - Trying to modify this programm to made full control on IR Reflow stations at [Amperka Forum](http://forum.amperka.ru/threads/%D0%98%D0%9A-%D0%BF%D0%B0%D1%8F%D0%BB%D1%8C%D0%BD%D0%B0%D1%8F-%D1%81%D1%82%D0%B0%D0%BD%D1%86%D0%B8%D1%8F-%D0%BD%D0%B0-arduino-mega-2560-%D0%94%D0%BE%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%BA%D0%B0-%D1%81%D0%BA%D0%B5%D1%82%D1%87%D0%B0-ars_v2_lilium_jsn.10176/)

### Added
- Sending commands over COM port
- Implemented Saving and loading configuration on program start and with buttons ( [Issue #10](https://github.com/CieNTi/serial_port_plotter/issues/10) THX: netbomo 5fd5021 )
- channel legend moved to right & plotting from right corner.
- implemented Plot Clean, by recieving command over COM (check inside help)
- added fast command buttons

### Bugfix
- com port control enables correctly if port plotter was started before device connected to pc 
- clean plot button now don't erase plottable names
- Y-Tick count loads propperly. 


## [1.3.0] - 2018-08-01

### Info

- Built with QT 5.11.1
- QT libraries updated and new plot features implemented
- Beginning of version 1.3

### Added

- COM port refresh button to update the list
- Channel visibility control added to turn off unwanted channel
- Autoscale button for Y axis will autoscale to the highest value + 10%
- Save to CSV support

### Changed

- qDarkStyle updated to 2.5.4
- qCustomplot updated 2.0.1

### Bugfix

- Axis rename dialog gets focus when popup occurs

## [1.2.2] - 2018-07-26

### Info

- Project forked from HackInvent since 1.2.1

### Added

- UART debug textBox
- Textbox control ( toggle visible and toggle data filter )

## [1.2.1] - 2017-09-24

### Fixed

- Support for float/double has been added
- Linux build fails because no `serial_port_plotter_res.o` file was found (Issue #4)

## [1.2.0] - 2016-08-28

### Added

- Negative numbers support ([cap we](https://developer.mbed.org/users/capwe/) FIX at [mbed forums](https://developer.mbed.org/comments/perm/22672/))
- Support for high baud rates (tested up to 912600 bps)

## [1.1.0] - 2016-08-28

### Added

- Original qdarkstyle resources (icons are working now)
- Manifest and all Windows related/recommended configs
- *Line Icon Set* icons in 3 colors
- *Lynny* icons in 3 colors
- Inno Setup file with auto-pack .bat file (installer tested on WinXP-32b and Win10-64b)
- Play/Pause/Stop, Clear and Help toolbar buttons

### Changed

- Resources structure
- Updated qcustomplot to v1.3.2
- Menubar is replaced by icon toolbar for usability
- [WiP] mainwindow.cpp doxygen friendly comments

### Removed

- Control over number of points
- Delete previous graph data
- *Connect* and *Start/Stop plot* buttons

## [1.0.0] - 2014-08-31

### Added

- Original [Borislav Kereziev](mailto:b.kereziev@gmail.com) work commit [source](https://developer.mbed.org/users/borislav/notebook/serial-port-plotter/)


## To-Do

- Port list refresh
- Fill baud automatically and allow custom by textbox (when COM ui)
- PNG *WITH* transparency
- Separate `receive_data` from `process_data` to allow non-throttled operations

[1.3.0]: https://github.com/Eriobis/serial_port_plotter/releases/tag/v1.3.0
[1.2.2]: https://github.com/Eriobis/serial_port_plotter/releases/tag/v1.2.2
[1.2.0]: https://github.com/CieNTi/serial_port_plotter/releases/tag/v1.2.0
[1.1.0]: https://github.com/CieNTi/serial_port_plotter/releases/tag/v1.1.0
[1.0.0]: https://github.com/CieNTi/serial_port_plotter/releases/tag/v1.0.0
