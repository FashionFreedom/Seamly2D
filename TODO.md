:

# TODO — Tagged SVG Handoff to SeamlyLayout

See `PROJECT_PLAN.md` for full details. Check off subtasks as they are accomplished; when every subtask of a task is complete, move the task to `COMPLETED.md`.

## Task 10 — Export label text as real SVG text (not paths or path outlines)

Labels currently export as glyph outlines even with "text as paths" off: `VLayoutPiece::createLabelItem()` (`src/libs/vlayout/vlayoutpiece.cpp`) creates `QGraphicsSimpleTextItem`s, but that item class paints text by stroking/filling a `QPainterPath` internally, so `QSvgGenerator` never receives a text draw call and emits `<path>` outlines instead of `<text>` elements (0 `<text>` in every export, matching the pre-change baseline).

- [ ] Replace the `textAsPaths == false` branch of `createLabelItem()` with a text item that paints through `QPainter::drawText()` / `QTextLayout` (e.g. `QGraphicsTextItem` or a small custom item) so the SVG paint engine's `drawTextItem()` emits real `<text>` elements
- [ ] Preserve current label appearance: font family/pixel size, bold/italic per line, label color, per-line alignment, middle-eliding to label width, mirroring and rotation transforms, line spacing
- [ ] Keep the `textAsPaths == true` branch unchanged (explicit vector outlines remain available)
- [ ] Verify `PrepareTextForDXF` / `RestoreTextAfterDXF` (`collectTextItems()`, `src/app/seamly2d/mainwindowsnogui.cpp`) still find and convert the new item type so DXF export keeps working
- [ ] Verify exports: tagged pieces SVG and Layout Mode `.pieces.svg` contain `<text>` inside `piece_label`/`pattern_label` groups (still correctly `data-*` tagged); `--text2paths` still produces outlines; DXF / PDF / PNG regression
- [ ] Update the label bullet of the `data-*` contract in `status-docs/svg-data-attributes.md` and the mirror in `seamlyLayout/docs/status-docs/svg-data-attributes.md`
- [ ] Doxygen briefs + inline comments on all touched functions

## Task 11 — Add `cut_path` to the SVG component groups

A cut path is a closed internal path that is cut out of the piece and can have its own seam allowance. The data model already separates them (`VLayoutPiecePath::isCutPath()`; stored as `m_cutoutPaths` on `VLayoutPiece`), but `createCutoutPathItem()` (`src/libs/vlayout/vlayoutpiece.cpp`) still tags them `internal_path` as a placeholder because the SVG spec defined no dedicated type.

- [ ] Tag `createCutoutPathItem()` items with `data-type="cut_path"` instead of `"internal_path"` (remove the placeholder comment); cut paths get their own per-piece counter and `piece-<n>-cut_path-<m>` ids automatically via `addComponentGroups()`
- [ ] Add `cut_path` to the type list in `status-docs/new-attributes.csv` and document its semantics (closed, cut out, may carry a seam allowance) in `status-docs/svg-data-attributes.md` and the mirror in `seamlyLayout/docs/status-docs/svg-data-attributes.md`
- [ ] Verify export with a pattern containing at least one cutout internal path (the richmond test pattern may not have one — add one or pick another test pattern): cutouts appear as `data-type="cut_path"` groups, plain internal paths keep `data-type="internal_path"`, counters/ids/`data-parent` correct
- [ ] Regression: tagged SVG inspection still passes; Layout Mode `.pieces.svg` carries the new type; DXF / PDF / PNG unaffected
- [ ] Doxygen briefs + inline comments on all touched functions

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
- [ ] Document both prompts and the silent-install property equivalents in the installer docs

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
