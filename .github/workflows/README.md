# Seamly2D GitHub Workflows Overview

## Automated Workflows

### [CI](ci.yml) - Main Continuous Integration Workflow
**Triggers**: Pull requests, pushes to develop, scheduled releases (Mondays 01:30 UTC), manual dispatch

**Features**:
- **Tests**: Builds all platforms on pull requests with downloadable artifacts and Linux unit tests
- **Pre-Releases**: Automatic prereleases when PRs are merged to develop branch
- **Releases**: Scheduled weekly releases with date-based versioning (vYYYY.MM.DD.HHMM)
- **Code Signing**: Integrated Windows and Mac code signing for develop branch
  - Signs both 64-bit and 32-bit Windows executables
  - Signs and notarizes Mac builds
  - Uses Google Cloud KMS with CloudHSM for secure signing
  - Uses Mac Developer ID certificate and private key and notarize API key in secrets

**Builds**: Linux AppImage, Windows 64-bit/32-bit installers (.exe/.zip), macOS (.dmg/.zip)

## Code Signing Workflow

### Integrated Signing Process
The main CI workflow includes integrated code signing for Windows and Mac executables.

### Signing Requirements
- **Branch**: Only runs on `develop` branch
- **Secrets**: Requires Google Cloud KMS secrets and Mac Developer ID certificate and notarize API key configured

## Emergency Procedures

### Skip Code Signing (Emergency Override)
When signing infrastructure fails (certificate expiration, KMS issues, etc.):

1. Go to repository **Settings** → **Secrets and variables** → **Actions**
2. Remove the **Secret** `SEAMLY_SIGNING_PROJECT_ID` for windows and `APPLE_SIGN_IDENTITY` for mac
3. Push to `develop` branch to trigger workflow
4. Workflow will:
   - ✅ Build Windows 64-bit and 32-bit executables
   - 📦 Release unsigned executables with warnings

**⚠️ Warning**: Unsigned executables will trigger security warnings and should only be used for testing or emergency releases.

### Re-enable Code Signing
To restore normal signing after emergency:

1. Add the **Secrets** back in
2. Push to `develop` branch
3. Normal signing workflow will resume with approval required

## External Github Actions
- [Install Qt](https://github.com/marketplace/actions/install-qt). Referenced as `jurplel/install-qt-action`, installs the Qt platform across all the three different runners (ubuntu-18.04, macos-latest, windows-latest) consistently. Internally it uses the [aqtinstall](https://github.com/miurahr/aqtinstall/) installer written in Python. Worth knowing if those errors propagate up through the GitHub action.
- [Enable Developer Command Prompt](https://github.com/marketplace/actions/enable-developer-command-prompt) Referenced as `ilammy/msvc-dev-cmd`, sets up the command line environment on the windows-latest runner (`PATH` and such) to expose Microsoft Visual C++.
- [softprops/action-gh-release](https://github.com/marketplace/actions/gh-release). Referenced as `softprops/action-gh-release`, creates a release and uploads all artifacts to that release.
- [Nullsoft Scriptable Install System](https://nsis.sourceforge.io/Main_Page) Not an action, but NSIS for short, builds the Windows installer using the [seamly2d-installer.nsi](/dist/seamly2d-installer.nsi) script file. As of this moment, the script includes steps for setting up a start menu group and configuration necessary to provide an uninstaller.
