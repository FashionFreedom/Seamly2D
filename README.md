![Seamly Banner](https://github.com/FashionFreedom/Seamly2D/blob/develop/share/img/Seamly2D_banner.svg)
# Seamly2D

Seamly2D is pattern design software to create reusable, scalable, sharable custom-fit patterns with using historical, couture and bespoke pattern techniques. Seamly2D saves time and improves fit by creating patterns that are reusable parametric CAD engineering documents.

Unlike other pattern design software, Seamly2D allows designers to have complete control of the patternmaking process. Each reusable, scalable pattern can read multi-size tables for boutique sizing *and* read individual measurement files for custom-fit.

Seamly2D is open source software released under the GPLv3+ license.  Seamly2D is available for Windows, MacOS, and Linux.  Learn more about Seamly2D by joining our active, friendly community at https://forum.seamly.net

## Supported platforms:  
   * Windows XP SP2 (32-bit) or later   
   * Ubuntu Linux 14.04 (32-bit/64-bit) or later   
   * OpenSUSE 13.02 (32-bit/64-bit) or later   
   * Fedora 24 (32-bit/64-bit) or later  
   * Mac OS X 10.8 (64-bit) or later  

## Seamly2D Community Resources:
   * [Homepage and downloads](https://fashionfreedom.eu)  
   * [Wiki & User Manual](https://wiki.valentinaproject.com)  
   * [Forum](https://forum.seamly.net/)  
   * [Translations](https://www.transifex.com/organization/valentina)  
   * [Code repository](https://github.com/fashionfreedom/seamly2d)
   * PPA for Ubuntu (release)   ppa:susan-spencer/seamly2d
   * PPA for Ubuntu (develop)   ppa:susan-spencer/seamly2d-dayly-build   
   * IRC channel #seamly2d on freenode    

___________________________________________________
## Download Seamly2D code

### Windows:  
* [stable version](https://bintray.com/fashionfreedom/Seamly2D/Seamly2D-win_release)  
* [development version](https://bintray.com/fashionfreedom/Seamly2D/Seamly2D-win_auto-upload)  

### Mac OS X  
* [stable version](https://bintray.com/fashionfreedom/Seamly2D/Seamly2D-mac_release)  
* [development version](https://bintray.com/fashionfreedom/Seamly2D/Seamly2D-mac_auto-upload)  

### Ubuntu  
release version:  
```
sudo add-apt-repository ppa:susan-spencer/seamly2d  
sudo apt-get update  
sudo apt-get install seamly2d  
```
development version:  
```
sudo add-apt-repository ppa:susan-spencer/seamly2d-dayly-build  
sudo apt-get update  
sudo apt-get install seamly2d  
```
___________________________________________________
## Build Seamly2D

Basic Software Prerequisites:  
* Qt 5.7.0 or later - https://www.qt.io/download-open-source (includes Qt, QtCreator, and QtChooser)
* QtCreator 4.3.1 or later (to build code)
* QtChooser (to select or update Qt version)
* Git - either Git from https://git-scm.com/downloads or Github Desktop (recommended for Windows and Mac) from https://desktop.github.com/
* Compiler - either gcc from https://gcc.gnu.org, or mingw from https://sourceforge.net/projects/mingw/
* XpdfWidget/Qt - https://www.xpdfreader.com/XpdfWidget.html
* Check the sections below for your operating system to find additional installation requirements

Learn Seamly2D development methods and styles:
   * Git Flow workflow: https://medium.com/@devmrin/learn-complete-gitflow-workflow-basics-how-to-from-start-to-finish-8756ad5b7394
   * Gibhub commit message style guide: https://www.conventionalcommits.org/en/v1.0.0/
   * Github issue description style guide: https://guides.github.com/features/issues/

Learn Seamly2D build method:
   * Building with Qt: https://forum.seamly.net/t/how-to-build-with-qt/1833

___________________________________________________
## Install Additional Libraries and Programs:

### Linux (general)
* For Linux in general, these additional tools and libraries may be required:
- g++ compiler (at least GCC 4.7 is needed and GCC 4.8 is recommended) OR clang (minimum clang 3.4 (2014), current 5.0 (2017)),
- xpdf (pdftops)
- OpenGL libraries
- Qt build libraries*
- XMLpatterns qt5 xml bindings https://command-not-found.com/xmlpatterns
* All Qt-tools come as qmake-qt5, etc. If you build from command line instead of within Qt Creator, run the following prior to running *qmake-qt5*:
```
    export QT_SELECT=5
```
* The default prefix for command `make install` is `/usr`. For using another prefix build with qmake command:  
```
    qmake PREFIX=/usr/local PREFIX_LIB=/usr/lib/i386-linux-gnu Seamly2D.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache
```
where `/usr/local` is a new prefix for installation binary files and `/usr/lib/i386-linux-gnu` is new prefix for install libraries.
Add path to Qt using `.bashrc`

#### Fedora
```
sudo yum groupinstall "C Development Tools and Libraries"  
sudo yum install mesa-libGL-devel  
```
#### Suse
```   
sudo zypper install -t pattern devel_basis  
```
#### Ubuntu
* Install the following packages:
```
  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test     
  sudo apt-get -qq update  
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5  
  sudo apt-get install -y build-essential mesa-utils mesa-common-dev libgl1-mesa-dev  
  sudo apt-get install -y poppler-utils  
  sudo apt-get install -y g++-5  
  sudo apt-get install -y xpdf  
  sudo apt-get install -y xvfb  
  sudo apt-get install -y libfontconfig1-dev libfreetype6-dev  
  sudo apt-get install -y libx11-devlibxext-dev libxfixes-dev libxi-dev  
  sudo apt-get install -y libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev
  sudo apt-get install -y libqt5xmlpatterns5-dev
```
* Manually retrieve and install `ccache`
```
wget https://launchpad.net/ubuntu/+archive/primary/+files/ccache_3.3.4-1_amd64.deb  
sudo dpkg -i ccache_3.3.4-1_amd64.deb
```
* Install Qt
- If Qt is not completely installed on your distrubution, uninstall Qt then install Qt from ppa. Use ppa for your distribution, this example is from Trusty:
```
sudo add-apt-repository -y ppa:beineri/opt-qt592-trusty  
sudo apt-get -qq update  
sudo apt-get install -y qt59-meta-full qt59-meta-dbg-full qt59creator  
```
* Configure Qt environment variables:  
```
sudo chmod +x /opt/qt59/bin/qt59-env.sh  
sudo /opt/qt59/bin/qt59-env.sh  
```   

### MAC OSX
--> Would some of our Mac devs fill in this section?

### Windows
* Install the following packages:
- MinGW v5.0.2 from http://mingw-w64.org/doku.php  
- Xpdf v4.0.0 (xpdftools) from http://www.xpdfreader.com/download.html. Put tool pdftops.exe in the same directory with Seamly2D's binary file.
* Add paths for Qt and mingw to the Windows PATH environment variable through Control Panel:
```
     [Control Panel|System And Security|System|Advanced Tab|Environment Variables button]
```
___________________________________________________
## Additional Information about Qt
* After Seamly2d, gcc or mingw, xpdf, and Qt or QtCreator installation, add their paths (eg `c:\Qt\%VERSION%\bin`)to your operating system's `PATH` environment variable via Linux `.bashrc` or Windows' Control Panel.
* Use *QtChooser* to update and set the latest QT version as default.
* Within *QtCreator* select your compiler in the compiler kit, and assign the compiler kit to your project. This automatically defines your Qt variables for compilers, debuggers, etc.
* You can build Seamly2D from within the *QtCreator* IDE (see [forum post](https://forum.seamly.net/t/how-to-build-with-qt/183)) **OR** from command line using Qt's *qmake-qtx* from a terminal window:
```
cd $SOURCE_DIRECTORY\build
qmake ..\Seamly2D.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache
make (or mingw32-make or nmake or jom, depending on your platform)
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
