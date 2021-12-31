# Seamly2D GitHub Workflows Overview
---
* AUTOMATED WORKFLOWS
** Tests:  
  [build-auto-qttest-on-pullrequest](.github/workflows/build-auto-qttest-on-pullrequest.yml). Automated Github Action to test code (doesn't build releases or pre-releases)whenever a pull_request is made.
** Pre-Releases:
  [build-auto-prerelease-on-merge](.github\workflows\build-auto-prerelease-on-merge.yml) - Automated GitHub Action to build a Pre-Release when a pull request is successfully merged into the develop branch. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'. Builds the Linux appimage (.AppImage), Windows installer .exe (.zip), and MacOS .dmg (.zip) packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage is built for the KDE desktop.
** Releases:
  [build-auto-weekly-release-on-cron](.github/workflows/build-auto-release-on-crom.yml) - Triggers on a cron schedule (every Monday at 00:00 UTC). Creates a rolling release using date versioning format YYYYMMDD.HHMM. Builds the Linux appimage (.AppImage), Windows installer .exe (.zip), and MacOS .dmg (.zip) packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage is built for the KDE desktop.
** AUTOMATED WORKFLOW SUMMARY:
    - Whenever a pull request is made 'build-auto-qttest-on-pullrequest' automatically runs to test the code
    - If this workflow passes the repo admin can Merge the PR into the develop branch which automatically runs 'build-auto-prerelease-on-merge' to build the Pre-release
    - Every Monday at 00:00 UTC 'build-auto-release-on-cron' automatically runs to build the weekly rolling Release

---
* MANUAL WORKFLOWS:
** Pre-Releases:
  -[build-manual-release](.github/workflows/build-manual-release.yml) - Manual run of steps for all three desktop builds, copied from the automated 'build-auto-release-on-cron' workflow, above. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'. Use this file to test changes to the total build process.
  -[build-manual-appimage](.github/workflows/build-manual-appimage.yml) - Manual run of steps from the Linux AppImage section in 'build-auto-release-on-cron'. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'. Use this file to test changes to the Linux build.
  -[build-manual-macos](.github/workflows/build-manual-macos.yml) - Manual run of steps from the MacOS section in 'build-auto-release-on-cron'. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'. Use this file to test changes to the MacOS build.
  -[build-manual-windows](.github/workflows/build-manual-windows.yml) - Manual run of steps from the Windows section in 'build-auto-release-on-cron'. Builds are tagged as 'Pre-Release-YYYYDDMM.HHMM'. Use this file to test changes to the Windows build.

---
## External Github Actions
* <a id="iq"></a>[Install Qt](https://github.com/marketplace/actions/install-qt). Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-latest, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](jurplel/install-qt-action@v2.13.0) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.
* <a id="edcp"></a>[Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt) Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.
* <a id="ai"></a>[Linux AppImages](https://appimage.org/) To build Linux AppImages this project uses [linuxdeployqt](https://github.com/probonopd/linuxdeployqt).
* <a id="nsis"></a>[Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page) Or NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
