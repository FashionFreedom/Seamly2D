# GitHub Workflows Overview

There are three distinct GitHub workflows:

[build-weekly-release.yml](build-weekly-release.yml) - triggers on a cron schedule (every Monday at 00:00/12 AM) and creates a `weekly-*` tag, then proceeds to build the application and packages.

[build-release-assets](build-release-assets.yml) - triggers when a release is created manually, or through an API. Does not trigger for the release created by the `build-weekly-release.yml` workflow. That is because a release created by a workflow does not emit by design events which could trigger other workflows. Switching from the GITHUB\_TOKEN to a personal action token should disable this behavior.

[pull-request.yml](pull-request.yml) - triggers as a pull request check, builds, and tests on the `ubuntu-latest` runner.


---
 1. Setup
   1.1 `actions/create-release` - outputs the upload url for the new release  
   1.3 [Install Qt](#iq)  
   1.4 [Enable Developer Command Prompt](#edcp)  
 2. Packaging & Distribution  
   2.1 [AppImages](#ai)  
   2.2 [Nullsoft Scriptable Install System](#nsis)  
   2.3 Code Signing - signs the windows installer using `signtool.exe`  
   2.4 `actions/upload-release-asset` - annoyingly can't infer the content type by itself  

## Setup

### <a id="iq"></a>[Install Qt](https://github.com/marketplace/actions/install-qt)

Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-latest, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](jurplel/install-qt-action@v2.10.0) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.

### <a id="edcp"></a>[Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt)

Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.

## Packaging & Distribution

### <a id="ai"></a>AppImages

To build [Linux AppImages](https://appimage.org/) this project uses [linuxdeployqt](https://github.com/probonopd/linuxdeployqt).

However, `linuxdeployqt` enforces a strict Glibc version on the target system it's running on (for portability reasons)[1]. At this moment that specific version is the same as the version on Ubuntu 16.04. While there is a Ubuntu 16.04 runner in GitHub, for consistency the workflows run on ubuntu-latest (Ubuntu 18.04 at this point). And to run the `linuxdeployqt` library, a docker container is used, built from the following [Dockerfile](https://github.com/mhitza/docker-linuxdeployqt/blob/master/Dockerfile).

The container is pulled from the docker registry instead of building it during the run for performance reasons. In the grand scheme of things, it might save a minute or two within the workflow, which seems insignificant when the builds take over 20 minutes on average. If this makes maintenance harder, it should be switched over.

### <a id="nsis"></a>[Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page)

Or NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.


