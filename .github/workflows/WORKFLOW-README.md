# GitHub Workflows Overview

* Automated workflows:

** There is one automated Github Action to test code (doesn't build releases or pre-releases):
  -[build-auto-qttest-on-push-or-pullrequest](.github/workflows/build-auto-qttest-on-push-or-pullrequest.yml). This workflow runs whenever any new branch, or code to an existing branch, is pushed to the repo. Also tests the code whenever a pull_request is made.

** There are two automated GitHub Actions to build code into Releases or Pre-releases:
  -[build-auto-weekly-release-on-cron](.github/workflows/build-auto-weekly-release-on-crom.yml) - Triggers on a cron schedule (every Monday at 00:00/12 AM). Creates a rolling release using date versioning format YYYYMMDD.HHMM. The rolling release version appears in the 'About' menu. This Github Action workflow builds the Linux appimage, Windows installer , and MacOS .dmg packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage package is built for the KDE desktop.
  -[build-auto-prerelease-on-pullrequest-merge](.github/workflows/bbuild-auto-prerelease-on-pullrequest-merge.yml) - Triggers when a pull request is successfully merged into the develop branch. Creates a Pre-release using prefix 'Pre-Release-' plus the date versioning format YYYYMMDD.HHMM. The Pre-release version appears in the 'About' menu. Builds the Linux appimage, Windows installer , and MacOS .dmg packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage package is built for the KDE desktop.

*** Summary: Whenever a pull request is made, the following actions occur
    - 'build-auto-qttest-on-push-or-pullrequest' automatically runs to test the code
    - if this workflow passes, then the repo admin can merge the PR into the develop branch
    - 'build-auto-prerelease-on-pullrequest-merge' automatically runs after the PR is merged into the develop branch

* Manual build workflows:

** This are four manual Github Actions to build code into Pre-Releases:
  -[build-manual-weekly-release](.github/workflows/build-manual-weekly-release.yml) - Can be triggered manually. Created to test changes to the automated 'build-auto-weekly-release-on-cron' build workflow, above. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'
  -[build-manual-appimage](.github/workflows/build-manual-appimage.yml) - Can be triggered manually. Created to test changes to the LinuxAppImage build workflow. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'
  -[build-manual-macos](.github/workflows/build-manual-macos.yml) - Can be triggered manually. Created to test changes to the the MacOS build workflow. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'
  -[build-manual-windows](.github/workflows/build-manual-windows.yml) - Can be triggered manually. Created to test changes to the the Windows build workflow. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'

---
## The following information is deprecated as of 2021-11-28 and should be updated...

 1. Setup
   1.1 `actions/create-release` - outputs the upload url for the new release  
   1.3 [Install Qt](#iq)  
   1.4 [Enable Developer Command Prompt](#edcp)  
 2. Packaging & Distribution  
   2.1 [AppImages](#ai)  
   2.2 [Nullsoft Scriptable Install System](#nsis)  
   2.3 Code Signing - signs the windows installer using `signtool.exe`  
   2.4 `actions/upload-release-asset` - annoyingly can't infer the content type by itself  

### Setup

#### <a id="iq"></a>[Install Qt](https://github.com/marketplace/actions/install-qt)

Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-latest, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](jurplel/install-qt-action@v2.13.0) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.

#### <a id="edcp"></a>[Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt)

Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.

### Packaging & Distribution

#### <a id="ai"></a>AppImages

To build [Linux AppImages](https://appimage.org/) this project uses [linuxdeployqt](https://github.com/probonopd/linuxdeployqt).

However, `linuxdeployqt` enforces a strict Glibc version on the target system it's running on (for portability reasons)[1]. At this moment that specific version is the same as the version on Ubuntu 16.04. While there is a Ubuntu 16.04 runner in GitHub, for consistency the workflows run on ubuntu-latest (Ubuntu 18.04 at this point). And to run the `linuxdeployqt` library, a docker container is used, built from the following [Dockerfile](https://github.com/mhitza/docker-linuxdeployqt/blob/master/Dockerfile).

The container is pulled from the docker registry instead of building it during the run for performance reasons. In the grand scheme of things, it might save a minute or two within the workflow, which seems insignificant when the builds take over 20 minutes on average. If this makes maintenance harder, it should be switched over.

#### <a id="nsis"></a>[Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page)

Or NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
