## Change and reason

Describe the problem, resulting behavior, and related issue.

## Automated validation

- [ ] Linux unit tests passed.
- [ ] Linux AppImage, macOS, and Windows matrix builds passed.

## Integration testing (before approval)

Rebase on current `develop`, build locally, and test with copies of the sample files:

- Patterns: `src/app/share/samples/patterns`
- Measurements: `src/app/share/samples/measurements`

Record results in a PR comment if testing happens after opening this PR.

- [ ] Built the PR locally and launched it.
- [ ] Opened and modified sample patterns and measurement files.
- [ ] Created/edited points, lines, curves, and arcs.
- [ ] Tested mirror, move, rotate, and true darts (explain any N/A).
- [ ] Created and edited pieces.
- [ ] Generated and exported layouts.
- [ ] Saved, closed, reopened, and verified geometry, formulas, and dependencies.
- [ ] Observed no data loss, unexpected geometry changes, or crashes.
- [ ] Attached screenshots of the corrected behavior in the running application.
- [ ] Tested other affected platforms and SeamlyMe where applicable.

Tester/date:

OS/version/architecture:

Tested commit SHA / version:

Sample files, steps, screenshots, and findings:

Result: PASS / FAIL / NOT YET TESTED

Not-applicable steps and reasons:

New commits or target-branch updates require a new build and integration review.
For code changes, a maintainer approves only after reviewing testing evidence
for the current commit. Eligible Weblate translation PRs retain their existing
automated approval/merge process and are exempt from human integration review.
