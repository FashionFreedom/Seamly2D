# PR integration testing

## Before merge

The existing CI workflow builds Linux AppImage, macOS, and Windows x64 / ARM64
artifacts for non-Weblate PRs, alongside Linux unit tests. Windows packaging
waits for the Linux tests to succeed. The existing Windows installer ZIPs now
include testing instructions, samples, and build details. No separate Windows
integration build is needed.

Tests and builds use GitHub's default PR merge revision: the proposed changes
combined with the target branch. BUILD-INFO.txt records the PR head and base
commits, actual built merge commit, numeric version, architecture, run URL,
attempt, and build time. PR identity is recorded in that file, not in the
application About dialog. Windows and macOS PR signing is explicitly disabled.

Weblate PRs retain Linux tests, no packaging artifacts, and the existing
automated translation approval/merge process.

## Download and test

1. Open the PR's Checks tab and follow the CI run, or open Actions > CI.
2. Confirm the Linux unit tests and the relevant platform build passed.
3. Download `Seamly2D-windows.zip` for Windows x64 or `Seamly2D-win-arm64.zip`
   for Windows ARM64 from that PR run. The corresponding Windows job summary
   also includes a download link. GitHub sign-in and repository read access
   are required. Existing artifact retention settings apply.
4. Extract the Actions artifact ZIP and then the installer ZIP within it.
   The inner ZIP contains `Seamly2D-installer.exe`, `BUILD-INFO.txt`,
   `SHA256SUMS.txt`, `TESTING.md`, and `samples`.
5. Use a disposable VM or separate test computer. The normal NSIS installer may
   replace an existing installation or share application settings; it is not a
   side-by-side PR installer. Use the installer matching your CPU architecture.
6. Test copies of `samples/patterns` with their associated measurement files.
   Keep source patterns and production work separate from test copies.

Unsigned installers may show Windows security warnings. Review the changes and
workflow before running them. The checksum identifies the installer bytes;
it does not certify correctness. For platform-specific changes, also exercise
that platform using the Linux and macOS artifacts or a local build.

## Record the integration result

Post a PR comment containing:

- Tester/date and OS edition, version, and architecture.
- Build details from BUILD-INFO.txt, including PR head SHA and built merge SHA.
- Sample and measurement filenames used, test steps, and screenshots showing
  the corrected behavior in the running application.
- An explicit PASS or FAIL, unexpected behavior, and reproduction steps.
- Reasons for any not-applicable steps.

Exercise the reported fix and the surrounding CAD workflow:

- Open existing sample patterns; modify dimensions or formulas.
- Create and edit points, lines, curves, and arcs.
- Test mirror, move, rotate, and true darts where applicable.
- Create a piece and edit existing pieces.
- Generate a layout and export it to relevant formats.
- Save under a new filename, close, reopen, and compare geometry and pieces.
- Verify formulas, measurements, and dependencies persist correctly, without
  unexpected geometry changes, crashes, or lost work.
- Test SeamlyMe when measurement handling or shared code changes.

A successful build or checked box does not establish that human testing passed.
For code changes, a maintainer must review testing evidence for the current
revision before approval. New commits or base updates require fresh testing.
Eligible Weblate translation PRs retain their automated approval exception.

## Configure merge protection (one-time maintainer setup)

YAML cannot configure repository merge rules. In Settings > Rules > Rulesets
(or Settings > Branches > Branch protection rules), protect `develop` and any
other merge target:

1. Require a pull request and at least one approving review.
2. Dismiss stale approvals after new commits; require approval of the most
   recent reviewable push by someone other than its author.
3. After CI runs successfully, select the existing Linux unit test, Linux
   AppImage, macOS, and Windows matrix checks as required checks. Use the actual
   check names GitHub offers. Remove any required check for the deleted
   `PR Integration Test Build` workflow, if previously configured.
4. Require the branch to be up to date before merging, so testing covers current
   target-branch code. Update the branch and retest after base changes.
5. Apply rules to administrators where appropriate and limit bypass permissions.

The checklist documents evidence; required reviews enforce the human gate for
code changes. The Weblate packaging jobs are explicitly skipped, preserving
its no-artifact process. Do not replace these job-level exclusions with a
workflow-level path filter that would leave required checks pending.

The original Weblate workflow approves only eligible same-repository,
non-draft PRs by `weblate` after checking that all changed paths are under
`share/translations/` and end with `.ts` or `.pro`. Its approval must be accepted
by repository rules for automatic merging to remain available. Additional
code-owner or review requirements may still block it. The `.pro` allowance is
part of the original configuration, not a guarantee of translation-only content.

## Apply the reviewer revision locally

1. In `C:\Users\susan\Projects\seamly2d`, run `git status` and preserve any
   unrelated work before switching branches.
2. Switch to the existing branch: `git switch improve-pr-process`.
3. Extract the supplied ZIP into a temporary folder. Copy its `.github` folder
   into the repository root, merging folders and replacing changed files.
4. Delete the previous `.github/workflows/pr-integration-build.yml` locally:
   it was removed from this version of the archive, but copying a ZIP does not
   delete files already present in your repository. If tracked, use
   `git rm .github/workflows/pr-integration-build.yml`.
5. Review `git diff --stat` and `git diff`, then stage the revised files:

```powershell
git add .github/workflows/ci.yml .github/workflows/PR_INTEGRATION_TESTING.md .github/workflows/README_WORKFLOWS.md .github/PULL_REQUEST_TEMPLATE.md
git commit -m "ci: reuse Windows PR artifacts and restore multiplatform builds"
git push origin improve-pr-process
```

`git rm` stages the removed workflow. If it was never tracked, remove it through
Explorer instead. The existing PR updates when you push this branch.

## Validation limits

The uploaded `.github` archive supplies the existing build commands, but does
not contain application source, `scripts/version.sh`, or the NSIS script.
Static validation covers workflow structure and conditions. GitHub CI must
verify full builds and installer behavior. Test both the PR run and normal
non-PR runs; the Windows job explicitly handles skipped Linux tests on non-PR
runs so scheduled/manual builds still execute.

## References

- Artifacts: https://docs.github.com/en/actions/concepts/workflows-and-actions/workflow-artifacts
- Merge protection: https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-protected-branches/about-protected-branches
