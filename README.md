Valentina
=========
Valentina is a cross-platform patternmaking program which allows designers complete control of the patternmaking process. Patterns can be created using either standard sizing tables or an individual’s set of measurements. Valentina enables haute couture and bespoke patternmaking techniques for creating digital, scalable, sharable patterns with perfect fit. 

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

Compiling Valentina
====================

See forum thread on building with Qt: https://forum.valentina-project.org/t/how-to-build-with-qt/1833 

Basic Prerequisites:   
   * Qt 5.7.0 or later https://www1.qt.io/download-open-source (included in Qt install)
   * Qt Creator 4.3.1 or later (to build code) https://www1.qt.io/download-open-source (included in Qt install)
   * Qt Chooser (to maintain Qt, set Qt default version) https://www1.qt.io/download-open-source (included in Qt install)
   * git (for working with repository)  https://git-scm.com/downloads
   
Additional:
   ________
   * <em>Unix</em>: 
     - g++ compiler (at least GCC 4.7 is needed and GCC 4.8 is recommended) or clang (minimum clang 3.4 (2014), current 5.0 (2017))
     - xpdf (pdftops)
     - OpenGL libraries
     - Qt build libraries
     <em>Fedora</em>:
       > sudo yum groupinstall "C Development Tools and Libraries"
       > sudo yum install mesa-libGL-devel
     <em>Suse</em>:
       > sudo zypper install -t pattern devel_basis
     <em>Ubuntu</em>:
     - g++ 
       > sudo apt-get install build-essential  
       > sudo apt-get install g++-4.8   
     - clang (optional)
       > sudo apt-get install clang-5.0 lldb-5.0 lld-5.0         
     - xpdf (pdftops in poppler-utils 0.41.0)  
       > sudo apt-get install poppler-utils   
     - mesa-utils (for Open GL)  
       > sudo apt-get install mesa-utils libgl1-mesa-dev       
     - - ccache (for quick build time after building code once) 
       >  sudo apt-get install ccache 
     - - qt build libraries
       > sudo apt-get install qtdeclarative5-dev libfontconfig1-dev libfreetype6-dev libx11-dev libxext-dev libxfixes-dev
       > sudo apt-get install libxi-dev libxrender-dev libxcb1-dev libx11-xcb-dev libxcb-glx0-dev 
       ________
   * <em>MAC OSX</em>:
   ________
   * <em>Windows</em>:     
     - MinGW v5.0.2 from http://mingw-w64.org/doku.php       
     - Xpdf v4.0.0 (xpdftools) from http://www.xpdfreader.com/download.html. Put tool pdftops.exe 
     in the same directory with Valentina's binary file.

The installed toolchains have to match the one Qt was compiled with.

You can build from within the <em>QtCreator</em> IDE (see forum post above), or build with Qt's <em>qmake</em> from a terminal window:

    cd $SOURCE_DIRECTORY\build
    qmake ..\Valentina.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache
    make (or mingw32-make or nmake or jom, depending on your platform)

Note: On some platforms e.g. Fedora, all Qt-tools come as qmake-qt5, etc. In this case you should install <em>qtchooser</em>. This wrapper resolves the Qt version with respect to QT_SELECT. So run 

    export QT_SELECT=5

prior to running <em>qmake</em>. You will not come along with just using <em>qmake-qt5</em> here.

Note: In order to build and use Valentina, the PATH environment variable needs to be extended to locate qmake, moc and other Qt tools. This is done by adding your Qt's installation path (eg c:\Qt\%VERSION%\bin) to the PATH variable. Check your Qt installation to find the correct path to enter. 

For newer versions of Windows, PATH can be extended through this menu:

     Control Panel|System|Advanced|Environment

You may also need to ensure that the locations of your compiler and other build tools are listed in the PATH variable. This will depend on your choice of software development environment.

Unix systems:
Default prefix for command "make install" is /usr. For using another prefix build with qmake command:

    qmake PREFIX=/usr/local PREFIX_LIB=/usr/lib/i386-linux-gnu Valentina.pro -r CONFIG+=noDebugSymbols CONFIG+=no_ccache

where /usr/local is a new prefix for installation binary files and /usr/lib/i386-linux-gnu is new prefix for install libraries.  

Note: If you use <em>QtChooser</em> to update and set the latest QT as default, and within the <em>QtCreator</em> IDE you define your compiler and compiler kit, and assign the compiler kit to your project, then there's no need to export or define path prefixes, this is automatically set in Qt. Your operating system's path to your Qt executable files is still required in PATH system variable via your Linux's .bashrc or Windows' Control Panel.

LICENSING
==========
Valentina is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Valentina is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

See LICENSE_GPL.txt file for further information

Other components released under:
* QMuParser - BSD license
* VPropertyExplorer - LGPLv2.1 license
© 2017 GitHub, Inc.

