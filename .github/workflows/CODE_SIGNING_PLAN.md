# Code Signing Implementation Plan

## Security Model (Recommended)

### **1. Workflow Approval Required**
- Manual approval for ALL signing operations (both scheduled and manual)
- Only members of SeamlySigners team can approve signing workflows
- Complete audit trail of who approved what commit
- Final security gate against malicious code

### **2. Trigger Strategy**
- **Scheduled releases**: Weekly releases require manual approval before signing
- **Manual signing**: Manual workflow dispatch for specific commits
- **No automatic tag triggers**: Remove automatic signing from tag creation
- **Commit SHA validation**: Always verify the exact commit being signed

### **3. Branch Protection**
- Enforce PR reviews for develop branch
- Require maintainer approval for merges
- Prevent direct pushes to develop
- Use GitHub branch protection rules

### **4. Team Permissions**
- Create new `SeamlySigners` team with "maintain" access
- Use CODEOWNERS file to require SeamlySigners team approval for signing workflow changes
- Allow SeamlyBuildTeam to modify other workflows but not signing workflows
- Only SeamlySigners team members can approve signing operations
- Only maintainers may create tags

## Implementation Plan

### 1. **File Organization**

**Plan document location**:
- `CODE_SIGNING_PLAN.md` → `.github/workflows/CODE_SIGNING_PLAN.md`

**Signing files location**:
- `jsign-image.sh` → `.github/workflows/signing/sign-windows-executable.sh`
- `jsign.jar` → `.github/workflows/signing/jsign.jar`
- `codesign-chain.pem` → `.github/workflows/signing/codesign-chain.pem`

**Script modifications needed**:
- Accept executable path as parameter
- Improve error handling (currently TODO comment)
- Add comprehensive logging and validation
- Make script more robust for CI environment

### 2. **Workflow Structure**

**File**: `.github/workflows/code-signing.yml`

**Required GitHub Secrets**:
- `SEAMLY_SIGNING_PROJECT_ID` (Google Cloud project number, e.g., "244418363949")
- `SEAMLY_SIGNING_LOCATION` (KMS location, e.g., "us-central1")
- `SEAMLY_SIGNING_KEYRING_NAME` (KMS keyring name)
- `SEAMLY_SIGNING_KEY_NAME` (KMS key name)
- `GOOGLE_CLOUD_SERVICE_ACCOUNT_KEY` (Service account JSON key)

**Environment Variables** (set in workflow):
- `SEAMLY_SIGNING_PROJECT_ID` (use project number, not project ID)
- `SEAMLY_SIGNING_LOCATION` 
- `SEAMLY_SIGNING_KEYRING_NAME`
- `SEAMLY_SIGNING_KEY_NAME`

**Workflow Inputs**:
- `commit_sha` (required) - specific commit to sign
- `version_number` (optional) - for release naming
- `test_mode` (optional) - if true, don't publish to releases

### 3. **Authentication Strategy**

**Service Account Key Approach**:
- Store service account JSON as GitHub secret
- Use `gcloud auth activate-service-account` in workflow
- Rotate service account keys regularly
- Monitor service account usage

### 4. **Workflow Jobs**

**1. Validation Job**:
- Verify commit exists and is on develop branch only
- Check if unsigned artifacts exist for the commit
- Validate user permissions and approval
- Log all validation steps with human-readable format

**2. Download Job**:
- **Source**: Windows build workflow artifacts (GitHub Actions artifacts storage)
- **Destination**: Workflow runner local filesystem
- Download `Seamly2D-windows-unsigned.zip` from Windows build workflow artifacts
- Verify artifact integrity and checksums
- Extract executables from installer if needed
- Log download operations with full file paths

**3. Signing Job**:
- Setup Google Cloud authentication
- Copy signing files from `.github/workflows/signing/` to workflow runner
- Sign individual executables:
  - `seamly2d.exe`
  - `seamlyme.exe`
- Rebuild installer with signed executables
- Sign the final installer
- Upload signed artifacts to workflow runner
- Comprehensive logging of all operations

**4. Publishing Job**:
- **Source**: Workflow runner local filesystem (signed files)
- **Destination**: GitHub releases (replacing current publish job)
- Upload signed artifacts to GitHub releases
- Delete unsigned artifacts from workflow runner
- Mark test releases as `Seamly2D-windows-test.zip` when in test mode
- Log all publishing operations

### 5. **Signing Process Details**

**Files to Sign**:
1. Individual executables (using existing script)
2. NSIS installer (using existing script)

**Script Usage**:
```bash
./signing/sign-windows-executable.sh path/to/executable.exe
```

