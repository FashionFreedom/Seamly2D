# Code Signing Implementation Plan

## Security Model (Recommended)

### **1. Workflow Approval Required**
- Manual approval for ALL signing operations (both scheduled and manual)
- Only trusted maintainers can approve signing workflows
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

### **4. Tag Restrictions**
- Limit tag creation to maintainers only
- Use signed tags for releases
- Audit all tags before they trigger workflows

## Implementation Plan

### 1. **File Organization**

**Move files to appropriate locations**:
- `jsign-image.sh` → `scripts/sign-windows-executable.sh`
- `jsign.jar` → `scripts/jsign.jar`
- `codesign-chain.pem` → `scripts/codesign-chain.pem`

**Script modifications needed**:
- Accept executable path as parameter
- Improve error handling (currently TODO comment)
- Add comprehensive logging and validation
- Make script more robust for CI environment

### 2. **Workflow Structure**

**File**: `.github/workflows/code-signing.yml`

**Required GitHub Secrets**:
- `SEAMLY_SIGNING_PROJECT_ID` (Google Cloud project ID)
- `SEAMLY_SIGNING_LOCATION` (KMS location, e.g., "us-central1")
- `SEAMLY_SIGNING_KEYRING_NAME` (KMS keyring name)
- `SEAMLY_SIGNING_KEY_NAME` (KMS key name)
- `GOOGLE_CLOUD_SERVICE_ACCOUNT_KEY` (Service account JSON key)

**Environment Variables** (set in workflow):
- `SEAMLY_SIGNING_PROJECT_ID`
- `SEAMLY_SIGNING_LOCATION` 
- `SEAMLY_SIGNING_KEYRING_NAME`
- `SEAMLY_SIGNING_KEY_NAME`

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
- Log all validation steps

**2. Download Job**:
- Download unsigned Windows artifacts from previous CI run
- Verify artifact integrity and checksums
- Extract executables from installer if needed
- Log download operations

**3. Signing Job**:
- Setup Google Cloud authentication
- Copy signing files to working directory
- Sign individual executables:
  - `seamly2d.exe`
  - `seamlyme.exe`
- Rebuild installer with signed executables
- Sign the final installer
- Upload signed artifacts
- Comprehensive logging of all operations

### 5. **Signing Process Details**

**Files to Sign**:
1. Individual executables (using existing script)
2. NSIS installer (using existing script)

**Script Usage**:
```bash
./scripts/sign-windows-executable.sh path/to/executable.exe
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

### 7. **Integration Strategy**

**Separate Workflow Approach**:
- Main CI builds and uploads unsigned artifacts
- Code signing workflow runs independently with approval
- Signed artifacts replace unsigned ones in releases
- Clear separation of concerns

### 8. **Triggering Strategy**

**Manual Trigger**:
```bash
gh workflow run code-signing.yml -f commit_sha=abc123 -f version_number=2024.1.1.1200
```

**Scheduled Integration**:
- Weekly scheduled release triggers signing workflow
- Manual approval required before signing proceeds
- Repository dispatch events for integration

### 9. **Artifact Management**

**Before Signing**:
- Download unsigned artifacts from CI run
- Verify checksums and commit association
- Extract executables if needed
- Log all artifact operations

**After Signing**:
- Upload signed artifacts with clear naming
- Replace unsigned artifacts in releases
- Maintain complete audit trail
- Verify signing was successful

### 10. **Error Handling and Monitoring**

**Validation Checks**:
- Verify commit is on develop branch only
- Check user has signing permissions and approval
- Validate artifact integrity
- Verify jsign.jar hash
- Confirm all environment variables are set

**Failure Handling**:
- Clear error messages with actionable information
- Rollback procedures for failed signing
- Comprehensive audit logging
- Notification on failures to maintainers
- Graceful degradation (unsigned artifacts if signing fails)

### 11. **Audit Logging Requirements**

**Log all operations with**:
- Commit SHA being signed
- Approver information (who approved the signing)
- Timestamp of all operations
- Files being signed and their checksums
- Signing tool version and configuration
- Success/failure status
- Error messages and stack traces
- User who triggered the workflow
- Workflow run ID for traceability

### 12. **Testing Strategy**

**Development Phase**:
- Test with manual workflow dispatch
- Use production KMS keyring (as specified)
- Validate signing process end-to-end
- Test error conditions and recovery

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

## Questions for Final Review

1. **Workflow Approval**: Given the open repository nature, do you want to require manual approval for ALL signing operations, or just for manual triggers?

2. **Tag Management**: Who currently has permission to create tags? Should we restrict this to maintainers only?

3. **Scheduled Releases**: Should the weekly scheduled release require manual approval, or is the existing PR review process sufficient protection?

4. **Audit Requirements**: What specific information do you want logged for each signing operation? (commit SHA, approver, timestamp, files signed, etc.)

5. **Fallback Procedures**: What should happen if signing fails during a scheduled release? Should it fail the entire release or continue with unsigned artifacts?

6. **Emergency Signing**: Do you need a way to sign specific commits outside the normal workflow (e.g., security fixes)?

7. **Repository Teams**: Can you share the team structure so I can suggest appropriate permission levels?

8. **Service Account Security**: Are you comfortable with the service account key being stored as a GitHub secret, or do you want additional encryption/rotation procedures?

## Implementation Checklist

### Phase 1: Setup
- [ ] Move signing files to scripts directory
- [ ] Modify signing script to accept parameters
- [ ] Create Google Cloud service account with minimal permissions
- [ ] Set up GitHub secrets for authentication
- [ ] Create code-signing.yml workflow file

### Phase 2: Testing
- [ ] Test signing script locally
- [ ] Test workflow with manual dispatch
- [ ] Validate audit logging
- [ ] Test error conditions
- [ ] Verify artifact replacement

### Phase 3: Integration
- [ ] Integrate with existing CI workflow
- [ ] Set up workflow approval requirements
- [ ] Configure branch protection rules
- [ ] Test scheduled release integration
- [ ] Document procedures for maintainers

### Phase 4: Production
- [ ] Deploy to production
- [ ] Monitor signing success rates
- [ ] Review audit logs regularly
- [ ] Establish key rotation schedule
- [ ] Create incident response procedures

---

*This plan provides maximum security for your open repository while maintaining operational efficiency through proper approval workflows and comprehensive audit logging.* 