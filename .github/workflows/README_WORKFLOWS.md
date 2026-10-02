# Seamly2D GitHub Workflows Overview

## CI (`ci.yml`)

Triggers: pull requests, pushes to `develop` and `feat-*`, Monday 01:30 UTC
schedule, and manual dispatch.

- **Pull requests:** Linux unit tests plus the existing Linux AppImage, macOS,
  Windows x64, and Windows ARM64 builds. The Windows matrix waits for Linux tests
  to pass. Linux and macOS packaging retain their existing dependency on version
  creation and may run alongside the tests.
- **PR Windows artifacts:** the existing installer ZIPs additionally contain
  sample files, integration-testing instructions, build details, and a SHA256
  checksum. There is no separate integration-build workflow or duplicate build.
- **Weblate:** Linux tests only, with the original automatic approval/merge
  workflow preserved. No packaging jobs or build artifacts are created for
  Weblate PRs.
- **Push, schedule, manual:** existing multiplatform builds. The normal installer
  ZIP contents and artifact names are preserved for these runs.
- **Signing:** macOS and Windows signing run only for non-PR events when secrets
  are available. PR builds remain unsigned even when submitted from the same
  repository.
- **Publish:** scheduled/manual runs on `develop` create the normal weekly
  release after all platform builds succeed. Pushes upload artifacts; they do
  not create GitHub pre-releases.
- **Documentation:** push, schedule, and manual runs deploy Doxygen to `gh-pages`.
- **Permissions:** default `contents: read`; publishing and documentation jobs
  receive `contents: write`.

## Human integration testing

Download the existing `Seamly2D-windows.zip` artifact for x64 or
`Seamly2D-win-arm64.zip` for ARM64 from the PR's CI run. The Windows job summary
also provides a link. Extract the artifact archive and then the installer ZIP.
Read BUILD-INFO.txt and TESTING.md before testing. GitHub sign-in and repository
read access are required.

See [PR_INTEGRATION_TESTING.md](PR_INTEGRATION_TESTING.md) for test procedures,
branch-protection setup, and instructions for applying these changes. Required
reviews enforce human testing for code changes; eligible Weblate PRs retain
existing automated approval as an exception.

## Weblate (`auto-merge-weblate.yml`)

This workflow is preserved unchanged. For eligible same-repository non-draft
PRs by `weblate`, it checks for `.ts` / `.pro` changes under `share/translations/`,
automatically approves, and enables auto-merge. Required checks still apply;
repository review rules must allow the existing automation to satisfy approval
requirements. This workflow only uses PR metadata/API operations and does not
check out or execute PR code.

## Signing references

[CODE_SIGNING.md](CODE_SIGNING.md) and
[signing/SECRETS_SETUP.md](signing/SECRETS_SETUP.md).
