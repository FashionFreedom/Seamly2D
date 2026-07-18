# TODO — Tagged SVG Handoff to SeamlyLayout

See `PROJECT_PLAN.md` for full details. Check off subtasks as they are accomplished; when every subtask of a task is complete, move the task to `COMPLETED.md`.

## Task 13 — Windows .msi installer for seamly2d, seamlyme, and seamlylayout (x64 + arm64)

Build a Windows MSI installer that installs all three apps — `seamly2d`, `seamlyme`, and the SeamlyLayout daughter app — with separate packages (or a multi-arch pipeline) for x64 and arm64.

**Prerequisite:** do not start until seamlyLayout can be launched from seamly2d and passes unit and functional testing.

- [ ] Choose and set up the MSI tooling (e.g. WiX Toolset) and decide packaging layout: one bundled installer vs. per-app MSIs, per-arch builds for x64 and arm64
- [ ] Author the installer definition: install `seamly2d.exe`, `seamlyme.exe`, and the seamlyLayout app plus their Qt runtime DLLs (windeployqt output) and any Rust-side runtime files for seamlyLayout
- [ ] Handle standard installer concerns: Start Menu shortcuts, file associations (`.sm2d`, measurement files), upgrade codes so newer versions upgrade in place, and clean uninstall
- [ ] Produce the arm64 variant: arm64 builds of the apps/Qt runtime (or document a cross-compile/CI story) and an arm64-flagged MSI
- [ ] Wire the MSI build into CI (GitHub runner, Qt 6.8.3 + MSVC 2022) and/or a local script following the `scripts/sd.ps1` pattern
- [ ] Verify: install and uninstall on a clean Windows x64 machine (and arm64 if hardware/VM available); all three apps launch, shortcuts and associations work, upgrade-over-install works
- [ ] Document the build/signing steps (code signing can be a follow-up if no certificate yet) in the repo docs

## Task 14 — Windows installer: prompt for executable and user-data install paths

In the updated Windows installation process (Task 13 installer), prompt the user for two locations instead of hard-coding the defaults:

1. **Executable install path** — where the Seamly executables go. Default `C:\Program Files (x86)\Seamly2D`; must support any drive/filepath (use case: install to `D:\Program Files (x86)\Seamly2D`) and add the chosen executable directory to the system `PATH` automatically.
2. **User data path** — root of the Seamly user data file tree. Default `C:\Users\<user>\seamly2d`; must support any drive/filepath, including cloud-synced drives (use case: `G:\My Drive\seamly2d` so patterns/measurements are accessible while travelling), and register the chosen user data path automatically (add to the system `PATH` per the request; evaluate whether an env var / registry setting / app config is the more appropriate mechanism for a data directory and document the decision).

- [ ] Add an installer UI page (or command-line properties for silent installs) prompting for the executable install directory, prefilled with the `C:\Program Files (x86)\Seamly2D` default, accepting any drive and filepath
- [ ] Add a second installer prompt for the user data root, prefilled with `C:\Users\<user>\seamly2d`, accepting any drive and filepath (including cloud-synced locations like `G:\My Drive\seamly2d`)
- [ ] On install, append the chosen executable directory to the system `PATH` (machine-wide, with proper broadcast so new shells pick it up); remove it on uninstall
- [ ] On install, register the chosen user data path automatically — add to the system `PATH` as requested, and/or persist it where the apps read it (registry/`QSettings`) so seamly2d/seamlyme/seamlyLayout use it as their default data location; remove/clean up on uninstall
- [ ] Make the apps honor the configured user data path on first run (no re-prompting, no fallback to the hard-coded `C:\Users\<user>\seamly2d`)
- [ ] Handle upgrades: preserve both previously chosen paths when a newer MSI upgrades in place
- [ ] Verify use case #1: fresh install to `D:\Program Files (x86)\Seamly2D`, exes run from there, directory present on system `PATH`, uninstall cleans it up
- [ ] Verify use case #2: user data root set to `G:\My Drive\seamly2d`, apps read/write pattern and measurement data there, path registered automatically, uninstall/upgrade behave correctly
- [ ] Document both prompts and the silent-install property equivalents in the installer docs89

