# GitHub Environments

This directory contains configuration for GitHub environments used in Seamly2D's CI/CD workflows.

## Current Environments

### code-signing-approval

**Purpose**: Production code signing environment for the main CI workflow

**Configuration**:
- **Required Reviewers**: SeamlySigners team
- **Wait Timer**: 0 minutes (immediate approval)
- **Deployment Branches**: `main`, `develop`
- **Protection**: Manual approval required for all signing operations

**Usage**: 
- Referenced by the `ci.yml` workflow for production Windows code signing
- Ensures all signing operations require manual approval from the SeamlySigners team
- Provides audit logging of all approvals and deployments

**Security Features**:
- Manual approval required for all signing operations
- Team-based control (only SeamlySigners team can approve)
- Complete audit trail with timestamps and approver information
- Branch protection (only allows signing from approved branches)

### code-signing-test

**Purpose**: Testing environment for code signing workflows

**Configuration**:
- **Required Reviewers**: SeamlySigners team
- **Wait Timer**: 0 minutes (immediate approval)
- **Deployment Branches**: `code-signing-test`, `code-signing-pr-branch`
- **Protection**: Manual approval required for test signing operations

**Usage**:
- Referenced by the `windows-build-sign-test.yml` workflow
- Allows safe testing of code signing infrastructure without affecting production
- Provides the same security controls as production but in an isolated environment 