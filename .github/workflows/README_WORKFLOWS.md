# Workflows

- [CI](ci.yml): Linux tests and Linux/macOS/Windows builds run in parallel on PRs.
- Pushes to `develop`/`feat-*` build artifacts. Monday 01:30 UTC and manual runs on `develop` publish releases.
- macOS and Windows builds are signed when signing secrets are available. Fork PR builds are unsigned.
- The Windows NSIS installer stages both executables and deploys their Qt dependencies into one shared runtime. CI smoke-tests the x64 staging tree without Qt on `PATH`; ARM64 is cross-compiled.
- The Linux AppImage includes both executables in one runtime. On macOS, the Seamly2D bundle contains SeamlyMe and a shared Qt runtime; the separately distributed SeamlyMe app remains standalone.
- [Weblate](auto-merge-weblate.yml): automatic approval/merge; Linux tests only, no artifacts.

For integration testing, see the [PR template](../PULL_REQUEST_TEMPLATE.md).