**Signing Commands**:
- Uses jsign.jar with Google Cloud KMS
- SHA256 signing with RFC3161 timestamping
- EV certificate from your CloudHSM setup

### 6. **Security Enhancements**

**Script Improvements**:
- Add proper error handling for hash verification
- Validate all required environment variables
- Add comprehensive logging and audit trail
- Verify executable exists before signing
- Exit on any validation failure

**Workflow Security**:
- Restrict to develop branch only
- Require workflow approval for ALL signing operations
- No signing on pull requests
- Complete audit logging of all signing operations
- Validate commit SHA matches expected
- Only SeamlySigners team can approve and modify signing workflows

### 7. **Integration Strategy**

**Separate Workflow Approach**:
- Main CI builds and uploads unsigned artifacts as `Seamly2D-windows-unsigned.zip`
- Code signing workflow runs independently with approval
- Signed artifacts replace unsigned ones in releases as `Seamly2D-windows.zip`
- Clear separation of concerns

### 8. **Triggering Strategy**

**Manual Trigger**:
```bash
gh workflow run code-signing.yml -f commit_sha=abc123 -f version_number=2024.1.1.1200 -f test_mode=false
```

**Scheduled Integration**:
- Weekly scheduled release triggers signing workflow
- Manual approval required before signing proceeds
- Only triggered by schedule and workflow_dispatch events

### 9. **Artifact Management**

**Before Signing**:
- **Source**: Windows build workflow artifacts (GitHub Actions artifacts storage)
- **Destination**: Workflow runner local filesystem
- Download `Seamly2D-windows-unsigned.zip` from Windows build workflow artifacts
- Verify checksums and commit association
- Extract executables if needed
- Log all artifact operations with full file paths

**After Signing**:
- **Source**: Workflow runner local filesystem (signed files)
- **Destination**: GitHub releases (replacing current publish job)
- Upload signed artifacts as `Seamly2D-windows.zip` to GitHub releases
- Delete unsigned artifacts from workflow runner
- Maintain complete audit trail
- Verify signing was successful

**Test Mode**:
- Upload signed artifacts as `Seamly2D-windows-test.zip` to workflow artifacts
- Do not publish to GitHub releases
- Clearly mark as test in logs

### 10. **Error Handling and Monitoring**

**Validation Checks**:
- Verify commit is on develop branch only
- Check user has signing permissions and approval
- Validate artifact integrity
- Verify jsign.jar hash
- Confirm all environment variables are set

**Failure Handling**:
- Clear error messages with actionable information
- No release created on signing failure
- Unsigned artifacts retained for debug and analysis
- Detailed logs saved as workflow output
- Notification to SeamlySigners team on failure
- Workflow exits with specific error code on failure
- If signing fails during scheduled release, entire release fails

### 10.5. **Emergency Skip Signing Override**

**Purpose**: 
- Emergency mechanism to release unsigned executables when signing infrastructure fails
- Bypass signing approval when certificates expire or KMS is unavailable
- Enable testing builds without waiting for signing approval

**Implementation**:
- Environment variable: `SKIP_SIGNING_AND_RELEASE_UNSIGNED`
- When set to `true`, the signing job is completely skipped
- A separate notification job runs to clearly indicate signing was skipped
- Unsigned executables are made available for download

**Usage**:

**To Skip Signing (Emergency Override)**:
1. Go to repository **Settings** → **Secrets and variables** → **Actions**
2. Add a new **Variable** (not secret):
   - **Name**: `SKIP_SIGNING_AND_RELEASE_UNSIGNED`
   - **Value**: `true`
3. Push to trigger workflow
4. Workflow will:
   - ✅ Build Windows executables
   - ⏭️ Skip signing approval step entirely
   - 📝 Show clear notice that signing was skipped
   - 📦 Make unsigned executables available

**To Re-enable Signing**:
1. Set variable to `false` or delete it entirely
2. Push again to re-enable normal signing workflow

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

### 11. **Audit Logging Requirements**

**Log all operations with human-readable format**:
- Commit SHA being signed
- Approver information (who approved the signing)
- Timestamp of all operations
- Files being signed with full file paths and checksums
- Checksums of jsign.jar and codesign-chain.pem files used
- Signing tool version and configuration
- Success/failure status
- Error messages and stack traces
- User who triggered the workflow
- Workflow run ID for traceability
- Test mode status if applicable

### 12. **Testing Strategy**

**Development Phase**:
- Test with manual workflow dispatch using `test_mode=true`
- Use production KMS keyring on non-release commits
- Validate signing process end-to-end
- Test error conditions and recovery
- Verify test artifacts are properly marked

