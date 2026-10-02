# Workflows

- [CI](ci.yml): Linux tests and Linux/macOS/Windows builds run in parallel on PRs.
- Pushes to `develop`/`feat-*` build artifacts. Monday 01:30 UTC and manual runs on `develop` publish releases.
- macOS and Windows builds are signed when signing secrets are available. Fork PR builds are unsigned.
- [Weblate](auto-merge-weblate.yml): automatic approval/merge; Linux tests only, no artifacts.

For testing, see [PR_INTEGRATION_TESTING.md](PR_INTEGRATION_TESTING.md).
