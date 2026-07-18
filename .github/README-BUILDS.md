# Seamly Builds — Knowledge Base

Pertinent knowledge about the Seamly family builds: why decisions were made, what is built, where things install and store data, and when/where each build runs. Update this file whenever build knowledge changes — it is the durable record behind the task entries in `TODO.md` / `COMPLETED.md`.

Apps covered:

- **seamly2d** — pattern drafting (parent app), `src/app/seamly2d`, Qt 6 / C++ / qmake
- **seamlyme** — measurements, `src/app/seamlyme`, Qt 6 / C++ / qmake
- **seamlyLayout** — daughter layout app, `seamlyLayout/`, Rust + Qt 6.10/QML, own build (`seamlyLayout/qt_frontend/qd.ps1`), deliberately outside the Seamly2D qmake build

## The apps are a family, not standalone programs

This constrains every packaging decision below:

- seamly2d launches seamlyme and seamlyLayout as detached processes (`QProcess::startDetached` in `src/app/seamly2d/mainwindow.cpp`; seamlyLayout via `exportPiecesToSeamlyLayout()`).
- seamly2d hands a tagged `.pieces.svg` file to seamlyLayout (Layout Mode handoff; attribute spec in `status-docs/new-attributes.csv`).
- The apps share files and variables: measurement files, settings values (e.g. the `paths/seamlyLayoutApp` executable path stored via `VSettings`, `src/libs/vmisc/vsettings.cpp`).

Therefore all packaging must keep the apps installed together (or mutually locatable) and able to see the same user data. On sandboxed platforms (Flatpak) they must share one sandbox.

## Toolchains (Windows development)

Two toolchains are in use — the difference is intentional, not an error:

| | Where | Qt | Compiler | Notes |
|---|---|---|---|---|
| **CI** | GitHub hosted runner | 6.8.3 | MSVC 2022 | Used by release/CI workflows; limited to what GitHub runners provide |
| **Local** | Developer PC | 6.10.1 `msvc2022_64` | VS 18 Community (`vcvars64.bat`) | qmake + jom; release shadow-build in `build/` (gitignored) |

- Local debug build: `scripts/sd.ps1` — auto-detects the newest Qt 6.10.x msvc2022_64 kit under `C:\Qt` and the VS 18 MSVC environment, shadow-builds `CONFIG+=debug` into `seamly2d-build-debug/` (gitignored); debug exe at `seamly2d-build-debug/src/app/seamly2d/bin/seamly2d.exe`, Qt debug DLLs deployed by windeployqt; `-Run` launches after build.
- seamlyLayout builds separately with `seamlyLayout/qt_frontend/qd.ps1` and must stay out of the qmake build.

## Settings / preferences storage

### Current state (as of 2026-07)

| App | Windows location | Source of the name |
|---|---|---|
| seamly2d, seamlyme | `C:\Users\<user>\AppData\Local\Seamly2D` | org name from `VER_COMPANYNAME_STR` (`src/libs/vmisc/projectversion.h`), applied in `src/app/seamly2d/core/application_2d.cpp` and `src/app/seamlyme/application_me.cpp` |
| seamlyLayout | `C:\Users\<user>\AppData\Local\Seamly Systems` | hard-coded `app.setOrganizationName("Seamly Systems")` in `seamlyLayout/qt_frontend/main.cpp` |
| seamlyLayout (packaged defaults) | `<exeDir>\settings\` (relative to `seamlyLayout.exe`) | Inno Setup installs `default_settings.json` and paper/roll presets there (`seamlyLayout/packaging/windows/SeamlyLayout.iss`) |

### Planned: one unified `Seamly` organization folder (TODO Tasks 15–18)

**Why:** three scattered locations are confusing to users and support, and complicate backup/migration. The fix is a single organization name, `Seamly`, so Qt's `QSettings`/`QStandardPaths` resolve every app under one parent folder.

**What:** change the org name everywhere (`VER_COMPANYNAME_STR` + the seamlyLayout hard-coded string), migrate legacy data in-app on first run (must not rely on installer logic — Flatpak has no installer), and make seamlyLayout's exe-relative `settings\` strictly read-only packaged defaults.

Per-platform targets:

| Platform | Unified location | Task |
|---|---|---|
| Windows | `C:\Users\<user>\AppData\Local\Seamly` | Task 15 |
| macOS | `~/Library/Application Support/Seamly` (+ Preferences plists) | Task 16 |
| Linux AppImage | `~/.config/Seamly`, `~/.local/share/Seamly` (XDG) | Task 17 |
| Linux Flatpak | `~/.var/app/<app-id>/config/Seamly` inside the **single shared** sandbox | Task 18 |

## User data files (patterns, measurements)

- Default user data tree on Windows: `C:\Users\<user>\seamly2d`.
- Users legitimately relocate it — e.g. to a cloud-synced drive (`G:\My Drive\seamly2d`) for access while travelling. Installers and apps must treat the location as configurable, not fixed (see the Task 14 installer prompts).

## Per-platform build & packaging

### Windows

- **Current:** CI builds via GitHub workflows (Qt 6.8.3 + MSVC 2022). seamlyLayout has its own Inno Setup installer (`seamlyLayout/packaging/windows/SeamlyLayout.iss`, `build_installer.ps1`) with legacy-settings migration logic.
- **Planned (Task 13):** a Windows **.msi** installer covering all three apps, x64 **and** arm64. Prerequisite: seamlyLayout launchable from seamly2d and passing unit/functional tests. Tooling decision pending (e.g. WiX).
- **Planned (Task 14):** the installer prompts for two paths instead of hard-coding them:
  1. **Executable install path** — default `C:\Program Files (x86)\Seamly2D`, any drive allowed (use case: `D:\Program Files (x86)\Seamly2D`); the chosen directory is added to the system `PATH` automatically and removed on uninstall.
  2. **User data path** — default `C:\Users\<user>\seamly2d`, any drive allowed including cloud-synced (use case: `G:\My Drive\seamly2d`); registered automatically so the apps use it without re-prompting. Open design point: whether a data directory belongs on `PATH` or is better served by an env var / registry / `QSettings` value — decide and document during Task 14.
  - Both paths must survive upgrade-in-place (MSI upgrade codes).
- Code signing: see `.github/workflows/CODE_SIGNING.md` and `.github/workflows/signing/`.

### macOS

- Settings unification is Task 16: land in `~/Library/Application Support/Seamly`, migrate legacy `Seamly2D` / `Seamly Systems` Application Support dirs and preferences plists on first run, keep packaged defaults read-only inside the app bundle resources.
- Existing user-facing install doc: `.github/Seamly-MacOS-Installation-v2.pdf`.

### Linux — AppImage

- Built in GitHub CI. Settings follow XDG paths; unification is Task 17 (`~/.config/Seamly`, `~/.local/share/Seamly`, first-run migration).
- AppImage mounts are read-only, which naturally enforces "bundled defaults are read-only"; all writes go to the XDG `Seamly` paths.

### Linux — Flatpak (built at Flathub, **not** on GitHub)

- **Where/when:** the Flatpak is built from the Flathub manifest repo, not this repo's CI. Releases reach Flathub via a version bump in that manifest — coordinate timing separately from GitHub releases.
- **Decision (2026-07): do NOT change the Flatpak way of building.** Keep the existing Flathub package structure and single app id.
- **Why one sandbox:** the apps share files and variables and launch each other via `QProcess::startDetached`; cross-sandbox process launches and file handoffs do not work in Flatpak. So all apps ship inside the one existing Flatpak app id, and the unified `Seamly` folder (`~/.var/app/<app-id>/config/Seamly/`) is **one shared physical directory** inside that sandbox — not per-app copies.
- Consequences (Task 18): seamlyLayout must be added to the existing Flathub package if not yet included; in-sandbox launches must resolve to `/app/bin` executables (not host paths), including the `paths/seamlyLayoutApp` setting default; legacy-settings migration must be in-app (no installer exists); packaged defaults are read from the read-only `/app/...` prefix.

## Related records

- `TODO.md` — Tasks 13–18 hold the current actionable subtasks for everything marked "planned" above; completed tasks move to `COMPLETED.md`.
- `PROJECT_PLAN.md` — the approved implementation plan.
- `.github/workflows/README_WORKFLOWS.md` — CI workflow details.
- `seamlyLayout/CHANGELOG.md` — history of seamlyLayout's settings-directory moves (e.g. `<exeDir>/settings/` → AppConfigLocation).
