Valentina / LibreFashion
====================

Valentina / LibreFashion creates reusable, scalable, sharable patterns with perfect fit using couture and bespoke pattern techniques. Valentina / LibreFashion saves designers and patternmakers time and improves quality by redefining patterns as accurate, reusable parametric CAD engineering documents.

Designers have complete control of the patternmaking process. Each reusable, scalable pattern can use multi-size tables for boutique sizing *and* individual measurements for custom-fit.

Valentina / LibreFashion is open source software with GPLv3+ license.  It is available for Windows, OSX, and Linux.  Valentina / LibreFashion has an active, friendly user forum at https://forum.valentina-project.org 

Supported platforms:  
   * Windows XP SP2 (32-bit) or later   
   * Ubuntu Linux 14.04 (32-bit/64-bit) or later   
   * OpenSUSE 13.02 (32-bit/64-bit) or later   
   * Fedora 24 (32-bit/64-bit) or later    
   * Mac OS X 10.8 (64-bit) or later  

Homepage and downloads     https://valentina-project.org   
Wiki & User Manual         https://wiki.valentinaproject.org    
Forum                      https://forum.valentina-project.org/       
Translations               https://www.transifex.com/organization/valentina   
PPA for Ubuntu (develop)   (soon) ...    
PPA for Ubuntu (test)      (soon) ...    
IRC channel                #valentina on irc.freenode.net  
Main repository            https://github.com/valentina-project/vpo2    

Building Valentina / LibreFashion
================

Basic Prerequisites:   
   * Qt 5.7.0 or later https://www1.qt.io/download-open-source (included in Qt install)  
   * Qt Creator 4.3.1 or later (to build code) https://www1.qt.io/download-open-source (included in Qt install)  
   * Qt Chooser (to maintain Qt, set Qt default version) https://www1.qt.io/download-open-source (included in Qt install)  
   * git (for working with repository)  https://git-scm.com/downloads    
   * Read forum on building with Qt: https://forum.valentina-project.org/t/how-to-build-with-qt/1833
   
Additional:
________________
*Linux: In general, these additional tools and libraries may be required:
 g++ compiler (at least GCC 4.7 is needed and GCC 4.8 is recommended) or clang (minimum clang 3.4 (2014), current 5.0 (2017)), xpdf (pdftops), OpenGL libraries, Qt build libraries*

<em>Fedora</em>: 
     
    sudo yum groupinstall "C Development Tools and Libraries"  
    sudo yum install mesa-libGL-devel  
     
<em>Suse</em>: 
     
    sudo zypper install -t pattern devel_basis  
         
<em>Ubuntu</em>:  

    sudo apt-get install build-essential    
    sudo apt-get install g++-4.8 
    sudo apt-get install clang-5.0 lldb-5.0 lld-5.0  
    sudo apt-get install poppler-utils 
    sudo apt-get install mesa-utils libgl1-mesa-dev
    sudo apt-get install ccache 
    sudo apt-get install qtdeclarative5-dev libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev  
    sudo apt-get install libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev  
    
________________

 <em>MAC OSX</em>:    
________________

<em>Windows</em>: 

* MinGW v5.0.2 from http://mingw-w64.org/doku.php         
* Xpdf v4.0.0 (xpdftools) from http://www.xpdfreader.com/download.html. Put tool pdftops.exe in the same directory with Valentina's binary file.  

________________

The installed toolchains have to match the one Qt was configured with.

You can build from within the <em>QtCreator</em> IDE (see forum post above), or build with Qt's <em>qmake</em> from a terminal window:

    cd $SOURCE_DIRECTORY\build
    qmake ..\Valentina.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache
    make (or mingw32-make or nmake or jom, depending on your platform)

Note: On some platforms e.g. Fedora, all Qt-tools come as qmake-qt5, etc. Run 

    export QT_SELECT=5

prior to running <em>qmake</em>. You will not come along with just using <em>qmake-qt5</em> here.

Note: In order to build and use Valentina / LibreFashion, the PATH environment variable needs to be extended to locate qmake (or qmake-qt5), moc and other Qt tools. This is done by adding your Qt's installation path (eg c:\Qt\%VERSION%\bin) to the PATH variable. Check your Qt installation to find the correct path to enter. 

For newer versions of Windows, PATH can be extended through this menu:

     Control Panel|System|Advanced|Environment

Add the locations of your compiler and other build tools to your system's PATH variable. This will depend on your choice of software development environment.

Unix systems: 
Default prefix for command "make install" is /usr. For using another prefix build with qmake command:

    qmake PREFIX=/usr/local PREFIX_LIB=/usr/lib/i386-linux-gnu Valentina.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache

where /usr/local is a new prefix for installation binary files and /usr/lib/i386-linux-gnu is new prefix for install libraries.  

Use <em>QtChooser</em> to update and set the latest QT version as default. Within the <em>QtCreator</em> IDE define your compiler and compiler kit, and assign the compiler kit to your project. This automatically defines your Path prefixes in Qt. Your operating system's path to your Qt executable files is still required in PATH system variable via your Linux's .bashrc or Windows' Control Panel.

LICENSING
================
Valentina / LibreFashion is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Valentina / LibreFashion is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

See LICENSE_GPL.txt file for further information

Other components released under:
* QMuParser - BSD license
* VPropertyExplorer - LGPLv2.1 license
© 2017 GitHub, Inc.

