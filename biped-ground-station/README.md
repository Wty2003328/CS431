# Biped Ground Station

## Table of Contents

1. [Project Prerequisites](#project-prerequisites)
2. [Qt Creator Setup](#qt-creator-setup)
3. [Setting Up the Project](#setting-up-the-project)
4. [Building the Project](#building-the-project)

## Project Prerequisites

This project depends on the installation of the following system packages:

Ubuntu:
```bash
apt install cmake libboost-all-dev libgl1-mesa-dev qtcreator qt6-base-dev
```

macOS:
```bash
brew install boost cmake qt-creator qt6
```

Click [here](https://brew.sh/) on how to install Homebrew on macOS.

Note: there are no CS 431 lab implementation objectives associated with this project. Furthermore, there is limited documentation for this project. For any 4-credit students seeking to use this project for their 4-credit projects, contact the TA for assistance.

## Qt Creator Setup

First, launch the Qt Creator.

On macOS, Qt Creator may display a `Link with Qt` message at the bottom of the window during the first launch. In this case, link with the Qt libraries as follows:
1. Click `Link with Qt` button next to the message at the bottom of the window.
2. In the file dialog window, type in the path to the Homebrew installation (should be either `/opt/homebrew` for Apple-silicon-based macOS or `/usr/local` for Intel-based macOS).
3. If the path entered is valid, the color of the path text should turn from red to black.
4. Click `Link with Qt` in the file dialog window.

## Setting Up the Project

On the lab workstation, create a project directory under your home document directory and navigate to the created project directory as follows:
```bash
mkdir -pv ~/Documents/Projects
cd ~/Documents/Projects
```

Under the created project directory above, clone this project using `git clone <project-url> biped-ground-station`.

After cloning the project, navigate to the project root directory as follows:
```bash
cd ~/Documents/Projects/biped-ground-station
```

To set up the project, perform the following in the Qt Creator:
1. Click `File` menu in the upper-left corner.
2. In `File` menu, select `Open File or Project...`.
3. In the file dialog window, select the file `<project root directory>/src/biped-ground-station/CMakeLists.txt`.
4. In `Configure Project` page, for Ubuntu, select only `Desktop` kit.
5. In `Configure Project` page, for Intel-based macOS, select only `Desktop (x86-darwin-generic-mach_o-64bit)` kit.
6. In `Configure Project` page, for Apple-silicon-based macOS, select only `Desktop (arm-darwin-generic-mach_o-64bit)` kit.
7. In `Configure Project` page, click `Details` for the selected kit and then select only `Debug` and `Release` build types.
8. For `Debug` build type, replace the existing path with `<project root directory>/build/biped-ground-station/debug`.
9. For `Release` build type, replace the existing path with `<project root directory>/build/biped-ground-station/release`.
10. Click `Configure Project` button to complete the project configuration.
11. Select build type, either `Debug` or `Release`, by clicking the monitor icon in the lower-left corner.
12. Wait for all progress bars in the lower-right corner to finish.

## Building the Project

To build the project in the Qt Creator, click the hammer icon in the lower-left corner.

To run the built project in the Qt Creator, click the play icon in the lower-left corner. To run the project in debug mode in the Qt Creator, click the debug play icon in the lower-left corner. It is recommended to select `Debug` build type before running the project in debug mode.

After it is set up in the Qt Creator, the project can also be built and run in the command line.

First, navigate to the project root directory as follows:
```bash
cd ~/Documents/Projects/biped-ground-station
```

To build the debug project in the command line, perform the following:
```bash
cd build/biped-ground-station/debug
make
```

To build the release project in the command line, perform the following:
```bash
cd build/biped-ground-station/release
make
```

To facilitate the building process, enable parallel make jobs when using `make` as follows:
```bash
make -j$(getconf _NPROCESSORS_ONLN)
```

To run the built debug project in the command line, perform the following:
```bash
cd build/biped-ground-station/debug
./biped-ground-station
```

To run the built release project in the command line, perform the following:
```bash
cd build/biped-ground-station/release
./biped-ground-station
```

Written by Simon Yu.
