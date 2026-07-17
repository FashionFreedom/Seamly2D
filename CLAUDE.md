0

# Seamly2D

Pattern drafting application — parent app of the Seamly family.

- **Author:** slspencer
- **Copyright:** 2026 Seamly2D Project
- **License:** GPL-3.0-or-later

## Architecture

- Qt 6.8.3 / C++ (QtWidgets), built with qmake and MSVC 2022 on Windows
- Apps: `src/app/seamly2d` (pattern drafting), `src/app/seamlyme` (measurements)
- Shared libraries under `src/libs/` (`vlayout`, `vformat`, `vpatterndb`, `ifc`, ...)
- `seamlyLayout/` — daughter layout app (Rust + Qt 6.10/QML), present for reference only; it has its own build (`seamlyLayout/qt_frontend/qd.ps1`) and must stay out of the Seamly2D qmake build. It has its own CLAUDE.md and rules.

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
