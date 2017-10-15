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
See this thread on forum for building with Qt: https://forum.valentina-project.org/t/how-to-build-with-qt/1833  

Prerequisites:   
   * Qt 5.7.0 or later (to provide Qt widgets)
   * Qt Creator 4.3.1 or later (to build code)
   * Qt Chooser (to maintain Qt)
   * git (for working with repository)   
   * On Unix:     
     - g++ (at least GCC 4.7 is needed and GCC 4.8 is recommended) or
       clang (clang 3.4 recommended)
     - xpdf (tool pdftops). 
     - additional packages needed for building on Ubuntu: 
     - - mesa-utils (for Open GL)
     - - ccache (for quick build time after building code once)
   * On Windows:   
     - MinGW  
     - Xpdf is an open source viewer for Portable Document Format (PDF) 
     files. Website http://www.foolabs.com/xpdf/. Put tool pdftops.exe 
     in the same directory with Valentina's binary file.

The installed toolchains have to match the one Qt was compiled with.

You can build Valentina within the <em>Qt Creator</em> IDE (see forum post above), or build with Qt's <em>qmake</em> from terminal window:

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
If you use QtChooser to update and set the latest QT version as your default, and within the QtCreator IDE you define your compiler and compiler kit, and assign the compiler kit to your project, then issues with prefixes are taken care of automatically when you run 'Build/Run Qmake' followed by 'Build/Build All'.

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

