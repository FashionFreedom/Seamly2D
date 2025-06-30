# GitHub Environments Setup

## Code Signing Approval Environment

This directory contains configuration for the `code-signing-approval` environment that enforces manual approval for all code signing operations.

### Manual Setup Instructions

Since GitHub doesn't automatically read environment files from repositories, you need to manually create the environment:

1. **Go to Repository Settings**
   - Navigate to your repository on GitHub
   - Click on "Settings" tab

2. **Access Environments**
   - In the left sidebar, click on "Environments"
   - Click "New environment"

3. **Create Environment**
   - **Environment name**: `code-signing-approval`
   - Click "Configure environment"

4. **Configure Protection Rules**
   - **Required reviewers**: Add the `SeamlySigners` team
   - **Wait timer**: 0 minutes (immediate approval)
   - **Deployment branches**: Select "Selected branches" and choose:
     - `main` (or your default branch)
     - `develop` (if you have a develop branch)

5. **Save Environment**
   - Click "Save protection rules"

### Environment Configuration

The environment will:
- Require approval from the SeamlySigners team for all signing operations
- Only allow deployments from protected branches
- Provide audit logging of all approvals and deployments

### Integration with Workflow

The `code-signing.yml` workflow references this environment to ensure all signing operations require manual approval before proceeding.

### Security Benefits

- **Manual Approval**: No automatic signing without human review
- **Team-Based Control**: Only SeamlySigners team can approve
- **Audit Trail**: All approvals are logged with timestamps and approver information
- **Branch Protection**: Only allows signing from approved branches 