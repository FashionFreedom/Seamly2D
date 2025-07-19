# Code Signing Implementation

## Overview

The code signing workflow is integrated into the main CI workflow (`ci.yml`) and provides secure digital signing for Windows executables using Google Cloud KMS with CloudHSM.

### Supported Platforms
- **Windows 64-bit**: `seamly2d.exe`, `seamlyme.exe`, `Seamly2D-installer.exe`
- **Windows 32-bit**: `seamly2d.exe`, `seamlyme.exe`, `Seamly2D-installer.exe`

## Implementation

### 1. **Secrets and inputs**

**Required GitHub Secrets**:
- `SEAMLY_SIGNING_PROJECT_ID` (Google Cloud project number, e.g., "244418363949")
- `SEAMLY_SIGNING_LOCATION` (KMS location, e.g., "us-central1")
- `SEAMLY_SIGNING_KEYRING_NAME` (KMS keyring name)
- `SEAMLY_SIGNING_KEY_NAME` (KMS key name)
- `SEAMLY_SIGNING_SA_KEY` (Service account JSON key)

### 2. **Usage**

**To Skip Signing (Emergency Override)**:
1. Go to repository **Settings** → **Secrets and variables** → **Actions**
2. Add a new **Variable** (not secret):
   - **Name**: `SKIP_SIGNING_AND_RELEASE_UNSIGNED`
   - **Value**: `true`
3. Push to `develop` branch to trigger workflow
4. Workflow will:
   - ✅ Build Windows 64-bit and 32-bit executables
   - ⏭️ Skip signing approval step entirely
   - 📝 Show clear notice that signing was skipped
   - 📦 Release unsigned executables with warnings

**To Re-enable Signing**:
1. Set variable to `false` or delete it entirely
2. Push to `develop` branch
3. Normal signing workflow will resume with approval required

**Security Considerations**:
- ⚠️ **WARNING**: Unsigned executables may trigger security warnings
- Should only be used for testing or emergency releases
- Clear audit trail shows when signing was skipped
- Requires repository variable access (typically maintainer level)
- Not recommended for production releases unless absolutely necessary

**When to Use**:
- Certificate expiration during critical release
- Google Cloud KMS service disruption
- Signing infrastructure maintenance
- Testing builds without signing overhead
- Emergency security patches that cannot wait for signing

**Audit Trail**:
- Workflow logs clearly indicate signing was skipped
- Reason for skipping is documented in logs
- User who set the variable is tracked
- Timestamp of skip operation is recorded
