0

# Seamly2D

Pattern drafting application — parent app of the Seamly family.

- **Author:** slspencer
- **Copyright:** 2026 Seamly2D Project
- **License:** GPL-3.0-or-later

## Architecture

- Qt 6 / C++ (QtWidgets), built with qmake (two toolchains — see Build Notes)
- Apps: `src/app/seamly2d` (pattern drafting), `src/app/seamlyme` (measurements)
- Shared libraries under `src/libs/` (`vlayout`, `vformat`, `vpatterndb`, `ifc`, ...)
- `seamlyLayout/` — daughter layout app (Rust + Qt 6.10/QML), present for reference only; it has its own build (`seamlyLayout/qt_frontend/qd.ps1`) and must stay out of the Seamly2D qmake build. It has its own CLAUDE.md and rules.

## Build Notes

Two toolchains are in use — do not treat the difference as an error:

- **CI toolchain (GitHub runner):** Qt 6.8.3 + MSVC 2022 — a selection of tools available on GitHub's hosted runners; used by the release/CI workflows
- **Local toolchain (developer PC, check builds of current work):** Qt 6.10.1 `msvc2022_64` + VS 18 Community MSVC (`vcvars64.bat`), qmake + jom; release shadow-build in `build/` (gitignored)
- Local debug build: `scripts/sd.ps1` ("seamly2d debug") — auto-detects the newest Qt 6.10.x msvc2022_64 kit under `C:\Qt` and the VS 18 Community MSVC environment, then shadow-builds `CONFIG+=debug` into `seamly2d-build-debug/` (gitignored); the debug exe lands at `seamly2d-build-debug/src/app/seamly2d/bin/seamly2d.exe` with Qt debug DLLs deployed by windeployqt. `-Run` launches it after the build; see the script's `.SYNOPSIS` for details.

## Coding Rules

- **New file naming:** new source files must NOT begin with `v` — begin them with `s` (existing `v*` files keep their names when edited)
- **License headers:** every new file gets a GPLv3-or-later header with copyright 2026 Seamly2D Project and author slspencer (follow the existing header block style, e.g. `src/libs/vformat/svg_generator.cpp`)
- **Documentation:** all new code and every modified function gets a Doxygen-compatible `@brief` (plus `@param`/`@return` where applicable) and inline comments so an intermediate-level programmer can follow the workflow, control flow, and data flow

## Task Tracking

- `PROJECT_PLAN.md` — the current approved implementation plan
- `TODO.md` — tasks with checkbox subtasks; check off subtasks as they are accomplished
- `COMPLETED.md` — when all subtasks of a task are complete, move the task here from `TODO.md`

## Key References

- `status-docs/new-attributes.csv` — SVG `data-*` attribute spec for the SeamlyLayout handoff
- Test pattern: `seamlyLayout/input/richmond-shirt_v1_v061-test.sm2d`
