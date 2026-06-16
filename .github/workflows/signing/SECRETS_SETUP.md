# GitHub Secrets Setup Guide

## Required Secrets for Code Signing Workflow

The following secrets must be configured in your GitHub repository for the code signing workflow to function properly.

### How to Add Secrets

1. **Go to Repository Settings**
   - Navigate to your repository on GitHub
   - Click on "Settings" tab

2. **Access Secrets and Variables**
   - In the left sidebar, click on "Secrets and variables"
   - Click on "Actions"

3. **Add Each Secret**
   - Click "New repository secret"
   - Enter the secret name and value
   - Click "Add secret"

## Required Secrets

### 1. SEAMLY_SIGNING_SA_KEY
**Description**: Google Cloud service account JSON key file content
**Type**: Repository secret
**Value**: The entire content of your service account JSON key file

**How to get this**:
1. Go to Google Cloud Console → IAM & Admin → Service Accounts
2. Find your service account: `seamly-signing-sa@seamly-code-signing.iam.gserviceaccount.com`
3. Click on the service account
4. Go to "Keys" tab
5. Click "Add Key" → "Create new key"
6. Choose "JSON" format
7. Download the file and copy its entire content

**Example format**:
```json
{
  "type": "service_account",
  "project_id": "seamly-code-signing",
  "private_key_id": "abc123...",
  "private_key": "-----BEGIN PRIVATE KEY-----\n...\n-----END PRIVATE KEY-----\n",
  "client_email": "seamly-signing-sa@seamly-code-signing.iam.gserviceaccount.com",
  "client_id": "123456789",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
  "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/seamly-signing-sa%40seamly-code-signing.iam.gserviceaccount.com"
}
```

### 2. SEAMLY_SIGNING_PROJECT_ID
**Description**: Google Cloud project ID
**Type**: Repository secret
**Value**: `seamly-code-signing`

### 3. SEAMLY_SIGNING_LOCATION
**Description**: Google Cloud KMS location
**Type**: Repository secret
**Value**: `us-central1`

### 4. SEAMLY_SIGNING_KEYRING_NAME
**Description**: Google Cloud KMS keyring name
**Type**: Repository secret
**Value**: `Seamly-Code-Signing-Keyring-2`

### 5. SEAMLY_SIGNING_KEY_NAME
**Description**: Google Cloud KMS key name
**Type**: Repository secret
**Value**: `EVCodeSigningKey1`

## Secret Configuration Summary

| Secret Name | Value | Description |
|-------------|-------|-------------|
| `SEAMLY_SIGNING_SA_KEY` | JSON content | Service account key file |
| `SEAMLY_SIGNING_PROJECT_ID` | `seamly-code-signing` | Google Cloud project ID |
| `SEAMLY_SIGNING_LOCATION` | `us-central1` | KMS location |
| `SEAMLY_SIGNING_KEYRING_NAME` | `Seamly-Code-Signing-Keyring-2` | KMS keyring name |
| `SEAMLY_SIGNING_KEY_NAME` | `EVCodeSigningKey1` | KMS key name |

## Security Best Practices

### 1. Secret Rotation
- **Service Account Key**: Rotate every 90 days
- **Other Secrets**: Rotate if compromised or personnel changes

### 2. Access Control
- Only repository administrators should have access to secrets
- Use team-based access control for secret management
- Audit secret access regularly

### 3. Monitoring
- Monitor service account usage in Google Cloud Console
- Set up alerts for unusual signing activity
- Review GitHub Actions logs for secret usage

### 4. Backup and Recovery
- Keep secure backups of service account keys
- Document secret rotation procedures
- Have recovery procedures for lost secrets

## Verification Steps

After adding all secrets:

1. **Test Service Account Authentication**:
   ```bash
   # In a test workflow or locally
   echo '${{ secrets.SEAMLY_SIGNING_SA_KEY }}' > /tmp/key.json
   gcloud auth activate-service-account --key-file=/tmp/key.json
   gcloud config set project ${{ secrets.SEAMLY_SIGNING_PROJECT_ID }}
   gcloud kms keys list --keyring=${{ secrets.SEAMLY_SIGNING_KEYRING_NAME }} --location=${{ secrets.SEAMLY_SIGNING_LOCATION }}
   ```

2. **Verify KMS Access**:
   - Confirm the service account can list keys
   - Verify the key exists and is enabled
   - Test signing permissions

3. **Check Workflow Integration**:
   - Run a test workflow dispatch
   - Verify secrets are accessible in workflow
   - Check for any permission errors

## Troubleshooting

### Common Issues

1. **"Permission denied" errors**:
   - Verify service account has correct roles
   - Check project ID and location values
   - Ensure keyring and key names are correct

2. **"Secret not found" errors**:
   - Verify secret names match exactly (case-sensitive)
   - Check that secrets are added to the correct repository
   - Ensure workflow has access to repository secrets

3. **"Invalid JSON" errors**:
   - Verify service account key is complete JSON
   - Check for extra characters or formatting issues
   - Ensure the entire file content is copied

### Support

If you encounter issues:
1. Check GitHub Actions logs for detailed error messages
2. Verify Google Cloud IAM permissions
3. Test service account authentication locally
4. Review this setup guide for any missed steps 