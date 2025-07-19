# Seamly2D GitHub Workflows Overview

## Automated Workflows

### [CI](ci.yml) - Main Continuous Integration Workflow
**Triggers**: Pull requests, pushes to develop, scheduled releases (Mondays 01:30 UTC), manual dispatch

**Features**:
- **Tests**: Builds all platforms on pull requests with downloadable artifacts and Linux unit tests
- **Pre-Releases**: Automatic prereleases when PRs are merged to develop branch
- **Releases**: Scheduled weekly releases with date-based versioning (vYYYY.MM.DD.HHMM)
- **Code Signing**: Integrated Windows code signing for develop branch (requires team approval)
- **Emergency Skip**: Set repository variable `SKIP_SIGNING_AND_RELEASE_UNSIGNED=true` to bypass signing

**Builds**: Linux AppImage, Windows installer (.exe/.zip), macOS (.dmg/.zip)

## Manual Workflows

### [Windows Build with Code Signing (Test)](windows-build-sign-test.yml)
**Purpose**: Test Windows build and code signing workflow for testing branches
**Triggers**: Pushes to `code-signing-test` and `code-signing-pr-branch` branches
**Use Case**: Safe testing of code signing infrastructure without affecting production

## Emergency Procedures

### Skip Code Signing (Emergency Override)
When signing infrastructure fails (certificate expiration, KMS issues, etc.):

1. Go to repository **Settings** → **Secrets and variables** → **Actions**
2. Add a new **Variable** (not secret):
   - **Name**: `SKIP_SIGNING_AND_RELEASE_UNSIGNED`
   - **Value**: `true`
3. Push to trigger workflow
4. Workflow will build and release unsigned executables with clear warnings

**⚠️ Warning**: Unsigned executables may trigger security warnings and should only be used for testing or emergency releases.

## External Github Actions
- [Install Qt](https://github.com/marketplace/actions/install-qt). Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-18.04, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](https://github.com/miurahr/aqtinstall/) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.
- [Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt) Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.
- [softprops/action-gh-release](https://github.com/marketplace/actions/gh-release). Referenced as `softprops/action-gh-release`, creates a release and uploads all artifacts to that release.
- [Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page) Not an action, but NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
