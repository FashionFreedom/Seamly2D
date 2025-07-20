# CODEOWNERS Configuration

## Overview

The `.github/CODEOWNERS` file enforces that only the **SeamlySigners** team can modify code signing-related files. This provides an additional security layer to prevent unauthorized changes to the signing workflow.

## Protected Files

The following files require approval from the SeamlySigners team:

### Workflow Files
- `.github/workflows/signing/` - All files in the signing directory

### Documentation and Configuration
- `.github/workflows/CODE_SIGNING.md` - Implementation documentation

## How It Works

1. **Automatic Review Requests**: When anyone (including team members) creates a pull request that modifies these files, GitHub automatically requests a review from the SeamlySigners team.

2. **Required Approval**: The pull request cannot be merged until a member of the SeamlySigners team approves it.

3. **Audit Trail**: All approvals are logged with timestamps and approver information.

## Security Benefits

- **Prevents Unauthorized Changes**: Only SeamlySigners team members can approve changes to signing workflows
- **Audit Trail**: Complete record of who approved what changes
- **Team-Based Control**: Leverages GitHub's built-in team permissions
- **Automatic Enforcement**: No manual intervention required

## Team Structure

- **SeamlySigners**: Can approve and modify signing workflows
- **SeamlyBuildTeam**: Can maintain other workflows (not signing)
- **SeamlyCodeTeam**: Code development and review

## Optional Configuration

The commented line at the bottom shows how to extend protection to all workflow files if desired:

```yaml
# .github/workflows/ @FashionFreedom/seamlybuildteam
```

This would require SeamlyBuildTeam approval for all workflow changes, but is currently disabled to avoid being too restrictive.

## Best Practices

1. **Regular Reviews**: SeamlySigners team should regularly review and approve legitimate changes
2. **Documentation**: Keep the CODEOWNERS file updated as new signing-related files are added
3. **Team Communication**: Ensure team members understand their responsibilities
4. **Monitoring**: Regularly check that the protection is working as expected

## Troubleshooting

### "Review Required" Not Appearing
- Verify the SeamlySigners team exists and has the correct permissions
- Check that the file paths in CODEOWNERS match exactly
- Ensure the team name format is correct: `@FashionFreedom/seamlysigners`

### Too Many Review Requests
- Consider adding more specific file patterns
- Review if all protected files actually need this level of protection
- Adjust team membership if needed

### Team Access Issues
- Verify team members have appropriate repository permissions
- Check that the team has "maintain" or "admin" access to the repository
- Ensure team members can actually approve pull requests 