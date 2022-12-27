![Seamly Banner](https://github.com/FashionFreedom/Seamly2D/blob/develop/share/img/Seamly2D_banner_700x200.png)
# Seamly2D
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/fashionfreedom/seamly2d?logo=github?color=blue&include_prereleases&style=flat-square&logo=github)   
![GitHub Workflow Status](https://img.shields.io/github/workflow/status/fashionfreedom/seamly2d/Build%20weekly%20release?style=flat-square&logo=github)  ![Travis (.org) branch](https://img.shields.io/travis/fashionfreedom/seamly2d/develop?label=travis-ci&style=flat-square&logo=travisci)   ![GitHub commit activity](https://img.shields.io/github/commit-activity/m/fashionfreedom/seamly2d?logo=github&color=brightgreen&style=flat-square)  ![GitHub contributors](https://img.shields.io/github/contributors/fashionfreedom/seamly2d?style=flat-square&logo=github)  
![GitHub](https://img.shields.io/github/license/fashionfreedom/seamly2d?color=blue&style=flat-square&logo=creativecommons)  ![GitHub language count](https://img.shields.io/github/languages/count/fashionfreedom/seamly2d?style=flat-square&logo=github)  
![Website](https://img.shields.io/website?down_message=Down&style=flat-square&up_color=brightgreen&up_message=Up&url=https%3A%2F%2Fseamly.net&logo=wordpress)   ![Discourse posts](https://img.shields.io/discourse/posts?server=https%3A%2F%2Fforum.seamly.net&style=flat-square&logo=discourse)      
![Qt](https://github.com/FashionFreedom/Seamly2D/blob/develop/.github/img/Built_with_Qt_RGB_logo_vertical_transparent_60x65px.png)

Seamly2D is pattern design software to create reusable, scalable, sharable custom-fit patterns using bespoke tailoring, haute couture, and historical pattern drafting techniques. Seamly2D saves time and improves fit by creating patterns that are reusable parametric CAD engineering documents.

Unlike other pattern design software, Seamly2D allows designers to have complete control of the patternmaking process. Each reusable, scalable pattern can read multi-size tables for boutique sizing, *and* read individual measurement files for custom-fit.

Seamly2D is open source software released under the GPLv3+ license.  Seamly2D is available for Windows, MacOS, and Linux.  Learn more about Seamly2D by joining our active, friendly community at https://forum.seamly.net

## Supported platforms:  
   * Windows 10 (64-bit)
   * Most current Linux distros via [Flathub](https://flathub.org/apps/details/net.seamly.seamly2d)
   * Mac OS X 10.8 (64-bit) or later

## Download Seamly2D
Download from our Releases page [here:](https://github.com/fashionfreedom/seamly2d/releases/latest)
___________________________________________________
## Seamly2D Community Resources:
   * [Website and downloads](https://seamly.net)  
   * [User Wiki Manual](https://wiki.seamly.net)  
   * [User Forum](https://forum.seamly.net/)  
   * [Developer Wiki](https://github.com/FashionFreedom/Seamly2D/wiki)
___________________________________________________
## Build Seamly2D

Basic Software Prerequisites:  
* Qt 5.15.2  - https://www.qt.io/download-open-source (includes Qt, QtCreator, QtChooser, and Qt Maintenance Tool)
* Git - either Git from https://git-scm.com/downloads or Github Desktop (recommended for Windows and Mac) from https://desktop.github.com/
* Compiler - MSVC 2019, gcc, and g++ are included with QtCreator, and you can add or update them using the Qt Maintenance Tool.
* XpdfWidget/Qt - https://www.xpdfreader.com/XpdfWidget.html
* Check the sections below for your operating system to find additional installation requirements

Development methods and styles:
   * GitHub Flow workflow: https://githubflow.github.io/
   * Gibhub commit message style guide: https://www.conventionalcommits.org/en/v1.0.0/
   * Github issue description style guide: https://guides.github.com/features/issues/

Build method:
   * Building with Qt: https://forum.seamly.net/t/how-to-build-with-qt/1833
   * Read more about code styles, issues, and other developer items of interest here: https://github.com/FashionFreedom/Seamly2D/wiki
___________________________________________________
## Install Additional Libraries and Programs:

### to build Seamly code on Linux (general)
* Install QtCreator for your distribution: https://wiki.qt.io/VendorPackages.
* For Linux in general, these additional tools and libraries may be required:
- gnu compiler
- poppler (pdftops)
- OpenGL libraries
- XMLpatterns:  https://command-not-found.com/xmlpatterns
* All Qt-tools come as qmake-qt5, etc. If you build from command line instead of within Qt Creator, run the following prior to running *qmake-qt5*:
```
    export QT_SELECT=5
```
* The default prefix for command `make install` is `/usr`. For using another prefix build with qmake command:  
```
    qmake PREFIX=/usr/local PREFIX_LIB=/usr/lib/i386-linux-gnu Seamly2D.pro CONFIG+=noDebugSymbols CONFIG+=no_ccache
```
where `/usr/local` is a new prefix for installation binary files and `/usr/lib/i386-linux-gnu` is new prefix for install libraries.
Add path to Qt using `.bashrc`

#### Ubuntu 22.04
* Install the following packages to have Qt5 build environment ready:
```
  sudo apt install -y libfuse2 build-essential git qt5-qmake qtbase5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qttools5-dev-tools
  sudo apt install -y poppler-utils
```

In doubt check how the github action CI [does it](workflows/build-auto-release-on-cron.yml).

### MAC OSX
* Download Xcode 11 - https://developer.apple.com/download/all/
* Download QtCreator - https://www.qt.io/download-thank-you
* Setup/validate build environment - https://doc.qt.io/qt-5/macos.html#build-environment:
   * Switch to Xcode: `sudo xcode-select --switch /Applications/Xcode.app`
   * Validate clang compiler points to Xcode: `xcrun -sdk macosx -find clang`
   * Validate SDK version (macOS 10.15): `xcrun -sdk macosx --show-sdk-path`

### Windows 10
* Download QtCreator - https://www.qt.io/download-thank-you
* Configure Qt & QtCreator- https://stackoverflow.com/questions/60245433/how-to-install-qt5-libraries-for-windows-10-for-cmake-development  
- Xpdf v4.0.0 (xpdftools) from http://www.xpdfreader.com/download.html. Put tool pdftops.exe in the same directory with Seamly2D's binary file.
* Add the Qt directory to the Windows PATH environment variable through Control Panel:
```
     [Control Panel|System And Security|System|Advanced Tab|Environment Variables button]
```
___________________________________________________
## Additional Information about Qt

* Add Seamly2d, gcc, g++, xpdf, and QtCreator directories to your operating system's `PATH` environment variable via Linux `.bashrc` or Windows' Control Panel.(example: for Windows add  `c:\Qt\%VERSION%\bin`)
* Use *QtChooser* to update and set the latest QT version as default.
* In *QtCreator* create your compiler kit (eg MSVC 2019, g++, gdb) and assign the compiler kit to your project. This automatically defines your Qt variables for compilers, debuggers, etc.
* Build Seamly2D from within *QtCreator* (see [forum post](https://forum.seamly.net/t/how-to-build-with-qt/183)) **OR** from command line using Qt's *qmake-qtx* from a terminal window:
```
cd $SOURCE_DIRECTORY\build
qmake ..\Seamly2D.pro CONFIG+=noDebugSymbols CONFIG+=no_ccache
make (or nmake or jom, depending on your platform)
```
___________________________________________________
## LICENSING
Seamly2D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Seamly2D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

See LICENSE_GPL.txt file for further information

Other components released under:  
* QMuParser - BSD license  
* VPropertyExplorer - LGPLv2.1 license
