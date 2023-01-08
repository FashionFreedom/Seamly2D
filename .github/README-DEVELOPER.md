## Build Seamly2D

Basic Software Prerequisites:  
* Qt 5.15.2  - https://www.qt.io/download-open-source (includes Qt, QtCreator, QtChooser, and Qt Maintenance Tool)
* Git - either Git from https://git-scm.com/downloads or Github Desktop (recommended for Windows and Mac) from https://desktop.github.com/
* Compiler - MSVC 2019, gcc, and g++ are included with QtCreator, and you can add or update them using the Qt Maintenance Tool.
* pdftops - To convert PDF to PS or EPS. Bundled for mac and in win in dist folder
* Check the sections below for your operating system to find additional installation requirements

Development methods and styles:
   * GitHub Flow workflow: https://githubflow.github.io/
   * Gibhub commit message style guide: https://www.conventionalcommits.org/en/v1.0.0/
   * Github issue description style guide: https://guides.github.com/features/issues/

Build method:
   * Read more about code styles, issues, and other developer items of interest on our [Developer wiki](https://github.com/FashionFreedom/Seamly2D/wiki).
___________________________________________________
## Install Additional Libraries and Programs:

### Build Seamly code on Linux

These instructions apply in general, with Ubuntu 22.04 as example:

* Install QtCreator for your distribution: https://wiki.qt.io/VendorPackages.
* For Linux in general, these additional tools and libraries are required:
  - gnu compiler
  - poppler (pdftops)
  - OpenGL libraries
* Install Qt 5.15
  - Example for Ubuntu 22.04: Install the following packages to have Qt5 build environment ready:
```
  sudo apt install -y libfuse2 build-essential git qt5-qmake qtbase5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qttools5-dev-tools
```
* Build and install:  
```
    qmake Seamly2D.pro CONFIG+=noDebugSymbols CONFIG+=no_ccache
    make -j$(nproc)
    sudo make install
```
* The default prefix for command `make install` is `/usr`. For using another prefix build with qmake command:  
```
    qmake PREFIX=/usr/local Seamly2D.pro CONFIG+=noDebugSymbols CONFIG+=no_ccache
```
where `/usr/local` is a new prefix for installation binary files.

In doubt check how the github action CI [does it](workflows/build-release.yml).

### Build Seamly code on MAC OSX
* Download Xcode 11 - https://developer.apple.com/download/all/
* Download QtCreator - https://www.qt.io/download-thank-you
* Setup/validate build environment - https://doc.qt.io/qt-5/macos.html#build-environment:
   * Switch to Xcode: `sudo xcode-select --switch /Applications/Xcode.app`
   * Validate clang compiler points to Xcode: `xcrun -sdk macosx -find clang`
   * Validate SDK version (macOS 10.15): `xcrun -sdk macosx --show-sdk-path`

### Build Seamly code on Windows 10
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
* In *QtCreator* create your compiler kit (eg MSVC 2019, g++, gdb) and assign the compiler kit to your project. This automatically defines your Qt variables for compilers, debuggers, etc.
* Build Seamly2D from within *QtCreator* (see [forum post](https://forum.seamly.net/t/how-to-build-with-qt/183)) **OR** from command line using Qt's *qmake* from a terminal window:
```
cd $SOURCE_DIRECTORY\build
qmake ..\Seamly2D.pro CONFIG+=noDebugSymbols CONFIG+=no_ccache
make (or nmake or jom, depending on your platform)
```