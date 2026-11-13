# Property browser integration tests — issue #1843

This QtTest target uses the production application sources and its MainWindow
connections. It is built with the other tests and runs through `make check`.
It intentionally has a separate executable and object directory; it does not
modify or relink the production executable.

The fixture is copied to a temporary directory before modification. Application
settings and backups are also temporary, so tests do not use personal settings
or modify sample patterns.

## Run

Configure and build the top-level `Seamly2D.pro` with tests enabled (the default).
From the build directory on headless Linux:

```sh
QT_QPA_PLATFORM=offscreen make -C src/test/PropertyBrowserTest check
```

Alternatively, use `xvfb-run -a` as in the project's Linux CI workflow.
No Python runner, local absolute paths or additional dependencies are required.

## Coverage

- Length and angle for cm, mm and inch patterns, axis-aligned and diagonal lines,
  and coincident endpoints.
- Agreement with the actual hover tooltip.
- Read-only editor widgets and clearing/reselecting a line.
- Changes to endpoint length, angle and selected endpoint, including undo/redo.
- Clearing selection while a property refresh is queued.
- Point coordinates, curve options and arc radius after edits and undo.
- Saving, closing and reopening a pattern.
- Existing Spanish translations of the new field labels.

## Validation baseline

Before adding this test target, the existing suite was compared against a clean
download of upstream commit `5ba8d468000838466b64674e654caa4db83464cf`.
Both versions compiled independently on Linux x86_64 with Qt 6.12.0, GCC 16.2.1
and release configuration, using identical external dependencies.

Both runs reported **237,175 passed, 1 failed and 1 skipped**. The same failure,
`TST_VAbstractPiece::EquidistantRemoveLoop(Issue 646.)`, returned 139 points instead
of the expected 153, at `src/libs/vtest/abstracttest.cpp:93`. The clean source was
verified unchanged after testing.

That failure was not introduced by this feature in the tested environment.
The complete suite is not claimed to pass, and Windows/macOS validation remains
for CI and reviewers. This baseline does not justify disabling the failing test.

With this target included, the local suite reports **237,199 passed, 1 failed
and 1 skipped**. The new target contributes **24 passing results** (22 functional
cases plus initialization and cleanup).

As a regression check, linking the new tests with the unchanged upstream
MainWindow object reproduces the stale value after undoing an endpoint change:
`editsAndUndoRedo` reads `7 cm` instead of `12.5 cm`. It passes with the queued
property refresh in this change.
