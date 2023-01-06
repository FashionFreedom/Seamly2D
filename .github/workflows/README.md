# Seamly2D GitHub Workflows Overview
---
- AUTOMATED WORKFLOWS
  - Tests:
    [build-auto-qttest-on-pullrequest](build-auto-qttest-on-pullrequest.yml). Automated Github Action to build for all platforms and test code (doesn't build releases or pre-releases) whenever a pull_request is made. Artifacts can be downloaded by logged in users from github actions menu.
  - Pre-Releases:
    [build-auto-prerelease-on-merge](build-auto-prerelease-on-merge.yml) - Automated GitHub Action to build a Pre-Release when a pull request is successfully merged into the develop branch. Builds are tagged as 'Pre-Release-YYYY-DD-MM.HHMM'. Builds the Linux appimage (.AppImage), Windows installer .exe (.zip), and MacOS .dmg (.zip) packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage is built for any modern linux desktop.
  - Releases:
    [build-release](build-release.yml) - Triggers on a cron schedule (every Monday at 01:30 UTC). Creates a rolling release using date versioning format vYYYY-MM-DD.HHMM. Builds the Linux appimage (.AppImage), Windows installer .exe (.zip), and MacOS .dmg (.zip) packages. The Windows and MacOS files are packaged into .zip files to prevent browser warning messages during download. The Linux appimage is built for any modern linux desktop.
  - AUTOMATED WORKFLOW SUMMARY:
    - Whenever a pull request is made 'build-auto-qttest-on-pullrequest' automatically runs to build for all platforms and test the code
    - If this workflow passes the repo admin can Merge the PR into the develop branch which automatically runs 'build-auto-prerelease-on-merge' to build the Pre-release
    - Every Monday at 01:30 UTC 'build-release' automatically runs to build the weekly rolling Release

---
- MANUAL WORKFLOWS:
  - Releases:
    -[build-release](build-release.yml) - The build-release can also be triggered manually to create a release independent of the weekly schedule (eg. if an important bug is fixed and should be available as a release during the week).

---
## External Github Actions
- <a id="iq"></a>[Install Qt](https://github.com/marketplace/actions/install-qt). Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-latest, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](jurplel/install-qt-action@v3) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.
- <a id="edcp"></a>[Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt) Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.
- <a id="nsis"></a>[Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page) Not an action, but NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