**Production Phase**:
- Gradual rollout with approval gates
- Monitor signing success rates
- Track audit logs for anomalies
- Regular security reviews

## Key Security Features

1. **Manual Approval Required**: No automatic signing without human review
2. **Develop Branch Only**: Restricts signing to approved code only
3. **Complete Audit Trail**: Logs every action for security monitoring
4. **Commit SHA Validation**: Ensures exact commit being signed
5. **Artifact Integrity**: Verifies files before and after signing
6. **Error Handling**: Graceful failure with clear logging
7. **Service Account Security**: Minimal permissions, regular rotation
8. **Team Isolation**: Only SeamlySigners team can approve and modify signing workflows

## Current Team Structure

### **SeamlyBuildTeam** (Current)
- **Current Members**: `sconklin`, `slspencer`, `csett86`, `DSCaskey`, `Onetchou`
- **Current Permission**: "pull" access
- **Proposed Permission**: "maintain" access
- **Responsibilities**: Maintain general CI/CD workflows (excluding signing workflows)

### **SeamlyCodeTeam** (Current)
- **Current Members**: `sconklin`, `slspencer`, `csett86`, `DSCaskey`, `pmjherman`, `Onetchou`
- **Current Permission**: "pull" access
- **Proposed Permission**: "write" access
- **Responsibilities**: Code development and review

### **SeamlySigners Team** (To be created)
- **Members**: `sconklin`, `slspencer`, `csett86`, `DSCaskey`, `Onetchou` (all current admin users)
- **Permission**: "maintain" access
- **Responsibilities**: 
  - Approve signing workflows
  - Modify signing workflow files
  - Monitor signing operations
  - Handle signing failures
- **Security**: All activities will be audited through comprehensive logging

### **Other Teams**
- **FFi Team**: "pull" access
- **orga**: "pull" access  
- **SeamlyWebTeam**: "pull" access

## Permission Enforcement Strategy

### **CODEOWNERS File**
Create `.github/CODEOWNERS` file to enforce SeamlySigners team approval for signing-related changes:

```
# Signing workflow files require SeamlySigners team approval
.github/workflows/signing/ @FashionFreedom/seamlysigners
.github/workflows/code-signing.yml @FashionFreedom/seamlysigners
.github/workflows/CODE_SIGNING_PLAN.md @FashionFreedom/seamlysigners
```

### **Branch Protection**
- Require PR reviews for develop branch
- Require maintainer approval for merges
- Prevent direct pushes to develop
- Only maintainers may create tags

## Workflow Dependencies

- Signing workflow depends on Windows build workflow completing successfully
- Waits for Windows build artifacts to be available before starting
- Downloads unsigned artifacts from Windows build workflow
- Replaces current publish job for Windows releases

## Implementation Checklist

### Phase 1: Setup
- [ ] Move plan document to `.github/workflows/CODE_SIGNING_PLAN.md`
- [ ] Create `.github/workflows/signing/` directory
- [ ] Move signing files to signing directory
- [ ] Modify signing script to accept parameters
- [ ] Create Google Cloud service account with minimal permissions
- [ ] Set up GitHub secrets for authentication
- [ ] Create code-signing.yml workflow file
- [ ] Create SeamlySigners team with appropriate permissions
- [ ] Create CODEOWNERS file for permission enforcement
- [ ] Update team permissions (SeamlyBuildTeam to "maintain", SeamlyCodeTeam to "write")

### Phase 2: Permission Reorganization
- [ ] Add missing users to appropriate teams
- [ ] Remove all direct repository access
- [ ] Verify team-based permissions work correctly
- [ ] Test CODEOWNERS enforcement

### Phase 3: Testing
- [ ] Test signing script locally
- [ ] Test workflow with manual dispatch using test_mode=true
- [ ] Validate audit logging
- [ ] Test error conditions
- [ ] Verify test artifact naming

### Phase 4: Integration
- [ ] Modify Windows build workflow to upload unsigned artifacts
- [ ] Integrate with existing CI workflow
- [ ] Set up workflow approval requirements
- [ ] Configure branch protection rules
- [ ] Test scheduled release integration
- [ ] Document procedures for maintainers

### Phase 5: Production
- [ ] Deploy to production
- [ ] Monitor signing success rates
- [ ] Review audit logs regularly
- [ ] Establish key rotation schedule
- [ ] Create incident response procedures

---

*This plan provides maximum security for your open repository while maintaining operational efficiency through proper approval workflows and comprehensive audit logging.* 