# Secrets Quick Reference

## Required GitHub Secrets

| Secret Name | Value |
|-------------|-------|
| `SEAMLY_SIGNING_SA_KEY` | [Service account JSON key content] |
| `SEAMLY_SIGNING_PROJECT_ID` | `seamly-code-signing` |
| `SEAMLY_SIGNING_LOCATION` | `us-central1` |
| `SEAMLY_SIGNING_KEYRING_NAME` | `Seamly-Code-Signing-Keyring-2` |
| `SEAMLY_SIGNING_KEY_NAME` | `EVCodeSigningKey1` |

## Setup Steps

1. **Go to**: Repository Settings → Secrets and variables → Actions
2. **Click**: "New repository secret"
3. **Add each secret** with the exact name and value above
4. **For SEAMLY_SIGNING_SA_KEY**: Copy entire JSON content from service account key file

## Service Account Details

- **Email**: `seamly-signing-sa@seamly-code-signing.iam.gserviceaccount.com`
- **Required Roles**: 
  - Cloud KMS CryptoKey Signer/Verifier
  - Cloud KMS Viewer

## Verification Command

```bash
gcloud kms keys list --keyring=Seamly-Code-Signing-Keyring-2 --location=us-central1 --project=seamly-code-signing
```

---

**For detailed instructions, see**: `SECRETS_SETUP.md` 