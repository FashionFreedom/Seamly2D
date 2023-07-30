# Seamly2D GitHub Workflows Overview
---
- AUTOMATED WORKFLOWS: [CI](ci.yml)
  - Tests: Whenever a pull-request is opened, all platforms are built and the result can be downloaded from the Github actions summary page of the run. In addition, the tests are run under linux.
  - Pre-Releases: When a pull request is successfully merged into the develop branch, prereleases are build and published to Github releases page
  - Releases: Release builds are triggered on a cron schedule (every Monday at 01:30 UTC). Creates a rolling release using date versioning format vYYYY.MM.DD.HHMM. Builds the Linux appimage (.AppImage), Windows installer .exe (.zip), and MacOS .dmg (.zip) packages.

---
- MANUAL WORKFLOWS:
  - Releases:
    -[CI](ci.yml) - The release build can also be triggered manually to create a release independent of the weekly schedule (eg. if an important bug is fixed and should be available as a release during the week).

    -[CI](build_docs.yml) - The documents build can also be triggered manually independent of the weekly schedule.

---
## External Github Actions
- [Install Qt](https://github.com/marketplace/actions/install-qt). Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-18.04, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](https://github.com/miurahr/aqtinstall/) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.
- [Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt) Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.
- [softprops/action-gh-release](https://github.com/marketplace/actions/gh-release). Referenced as `softprops/action-gh-release`, creates a release and uploads all artifacts to that release.
- [Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page) Not an action, but NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