## Task 15 — Unify app settings/preferences directories under one `Seamly` folder (Windows)

On Windows the apps currently scatter settings across three locations: seamly2d/seamlyme write under `C:\Users\<user>\AppData\Local\Seamly2D` (organization set from `VER_COMPANYNAME_STR` in `src/libs/vmisc/projectversion.h`, applied in `src/app/seamly2d/core/application_2d.cpp` and `src/app/seamlyme/application_me.cpp`), while seamlyLayout writes under `C:\Users\<user>\AppData\Local\Seamly Systems` (`app.setOrganizationName("Seamly Systems")` in `seamlyLayout/qt_frontend/main.cpp`) and also keeps a `settings\` subdirectory relative to `seamlyLayout.exe`. Combine all of these as `C:\Users\<user>\AppData\Local\Seamly`.

- [ ] Change the organization name to `Seamly` consistently: `VER_COMPANYNAME_STR` (seamly2d, seamlyme, tests) and the hard-coded `"Seamly Systems"` in `seamlyLayout/qt_frontend/main.cpp`, so `QSettings`/`QStandardPaths` (AppConfigLocation, AppDataLocation, AppLocalDataLocation) all resolve under `AppData\Local\Seamly\<app>`
- [ ] Migrate existing user data on first run: copy/move settings from `AppData\Local\Seamly2D` and `AppData\Local\Seamly Systems` into `AppData\Local\Seamly`, without clobbering anything already migrated; follow the existing seamlyLayout migration pattern (`PreferencesModel` legacy-directory rewrites and the Inno Setup migration in `seamlyLayout/packaging/windows/SeamlyLayout.iss`)
- [ ] Eliminate seamlyLayout's exe-relative `settings\` directory as a writable location: packaged defaults (`default_settings.json`, paper/roll presets) stay read-only next to the exe or move into the installer payload, but all user-writable settings/preferences live under `AppData\Local\Seamly`
- [ ] Update the installers/packaging that reference the old paths (`seamlyLayout/packaging/windows/SeamlyLayout.iss`, `build_installer.ps1`, and the Task 13 MSI work) including uninstall cleanup of the new `Seamly` directory
- [ ] Update tests that assert the old locations (e.g. `seamlyLayout/qt_frontend/tests/preferences/PreferencesModelTests.cpp`, `src/test/Seamly2DTest`)
- [ ] Verify on Windows: fresh install writes only under `AppData\Local\Seamly`; upgrade from an install with data in both legacy directories migrates settings and the apps keep their preferences
- [ ] Doxygen briefs + inline comments on all touched functions; document the new layout in the repo docs

## Task 16 — Unify settings directories: macOS build

Apply the Task 15 consolidation to the macOS build, where the organization name maps to `~/Library/Application Support/<org>` and `~/Library/Preferences` plist domains instead of `AppData\Local`.

- [ ] Confirm the org-name change from Task 15 lands the apps in `~/Library/Application Support/Seamly/<app>`; adjust any macOS-specific bundle identifiers / `Info.plist` values that feed the settings domain
- [ ] Migrate existing user data from the legacy `Seamly2D` and `Seamly Systems` locations (Application Support dirs and preferences plists) on first run
- [ ] Remove any exe-relative (app-bundle-relative) writable settings usage in seamlyLayout on macOS; keep packaged defaults read-only inside the bundle resources
- [ ] Update the macOS packaging/CI (dmg/bundle steps) for any path references
- [ ] Verify: fresh install and upgrade-with-legacy-data on macOS; both apps retain preferences after migration

## Task 17 — Unify settings directories: Linux AppImage build

Apply the Task 15 consolidation to the Linux AppImage build, where settings follow the XDG paths (`~/.config/<org>`, `~/.local/share/<org>`).

- [ ] Confirm the org-name change lands the apps in `~/.config/Seamly/` and `~/.local/share/Seamly/`; check any AppImage-specific overrides (`APPDIR`-relative paths, `portable`-mode config)
- [ ] Migrate existing user data from `~/.config/Seamly2D`, `~/.local/share/Seamly2D`, and the `Seamly Systems` equivalents on first run
- [ ] Ensure seamlyLayout inside the AppImage treats its bundled `settings/` defaults as read-only (AppImage mounts are read-only anyway) and writes only to the XDG `Seamly` paths
- [ ] Update the AppImage build/CI scripts for any path references
- [ ] Verify: run the AppImage fresh and over legacy data; preferences persist across runs and migrate correctly

## Task 18 — Unify settings directories: Linux Flatpak build (built at Flathub, not on GitHub)

Apply the Task 15 consolidation to the Flatpak build. Flatpak sandboxes per-app data under `~/.var/app/<flatpak-app-id>/`, and the build is produced from the Flathub manifest repo rather than this repo's CI.

**Decision:** do NOT change the Flatpak way of building — keep the existing Flathub package structure and app id. The apps share files and variables and launch each other via `QProcess::startDetached` (seamly2d → seamlyme in `src/app/seamly2d/mainwindow.cpp`, seamly2d → seamlyLayout in `exportPiecesToSeamlyLayout()`, plus the `.pieces.svg` handoff and shared measurement files), so they must live in the **same sandbox**: all apps ship inside the one existing Flatpak app id, and the unified `Seamly` folder (`~/.var/app/<app-id>/config/Seamly/`) is one shared physical directory inside that sandbox. The folder-name change itself flows entirely from the Task 15 org-name change in the app source.

- [ ] Confirm the org-name change lands all apps' settings under `~/.var/app/<app-id>/config/Seamly/` (and `data/Seamly/`) inside the single shared sandbox, and that cross-app sharing (settings variables, `.pieces.svg` handoff paths, measurement files) works there
- [ ] Confirm the in-sandbox app launches keep working: seamly2d → seamlyme and seamly2d → seamlyLayout via `QProcess::startDetached` resolve to executables inside the same Flatpak prefix (`/app/bin`), not host paths; the `paths/seamlyLayoutApp` setting default must work inside the sandbox
- [ ] Migrate legacy `Seamly2D` / `Seamly Systems` config dirs inside the sandbox on first run (same in-app migration as Task 15 — it must not rely on installer logic, since Flatpak has no installer)
- [ ] Ensure seamlyLayout's packaged defaults are read from the Flatpak app prefix (`/app/...`) read-only, with all writable settings in the sandbox `Seamly` paths
- [ ] Flathub manifest: no build restructuring — add seamlyLayout to the existing package if not yet included (it must ship in the same sandbox for the handoff to work), fix any stale references to the old dir names, and otherwise a routine version bump to the new source release
- [ ] Verify: install the Flatpak fresh and over an existing sandbox with legacy data; preferences migrate and persist; the seamly2d → seamlyLayout handoff works end-to-end inside the sandbox

## Task 19 — Move seamlyLayout code from `/seamlyLayout` to `/src/app/seamlylayout`

Relocate the daughter layout app into the standard app tree alongside `src/app/seamly2d` and `src/app/seamlyme`. It keeps its own build (Rust + Qt 6.10/QML, `qd.ps1`) and must still stay out of the Seamly2D qmake build.

**Decision:** seamlylayout is treated the same as seamlyme — its source is tracked directly in this repo as ordinary files (no submodule). `seamlyLayout/` is currently a nested git repository (own `.git`, untracked by this repo), so the move must absorb it, not `git mv` it.

- [ ] Absorb the nested repo: remove (or archive elsewhere) `seamlyLayout/.git`, move the tree to `src/app/seamlylayout`, and `git add` the files so they are tracked directly like `src/app/seamlyme` (its standalone history stays in the old repo/remote if needed for reference)
- [ ] Make sure this repo's `.gitignore` covers the seamlylayout build outputs formerly ignored by the nested repo's own `.gitignore` (Rust `target/`, `qt_frontend/build/`, etc.) before the `git add`
- [ ] Update all path references to the old location: root `CLAUDE.md` (architecture bullet, test-pattern path `seamlyLayout/input/richmond-shirt_v1_v061-test.sm2d`), `TODO.md`/`PROJECT_PLAN.md` task text, status-doc mirrors (`seamlyLayout/docs/status-docs/svg-data-attributes.md`), packaging scripts (`packaging/windows/SeamlyLayout.iss`, `build_installer.ps1`), and any scripts/docs that hard-code `seamlyLayout/`
- [ ] Check seamly2d-side references: `exportPiecesToSeamlyLayout()` and the `paths/seamlyLayoutApp` setting default — update any hard-coded relative paths to the app or its `input/` directory
- [ ] Confirm the qmake build still excludes the moved directory (nothing under `src/app/seamlylayout` is picked up by `src/app/app.pro` / SUBDIRS)
- [ ] Verify the seamlyLayout build still works from the new location (`src/app/seamlylayout/qt_frontend/qd.ps1`) and its tests pass
- [ ] Verify a full Seamly2D build (`scripts/sd.ps1`) is unaffected
- [ ] Update `.github/README-BUILDS.md` and the seamlyLayout `CLAUDE.md` for the new paths

## Task 20 — GitHub workflow to build seamlylayout on push (Qt 6.10)

Add a push-triggered CI workflow that builds seamlylayout, following the pattern used for seamly2d/seamlyme in `.github/workflows/ci.yml`. The existing CI pins Qt 6.8.3 (`QT_VERSION` in `ci.yml`), but seamlylayout requires Qt 6.10, so for now it needs its own job/workflow on a separate `ubuntu-latest` runner installing Qt 6.10 via `jurplel/install-qt-action`. In the future seamly2d and seamlyme will move to Qt 6.10 as well, at which point the workflows can share one toolchain.

- [ ] Create `.github/workflows/` workflow (or a job in `ci.yml`) triggered on push touching the seamlylayout tree, on `ubuntu-latest`
- [ ] Install the toolchain: Qt 6.10.x via `jurplel/install-qt-action` (with the QML/Quick modules seamlylayout needs) plus a stable Rust toolchain (`rustup`/`dtolnay/rust-toolchain`), with cargo and Qt caching
- [ ] Build the Rust backend and the Qt 6.10 QML frontend from the seamlylayout directory (mirror what `qd.ps1` does, adapted to Linux/CI)
- [ ] Run the seamlylayout unit tests (Rust tests and the Qt frontend tests, e.g. `PreferencesModelTests`) in the workflow
- [ ] Keep the job independent of the Qt 6.8.3 seamly2d/seamlyme jobs so a seamlylayout failure doesn't block them (and vice versa); use path filters so it only runs when seamlylayout files change
- [ ] Note the future consolidation in the workflow comments: when seamly2d/seamlyme move to Qt 6.10, merge this into the shared CI toolchain
- [ ] Document the new workflow in `.github/README-BUILDS.md` and `.github/workflows/README_WORKFLOWS.md`

## Task 21 — SeamlyLayout: three text modes for SVG export in the Exports menu

Replace the single "SVG" item in the SeamlyLayout Exports menu (`seamlyLayout/qt_frontend/qml/ExportMenu.qml`, `exportSvgRequested()` wired through `Main.qml` to the Rust backend in `seamlyLayout/crates/cxxqt_bridge/src/exports.rs`) with three SVG export modes differing in how label text is written.

1. **Text as `<text>` in the designer's selected (outline) font** — searchable, editable, re-stylable, human- and machine-readable; embeds the font via `@font-face` so it renders correctly on machines without it; supports tech-pack generation. Smallest file size.
2. **Text as `<text>` in a Hershey/single-line font** — same searchable/editable/machine-readable intent as mode 1, but using a bundled single-line font so the result is also friendly to CAD/CAM tools that resolve text. **Implementation note:** true Hershey fonts are stroke data, not installable outline fonts — for `<text>` + `font-family` to work this mode needs a "hairline" single-line TTF/OTF (an engineered font whose outline doubles back on itself to look like one stroke) embedded via `@font-face`. Known candidates: CamBam Stick Fonts (free, 9 variants, designed for CNC/plotting), MecSoft/Rhino single-stroke fonts, commercial single-line TTF bundles — verify redistribution/embedding license compatibility with the MIT Rust core before bundling. Caveats: hairline TTFs are still doubled-back outlines (stroke width not controllable via the font), and consumers that ignore embedded fonts will substitute — so mode 3 remains the guaranteed-fidelity choice for cutters. Record the font choice and rationale in `DECISIONS.md`.
3. **Text converted to paths (single-stroke)** — each label rendered as single-stroke `<path>` polylines from Hershey glyph data (**decision:** the path conversion uses the Hershey font, not the designer's outline font — a plotter then draws each character in one pen pass instead of tracing hollow glyph contours). Compatible with CAD/CAM/cutters/plotters/engravers with no font dependency in the consumer. Keep the original string machine-readable via `data-*`/`<desc>` on the label group (text is no longer searchable/editable as SVG text).

**Dependency:** all three modes need real `<text>` elements in the incoming `.pieces.svg` (Task 10) — even mode 3 needs the label *strings* to re-render them in stroke glyphs. Already-outlined input (Seamly2D `--text2paths`) can only be passed through as-is; the UI must handle path-only input (disable the text modes with an explanatory tooltip, or export the existing paths with a warning). Optional Hershey display/export on the Seamly2D side is Task 22.

- [ ] Replace the single "SVG" `MenuItem` with a three-entry submenu (or dialog choice) in `ExportMenu.qml`; add per-mode tooltips summarizing the compatibility/editability trade-off; wire new signals through `TopMenuBar.qml`/`Main.qml` to the bridge
- [ ] Mode 1: pass `<text>` through in the designer's font and embed the font as a subsetted `@font-face` data-URI (WOFF/TTF); document the font-licensing caveat (embedding rights vary by font license) in the export docs
- [ ] Mode 2: emit `<text>` styled with the bundled single-line font, embedded via `@font-face`, per the `DECISIONS.md` decision
- [ ] Mode 3: implement single-stroke text rendering in the Rust core — shape each label string into Hershey glyph strokes and emit stroked (fill-less) `<path>` polylines via `svg_dom`; keep the original label string on the group via `data-*`/`<desc>`
- [ ] Bundle Hershey/single-line glyph data under a permissive license compatible with the MIT Rust core (evaluate existing crates, e.g. a Hershey-font crate, before hand-rolling)
- [ ] Preserve the `data-*` tagging contract (`piece_label`/`pattern_label` groups, ids, `data-parent`) identically in all three modes
- [ ] Detect path-only input (no `<text>` in labels) and gate all three text modes accordingly
- [ ] Persist the last chosen SVG text mode in preferences (`PreferencesModel`)
- [ ] Tests: Rust unit tests for each conversion mode (mode 3 output is stroked polylines, not filled contours), plus the path-only-input case; frontend test for menu gating; end-to-end check with the richmond test pattern
- [ ] Update `seamlyLayout/docs/status-docs/svg-data-attributes.md`, the root `status-docs/svg-data-attributes.md` mirror, and `seamlyLayout/docs` export docs
- [ ] Doxygen briefs + inline comments on all touched functions

## Task 22 — Seamly2D: optional single-stroke (Hershey) label display and export

**Scope decision (2026-07-18):** outline fonts remain fully acceptable in Seamly2D — the existing outline-font canvas display and exports stay the default and are NOT removed or locked out. This task adds single-stroke (Hershey-style) label support as an *option*, so a designer targeting a cutter/plotter can preview labels on the canvas the way the machine will draw them and export matching single-stroke output.

Current label text handling (all outline-font based, and staying available): the canvas paints labels with the user-selected `QFont` via `painter->drawText()` (`VTextGraphicsItem::paint()`, `src/libs/vwidgets/vtextgraphicsitem.cpp`); the `textAsPaths == true` branch of `VLayoutPiece::createLabelItem()` (`src/libs/vlayout/vlayoutpiece.cpp`) outlines glyphs via `QPainterPath::addText()`; DXF text goes through `VDxfEngine::drawTextItem()` (`src/libs/vdxf/vdxfengine.cpp`).

**Constraint:** neither Windows nor Qt has native stroke-font support — the OS/Qt typography stack (`QFont`) only loads outline TTF/OTF, and a true single-stroke font cannot be installed as a system font (Windows errors on it or auto-closes the open contours into filled shapes). Two known workarounds exist in the wild:

1. **App-internal stroke-glyph data** (the true single-stroke route) — bypass the system font stack entirely and render strokes from Hershey glyph data inside the app, as Inkscape's *Hershey Text* extension does (Hershey Sans/Serif/Script 1-stroke, Gothic, and Duplex/Triplex multi-stroke variants). **This is the approach for this task** — the custom renderer below; prior art also includes Valentina's single-line/SVG-font label support.
2. **"Hairline" TTFs** — engineered fonts that trace each line forward and back over itself so the closed outline *looks* like a single stroke (e.g. CamBam Stick Fonts — free, 9 variants for CNC/plotting; MecSoft/Rhino single-stroke; commercial single-line TTF bundles). These install as normal Windows fonts and work in ordinary apps, but they are still outlines (doubled-back), stroke width isn't controllable via the font, and licensing must be checked before bundling. Relevant mainly as the embeddable-`<text>` option in Task 21 mode 2, not for this task's renderer.

- [ ] Choose and bundle the stroke-glyph source (Hershey glyph data and/or single-line SVG fonts) under a GPL-compatible license; record the decision in the repo docs
- [ ] Implement a single-stroke text renderer: shape a label line into stroked (fill-less) `QPainterPath` polylines with proper advance/kerning, honoring size, bold/italic variants (stroke width/slant), alignment, eliding, mirroring, and rotation
- [ ] Add the option to the label settings UI: extend the label font selection (preferences Graphics View page / piece label settings) with the bundled single-stroke fonts alongside the existing system outline fonts; outline stays the default
- [ ] Canvas: when a single-stroke font is selected, render labels with the stroke renderer in `VTextGraphicsItem` so the preview matches what a plotter will draw; outline-font labels keep the existing `painter->drawText()` path
- [ ] Export: when a single-stroke font is selected, the `textAsPaths == true` branch of `createLabelItem()` emits single-stroke paths for those labels (outline-font labels keep the existing `QPainterPath::addText()` conversion); DXF export of single-stroke labels emits polylines
- [ ] Coordinate with Task 10/Task 21: labels in a single-stroke font exporting as `<text>` should reference the bundled single-line font name so SeamlyLayout can match it
- [ ] Verify: canvas single-stroke labels legible at typical zooms with correct placement, mirroring, and rotation; outline-font behavior unchanged everywhere; tagged pieces SVG / `.pieces.svg` / `--text2paths` / DXF / PDF / PNG correct in both font modes
- [ ] Doxygen briefs + inline comments on all touched functions; document the font architecture in the repo docs

## Task 23 — Fix the Seamly2DTests suite hanging at startup on Windows (local runs)

The full unit-test suite cannot be run locally on this Windows machine: the debug-built `Seamly2DTests.exe` (from `scripts/sd.ps1`, exe at `seamly2d-build-debug/src/test/Seamly2DTest/bin/`) hangs at startup — no QTest output at all (not even the first `********* Start testing` banner), near-zero CPU after 10+ minutes, so it is blocked waiting on something, not computing. CI is unaffected (the `linux-test` job runs the suite under xvfb on Ubuntu), so the new-test gate currently relies entirely on CI.

Known observations (2026-07-18, while verifying Task 10):

- The exe needs DLLs that are not on `PATH` by default: the **debug** Qt DLLs (`Qt6Cored.dll` etc.) and `xerces-c_3_3.dll`, both deployed next to `seamly2d.exe` in `seamly2d-build-debug/src/app/seamly2d/bin/` — without them it dies instantly with `0xC0000135` (that part is understood, not the bug)
- With that directory on `PATH` the process starts but blocks before any test output; stdout redirection shows nothing even after minutes
- The hang is before or during `TestApplication2D` startup in `qttestmainlambda.cpp` (`VAbstractApplication` → `QApplication` init, `openSettings()`) or in the first test class init — exact point unknown
- The release-built `Seamly2DTests.exe` in `build/` exited within seconds but silently with no output when probed (likely the missing-`xerces` DLL case, needs a re-probe with proper `PATH`)
- The debug `seamly2d.exe` GUI app itself starts fine on this machine, so plain `QApplication` init is not the blocker

**Root cause found (2026-07-18, while verifying Task 11):** the "hang" is a hidden modal error dialog, not a deadlock. `Seamly2DTests.exe` finds the Qt debug DLLs via `PATH` but Qt looks for the **platform plugin** (`platforms\qwindowsd.dll`) next to the *executable* — windeployqt deployed the plugin directories next to `seamly2d.exe` only, so `QGuiApplication` startup hits the "no Qt platform plugin could be initialized" `qFatal` in `qguiapplication.cpp`, which in a debug-CRT build pops a modal dialog and blocks (the popup was observed on screen while the suite "hung"). With `QT_PLUGIN_PATH` set to `seamly2d-build-debug\src\app\seamly2d\bin` (in addition to `PATH`), the suite runs to completion in seconds. Remaining loose ends for this task:

- The suite currently exits with status 2 (QTest failure count OR-ed across suites) — some pre-existing suite has 2 local failures on Windows; identify and fix (the per-suite `-o file,txt` logger is overwritten by each `qExec`, so per-suite output capture needs the runner script or a logger tweak)
- QTest output to **stdout** is lost when redirected (console and `cmd` redirection both yield 0 bytes; the `-o <file>,txt` file logger works fine) — investigate/route around in the `st.ps1` runner
- New suites verified locally via the file logger: `TST_SvgComponentTags` (Task 11) all pass

- [x] Reproduce and locate the block — root cause found 2026-07-18: missing platform plugin next to the test exe → modal `qFatal` dialog from `qguiapplication.cpp` (see above); workaround `QT_PLUGIN_PATH` confirmed
- [ ] Fix it properly so the suite runs without manual env setup: deploy the Qt plugins next to `Seamly2DTests.exe` (windeployqt post-link step on the test target, mirroring `seamly2d.pro`) or set the plugin path in the runner script; both debug and release builds
- [ ] Identify and fix the 2 pre-existing local test failures (suite exit status 2; suite unknown — per-suite output capture needed, see stdout loose end above)
- [ ] Make the suite easy to run: a small script (e.g. `scripts/st.ps1`, "seamly2d tests", s-prefix rule) that sets `PATH` to the deployed DLL directory (debug Qt DLLs + xerces) plus `QT_PLUGIN_PATH`, runs `Seamly2DTests.exe`, and works around the lost-stdout issue (file logger), mirroring the `sd.ps1` style, with the GPLv3 header and `.SYNOPSIS`
- [ ] Verify: full suite passes locally (including `TST_SvgTextItem` from Task 10); document the Windows test-run procedure in `.github/README-BUILDS.md`

## Task 24 — CLI: run seamly2d from the command line through to a finished seamlyLayout layout

Extend the existing console export mode (`--basename` in `src/app/seamly2d/core/vcmdexport.cpp`) so a single seamly2d command line produces the final layout: seamly2d generates the tagged `.pieces.svg` (the Layout Mode handoff, `exportPiecesToSeamlyLayout()` in `src/app/seamly2d/mainwindow.cpp`) and then runs seamlyLayout on it to produce the layout output, using the new seamlyLayout export options (the Task 21 SVG text modes and the other export formats).

**Dependency:** Task 21 (seamlyLayout export modes) for the mode pass-through; seamlyLayout also needs a headless/CLI export mode of its own, since today it is only driven interactively through its QML UI.

- [ ] Add a seamly2d CLI option (export mode) that triggers the Layout Mode handoff from the console: generate `<basename>.pieces.svg` and invoke seamlyLayout on it, resolving the app path the same way as the GUI (`paths/seamlyLayoutApp` setting)
- [ ] Add a headless CLI export mode to seamlyLayout: input `.pieces.svg`, run the layout/nesting, export to a chosen format and output path without showing the QML UI, exit with a meaningful status code
- [ ] Pass the seamlyLayout export options through the seamly2d command line (export format incl. the Task 21 SVG text modes, output destination), and document the option mapping
- [ ] Make the seamly2d invocation wait for seamlyLayout (unlike the GUI's `QProcess::startDetached`), propagate its exit status and stderr so scripted callers see failures
- [ ] Tests: seamly2d CLI option parsing (extend `tst_vcommandline`), seamlyLayout headless-export tests (Rust/Qt side), and an end-to-end check with the richmond test pattern
- [ ] Document the workflow (command-line examples) in the repo docs / `--help` output

## Task 25 — Audit and fix the Seamly2D CLI so all options work

Go through every command-line option seamly2d advertises (defined in `src/libs/vmisc/commandoptions.cpp`, wired in `src/app/seamly2d/core/vcmdexport.cpp`) and make each one actually work in console export mode. Known friction from Task 11 verification: option names are case-sensitive and inconsistently cased (e.g. `--exportOnlyDetails`), and errors only surface in a redirected stderr, not on the console.

- [ ] Inventory all options and build a test matrix: expected behavior, required companions (e.g. `--basename` enabling export mode), valid values
- [ ] Exercise each option against the richmond test pattern (all export formats, gradation size/height, page options, `--text2paths`, measurement overrides, etc.) and record which are broken, ignored, or misdocumented
- [ ] Fix the broken/ignored options; make error messages reach the console reliably (the GUI-subsystem exe detaches from the console — evaluate `AttachConsole`/subsystem handling on Windows so `--help` and errors print without redirection)
- [ ] Consider case-insensitive or consistently lowercase option aliases (keeping the existing names working for compatibility)
- [ ] Unit tests: extend `tst_vcommandline` to cover every option and the failure modes found
- [ ] Update `--help` text and repo docs with the verified behavior

## Task 26 — Export multisize patterns (nested / marker / sized-layout-set)

Add layout export for multisize patterns — `.sm2d` patterns opened with a `.smms` multisize measurement file (multiple sizes; the CLI already exposes per-size gradation via `--gradationsize`/`--gradationheight`). The user chooses one of three multisize layout products in the settings dialog; all products orient every piece with its grainline pointing up.

- [ ] Settings dialog: user chooses "nested layout", "marker layout", or "set of sized layouts" for multisize export
- [ ] Generate a "size layout" for each size in the `.smms` file, all grainlines pointing up (per-size piece generation via the existing gradation machinery)
- [ ] Nested layout:
  - [ ] For each piece in the largest size, create a layout with all grainlines pointing up
  - [ ] For the remaining sizes in descending order: place each piece on top of its matching largest-size piece, grainline up, centering its center point on the largest piece's center point — each large piece becomes the base of a "pyramid" of matching pieces with the smallest on top
  - [ ] Apply transforms so all pieces are placed in global space
  - [ ] Group all pieces of each size together, so upstream tools (Pattern Projector, Inkscape, Illustrator, ...) can toggle each size's visibility
- [ ] Marker layout: copy all pieces from the size layouts and arrange them into a single marker layout, all grainlines pointing up
- [ ] Set of sized layouts:
  - [ ] Let the user view each size's layout in the canvas — UI design open: per-size tabs across the top of the canvas is the working idea, to be settled during implementation
  - [ ] Export the set to a single multi-page PDF, or to individual files of any export type
- [ ] Tests with a multisize test pattern (need a `.sm2d` + `.smms` fixture); verify grouping/grainline orientation in the exported SVG/PDF
- [ ] Doxygen briefs + inline comments on all touched functions; document the three products in the repo docs
