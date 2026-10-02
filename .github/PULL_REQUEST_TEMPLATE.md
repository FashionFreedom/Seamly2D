## Change and reason

Describe the problem, resulting behavior, and related issue.

## Automated validation

- [ ] Linux unit tests passed.
- [ ] Windows x64 PR integration-test artifact built successfully.

## Integration testing (before approval)

Use the artifact and instructions in
[PR_INTEGRATION_TESTING.md](workflows/PR_INTEGRATION_TESTING.md).
Test copies of the samples in `src/app/share/samples/patterns`.
Record results in a PR comment if testing happens after opening this PR.

- [ ] Installed and launched the test build on a test computer or VM.
- [ ] Opened and modified sample patterns.
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

PR head SHA / built PR merge SHA (from BUILD-INFO.txt):

Workflow run URL / run attempt / version:

Sample files, steps, screenshots, and findings:

Result: PASS / FAIL / NOT YET TESTED

Not-applicable steps and reasons:

New commits or target-branch updates require a new build and integration review.
A maintainer approves only after reviewing testing evidence for the current build.
