# Seamly2D GitHub Workflows Overview

## CI (`ci.yml`)

Triggers: pull requests, pushes to `develop` and `feat-*`, Monday 01:30 UTC
schedule, and manual dispatch.

- **Pull requests:** Linux build/unit tests, then one unsigned Windows x64
  integration-test installer via `pr-integration-build.yml`. All PR authors,
  including Weblate, follow the same build path.
- **Push, schedule, manual:** existing Linux AppImage, macOS, and Windows x64 / 
  ARM64 builds. The existing signing steps run when their secrets are available.
- **Publish:** scheduled/manual runs on `develop` create the normal weekly
  release after all platform builds succeed. Pushes upload build artifacts;
  they do not create GitHub pre-releases.
- **Documentation:** push, schedule, and manual runs deploy Doxygen to `gh-pages`.
- **Permissions:** default `contents: read`; only release publishing and
  documentation deployment receive `contents: write`.

## PR integration-test build (`pr-integration-build.yml`)

This reusable workflow is called by CI after both version creation and Linux
unit tests succeed. It cannot be run independently. It builds the same PR merge
revision used by the Linux tests, with the existing Windows x64 build/NSIS steps.
It receives no inherited secrets and does not sign, publish, tag, or approve a PR.

One Actions artifact contains the installer, samples, test instructions, source
revision details, and an installer SHA256 checksum. Its name identifies the PR,
head commit, run, and attempt. Retention is 14 days. Download from the CI run's
Artifacts section or the Windows job summary (GitHub sign-in required).

See [PR_INTEGRATION_TESTING.md](PR_INTEGRATION_TESTING.md) for the complete test
procedure, local installation instructions for these workflow changes, and
required branch-protection setup. The PR template records testing evidence;
required reviews enforce the human approval gate.

## Weblate (`auto-merge-weblate.yml`)

For eligible same-repository translation PRs, verifies the changed paths and
enables auto-merge. It no longer auto-approves PRs. Required checks and human
reviews must be enforced with repository merge rules. This workflow uses
`pull_request_target` only for PR metadata/API operations and does not check out
or execute PR code.

## Signing and platform coverage

PR installers are unsigned and use the normal installer, which may replace an
existing installation. Use a disposable VM or test computer. PR workflow edits
do not change the normal release signing implementation.

Windows x64 integration testing reduces PR build cost but does not establish
macOS, Linux, or Windows ARM64 compatibility. Contributors must test affected
platforms for platform-specific changes.

Existing signing references: [CODE_SIGNING.md](CODE_SIGNING.md) and
[signing/SECRETS_SETUP.md](signing/SECRETS_SETUP.md).
