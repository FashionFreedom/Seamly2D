# PR testing

Download your platform's artifact from the PR's CI run. For Windows, extract both ZIPs and run the installer. Use a test computer/VM and copies of sample files—the Windows installer may replace your installation.

Before approval:

- Verify the fix; capture screenshots.
- Edit patterns, points, lines, curves, and arcs.
- Test mirror/move/rotate/true darts where applicable.
- Create/edit pieces; generate/export layouts.
- Save, close, reopen; check geometry and dependencies.

Post PASS/FAIL, OS/version, build/commit, sample files, screenshots, and any failures or N/A steps. Retest after changes.

Require CI checks and reviewer approval through branch protection. Eligible Weblate PRs retain automatic approval/merge.
