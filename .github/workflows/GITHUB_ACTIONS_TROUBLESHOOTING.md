# GitHub Actions Troubleshooting Guide

## Issue: Workflows Not Appearing on Topic Branch

If GitHub Actions workflows are not appearing on your topic branch, try these solutions:

## 🔍 Step 1: Check Repository Settings

### Actions Permissions

1. Go to **Repository Settings** → **Actions** → **General**
2. Ensure **"Allow all actions and reusable workflows"** is selected
3. Check that **"Allow GitHub Actions to create and approve pull requests"** is enabled

### Branch Protection

1. Go to **Settings** → **Branches**
2. Check if there are branch protection rules on `code-signing-test`
3. If rules exist, ensure **"Allow force pushes"** and **"Allow deletions"** are enabled for testing

## 🔍 Step 2: Check Workflow File Syntax

### Common Issues

- **YAML syntax errors** (indentation, missing quotes)
- **Invalid workflow structure** (missing jobs, steps)
- **Unsupported actions** or versions

### Test with Simple Workflow

Create a minimal workflow to test:

```yaml
---
name: Minimal Test

on:
  workflow_dispatch:

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - run: echo "Hello World"
```

## 🔍 Step 3: Check Branch Configuration

### Verify Branch Tracking

```bash
git branch -vv
```

Ensure your branch is tracking the correct remote:

```bash
code-signing-test  [origin/code-signing-test] commit-hash
```

### Push to Correct Remote

```bash
git push origin code-signing-test
```

## 🔍 Step 4: Check GitHub Actions Logs

### View Action Logs

1. Go to **Actions** tab
2. Look for any failed workflow runs
3. Check for error messages in the logs

### Common Error Messages

- **"Workflow is not valid"** - Syntax error in workflow file
- **"Branch not found"** - Branch not pushed to remote
- **"Permission denied"** - Repository settings issue

## 🔍 Step 5: Alternative Testing Approaches

### Test on Main Branch

1. Temporarily merge your workflow to `develop` or `main`
2. Test if it appears there
3. If it works, the issue is branch-specific

### Use Pull Request

1. Create a pull request from `code-signing-test` to `develop`
2. Check if workflows appear in the PR
3. Some workflows only show in PR context

### Force Workflow Detection

1. Make a small change to any workflow file
2. Commit and push the change
3. This can trigger GitHub to re-scan workflows

## 🔍 Step 6: Repository-Specific Issues

### Organization Settings

If this is an organization repository:

1. Check organization Actions settings
2. Verify your account has appropriate permissions
3. Check if Actions are enabled for the organization

### Fork vs Original Repository

If working with a fork:

1. Ensure you're pushing to the correct repository
2. Check if Actions are enabled in your fork
3. Verify upstream settings

## 🔍 Step 7: GitHub Service Issues

### Check GitHub Status

1. Visit [GitHub Status](https://www.githubstatus.com/)
2. Check if Actions service is operational
3. Wait if there are known issues

### Clear Browser Cache

1. Clear browser cache and cookies
2. Try incognito/private browsing mode
3. Check from different browser or device

## 🔍 Step 8: Debugging Commands

### Verify Files in Branch

```bash
git ls-tree HEAD .github/workflows/
```

### Check Workflow Syntax

```bash
# If you have yamllint installed
yamllint .github/workflows/*.yml
```

### Verify Remote Branch

```bash
git ls-remote origin code-signing-test

```

## 🔍 Step 9: Contact GitHub Support

If all else fails:

1. Document the issue with screenshots
2. Include repository URL and branch name
3. Contact GitHub Support with detailed information

## 🔍 Quick Fixes to Try

1. **Wait 5-10 minutes** - GitHub Actions can take time to detect new workflows
2. **Refresh the page** - Use Ctrl+F5 or Cmd+Shift+R
3. **Check "All workflows"** - Look in the left sidebar
4. **Try different browser** - Sometimes browser cache issues
5. **Check from mobile** - Different user agent might help

## 🔍 Prevention for Future

1. **Test workflows on main branch first**
2. **Use simple workflows for initial testing**
3. **Check syntax before pushing**
4. **Monitor GitHub Actions status**
5. **Keep workflows simple and well-documented**
