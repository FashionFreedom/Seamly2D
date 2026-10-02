# Workflows

- [CI](ci.yml): Linux tests and Linux/macOS/Windows builds run in parallel on PRs.
- Pushes to `develop`/`feat-*` build artifacts. Monday 01:30 UTC and manual runs on `develop` publish releases.
- Windows PR builds are unsigned. macOS signing runs when secrets are available.
- [Weblate](auto-merge-weblate.yml): automatic approval/merge; Linux tests only, no artifacts.

For testing, see [PR_INTEGRATION_TESTING.md](PR_INTEGRATION_TESTING.md).
