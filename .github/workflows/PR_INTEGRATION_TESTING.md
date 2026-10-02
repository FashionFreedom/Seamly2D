# PR integration testing

## Before merge

For every PR, CI runs Linux unit tests and then calls
`pr-integration-build.yml` to build one unsigned Windows x64 installer. The
installer is an Actions artifact, not a GitHub Release or pre-release.
The Linux tests and installer use GitHub's same PR merge revision so testing
covers the proposed changes combined with the target branch.

The Windows build follows the existing CI qmake/nmake, windeployqt-prepared
binary directories, and NSIS packaging steps. The application version remains
numeric. PR identity is in the artifact name and BUILD-INFO.txt, not the About
dialog. There is no signing, release publishing, or inherited repository secret
in the reusable PR workflow. It uses a GitHub-hosted runner and read-only token.
Fork PRs may need a maintainer to approve the workflow run before it starts.

## Download the test build

1. Open the PR's Checks tab and follow the CI run, or open Actions > CI.
2. Wait for `Linux: Run unit tests` and the Windows integration build to pass.
3. Open the workflow summary and download the artifact named
   `Seamly2D-PR-<number>-<head SHA>-windows-x64-run-<run ID>-attempt-<attempt>`.
   The Windows job summary also includes a download link. GitHub sign-in and
   repository read access are required; artifacts expire after 14 days.
4. Extract the ZIP. It contains `Seamly2D-PR-test-installer.exe`,
   `BUILD-INFO.txt`, `SHA256SUMS.txt`, `TESTING.md`, and sample files.
5. Use a disposable Windows VM or separate test computer. This uses the normal
   NSIS installer and may replace an existing Seamly installation or share
   settings. It does not install as an isolated PR-specific application.
6. Test copies of `samples/patterns` with their associated measurement files.
   Keep source patterns and production work separate from the test copies.

Windows may warn because this is an unsigned build. Review the source changes
and workflow before running it. A checksum identifies the downloaded installer;
it does not certify that the code is safe or correct.

## Record the integration result

Post a PR comment containing:

- Tester and date; Windows edition, version, and architecture.
- PR head SHA, built PR merge SHA, version, run URL, and attempt from BUILD-INFO.txt.
- Sample filenames, measurement files used, and the steps performed.
- Screenshots showing the corrected behavior in the running application.
- Results for the workflow below, unexpected behavior, and reproduction steps.
- An explicit PASS or FAIL. Explain any not-applicable step rather than silently
  skipping it. A new commit or base update requires a fresh build and review.

Exercise the reported fix and the surrounding CAD workflow:

- Open existing sample patterns; modify dimensions or formulas.
- Create and edit points, lines, curves, and arcs.
- Test mirror, move, rotate, and true darts where applicable.
- Create a piece and edit existing pieces.
- Generate a layout and export it to relevant formats.
- Save under a new filename, close, reopen, and compare geometry and pieces.
- Confirm that formulas, measurements, and dependencies persist correctly and
  that there are no unexpected geometry changes, crashes, or lost work.
- Test SeamlyMe if the PR changes measurement handling or shared code.

A green build or checked box is not proof of successful human testing. A
maintainer must inspect the evidence and approve the current PR revision.
Windows testing alone does not validate platform-specific behavior on macOS,
Linux, or Windows ARM64. For changes affecting those platforms, build and test
on the affected platforms before approval.

## Configure GitHub merge protection (one-time maintainer setup)

YAML cannot configure repository merge rules. In Settings > Rules > Rulesets
(or Settings > Branches > Branch protection rules), protect `develop` and any
other merge target:

1. Require a pull request and at least one approving review.
2. Dismiss stale approvals after new commits; require approval of the most
   recent reviewable push by someone other than its author.
3. Require status checks. After this workflow has run successfully, select
   `Linux: Run unit tests` and the Windows integration build check. The latter
   may appear as `PR Integration Test Build / Windows x64: Integration Test Build`;
   use the exact name GitHub offers for the completed run, not a guessed job ID.
4. Require the branch to be up to date before merging, to test against current
   target-branch code. Update the PR branch and retest if the base has changed.
5. Apply rules to administrators where appropriate and limit bypass permissions.

The PR checklist documents evidence; it does not itself block merging. The
required approving review is the human integration gate. Reviewers must
withhold approval until the current artifact has passed testing.

The existing Weblate workflow still enables auto-merge for eligible translation
PRs, but no longer submits an automatic approving review. With these merge rules,
it waits for required checks and a human approval. Without branch protection,
auto-merge does not establish an integration-testing gate.

## Install these changes in the local repository

1. In PowerShell, start from the intended base branch in
   `C:\Users\susan\Projects\seamly2d`. Run `git status` and commit or stash
   existing work before continuing.
2. Create a branch: `git switch -c ci/pr-integration-build`.
3. Extract the supplied ZIP into a temporary folder. Copy its `.github` folder
   into the repository root, merging folders and replacing the changed files.
4. Review `git diff --stat` and `git diff`.
5. Stage only the six files below, commit, and push:

```powershell
git add .github/workflows/ci.yml .github/workflows/pr-integration-build.yml .github/workflows/PR_INTEGRATION_TESTING.md .github/workflows/README_WORKFLOWS.md .github/workflows/auto-merge-weblate.yml .github/PULL_REQUEST_TEMPLATE.md
git commit -m "ci: build PR integration test installer after Linux tests"
git push -u origin ci/pr-integration-build
```

6. Open a PR targeting `develop`. Both workflow files are included in the PR,
   so its own CI can exercise the new workflow before merge.
7. Download and test the installer; then configure the required checks using
   their actual completed check names and arrange a maintainer review.

## Validation limits

The supplied `.github` archive was enough to reuse the existing build commands,
but did not include the application source, `scripts/version.sh`, or the NSIS
script. The full Windows build and installer must be verified on GitHub. The
first run should confirm the Qt download, dependency deployment, installer
creation, installation, application launch, and sample-file handling.

## References

- Reusable workflows: https://docs.github.com/en/actions/how-tos/reuse-automations/reuse-workflows
- Artifact downloads: https://docs.github.com/en/actions/how-tos/manage-workflow-runs/download-workflow-artifacts
- Merge protection: https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches
