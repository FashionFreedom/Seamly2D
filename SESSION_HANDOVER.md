
Here's what happened this session (2026-07-17):

**Task 8 — Verification is complete; the Tagged SVG Handoff plan is fully done.** `TODO.md` is empty and all tasks live in `COMPLETED.md`.

**What was done:**

1. **Build** — `vlayout`, `vformat`, `seamly2d` (plus everything else) built clean on branch `run-seamlyLayout` with qmake + jom in `build/` (gitignored). Toolchain actually on this machine: Qt **6.10.1** msvc2022_64 (`C:\Qt\6.10.1`) and MSVC from **VS 18 Community** (`vcvars64.bat`) — not the Qt 6.8.3 / VS 2022 named in CLAUDE.md; neither of those is installed.
2. **Layout Mode handoff verified in the real GUI** — launched the built `seamly2d.exe` with the richmond test pattern, sent Shift+L: `<basename>.pieces.svg` was written beside the pattern and SeamlyLayout (dev build at `seamlyLayout\qt_frontend\build\Debug\SeamlyLayout.exe`) launched detached with the SVG path argument.
3. **Exports verified via CLI** (`--exportOnlyDetails --mfile C:\Users\susan\seamlyLayout\input\2025-06-08-Sue.smis`): tagged SVG with/without `--text2paths`, PDF, PNG, DXF flat + AAMA — all pass structural checks; geometry is byte-identical to the pre-change baseline (only label glyph outlines differ, a font-rendering artifact of the older installed baseline build).
4. **Contract documented** — `status-docs/svg-data-attributes.md`, mirrored to `seamlyLayout/docs/status-docs/svg-data-attributes.md`.

**Next steps:** none pending from the plan. Possible follow-ons noted in the plan: per-notch group splitting, physical removal of the built-in layout UI once SeamlyLayout is fully adopted.
