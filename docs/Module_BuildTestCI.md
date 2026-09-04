# Module: Build, Tests, i18n, and Packaging

Parent: [Architecture.md](./Architecture.md)

Seamly2D is a **qmake `SUBDIRS`** Qt 6 project (developer docs specify Qt **6.11.1**; code min is Qt **6.2.4**). CI produces rolling versions `YYYY.M.D.Hmm`.

---

## 1. qmake tree

```
Seamly2D.pro          SUBDIRS = src, out
  src/src.pro         libs → app → test   (test skipped if CONFIG+=noTests)
    libs/libs.pro     qmuparser, vpropertyexplorer, ifc, vobj, vdxf, vlayout,
                      vgeometry, vpatterndb, vmisc, vwidgets, vtools, vformat,
                      fervor, vtest, tools
    app/app.pro       seamlyme, seamly2d   (macOS: seamly2d.depends = seamlyme)
    test/test.pro     Seamly2DTest, CollectionTest, ParserTest, TranslationsTest
  out/out.pro         packaging / deploy
```

Root extra target: `lupdate` on `share/translations/translations.pro` and `measurements.pro`.

**Constraint:** source and build paths must not contain spaces (`Seamly2D.pro` errors out).

There is **no** CMakeLists in-tree. Windows: `nmake`/`jom` after `qmake`. Linux: `qmake`/`qmake6` + `make`. macOS: `CONFIG+=macSign` for notarize.

Third-party:

| Lib | How it is obtained |
|-----|-------------------|
| qmuparser | Vendored `src/libs/qmuparser` (BSD) |
| libdxfrw | Vendored `src/libs/vdxf/libdxfrw` |
| xerces-c | System on Linux/macOS; prebuilt `src/libs/xerces-c/{msvc,mingw,macx,msvc-arm64}` |
| Qt 6 | Multimedia, Image Formats, PrintSupport, Xml, Network, Widgets |

`pdftops` (Poppler/Xpdf) is a **runtime** optional for PS/EPS, copied next to the binary on Windows/macOS per developer README.

---

## 2. Tests (`src/test`)

| Suite | Role |
|-------|------|
| **Seamly2DTest** | Unit: geometry, pieces, layout detail, measurements, posters, CLI, name regexp, formula tokens, lock guard, translate vars, find point, read `.val` |
| **CollectionTest** | Integration: real `.sm2d`/`.smis`/`.smms` (and legacy), broken files, issue regressions via CLI |
| **ParserTest** | Console QMuParser harness (`CONFIG += testcase`) |
| **TranslationsTest** | `.ts` integrity, measurement/builtin regexps, parser error i18n |

`vtest` library: shared helpers. PR CI job `linux-test` runs `make check` under xvfb.

---

## 3. i18n / Weblate

- Hosted Weblate: https://hosted.weblate.org/engage/seamly/
- Catalogs: `share/translations/seamly2d_*.ts`, `measurements_*.ts`, `qtbase_*.ts`
- Workflows: `auto-merge-weblate.yml`, `create-and-auto-translate.yml`, `scripts/translations/`
- `SupportedLocales` includes ru, uk, de, cs, he, fr, it, nl, id, es, fi, en_US/CA/IN/GB, ro, zh_CN, pt_BR, el, tr, pl, hu (see `def.h` / settings for the live list)

Measurement strings are a **separate** `.ts` so SeamlyMe and formula translation stay in sync.

---

## 4. CI (`.github/workflows/ci.yml`)

- Qt **6.11.1**
- Version job: `date +%Y.%-m.%-d.%-H%M` (PRs keep a static number)
- Triggers: `develop`, `feat-*`, pull_request, Monday cron, workflow_dispatch
- Linux AppImage via linuxdeploy
- Windows NSIS (`dist/seamly2d-installer.nsi`) + zip (`windeployqt`); x64 and ARM64 artifacts exist in README
- macOS signed/notarized DMG/zip (`out/out.pro`, codesign docs under `.github/workflows/`)

Related docs: `.github/README-DEVELOPER.md`, `CODE_SIGNING.md`, `TRANSLATION_WORKFLOW.md`.

---

## 5. Packaging

| Channel | Mechanism |
|---------|-----------|
| Windows | NSIS + zip; OpenSSL DLLs under `dist/win` |
| macOS | `macdeployqt`, hdiutil, notarytool |
| Linux AppImage | CI |
| Linux Flatpak | Flathub `io.seamly.seamly2d` (recipe **outside** this repo) |
| Desktop | `dist/seamly2d.desktop`, `seamlyme.desktop` |
| MIME | `dist/debian/seamly2d.sharedmimeinfo` |

**Known inconsistency:** Debian MIME lists individual measurements as `*.smim`; application code and tests use **`*.smis`**. Treat `.smis` as canonical.

---

## 6. Version headers

`src/libs/vmisc/projectversion.h`:

- Rolling/product strings updated by `scripts/version.sh` (do not hand-edit the `VER_FILEVERSION` block).
- `VER_COMPANYDOMAIN` is `seamly.io` **without** a URL scheme (Wayland app_id).
- `V_PRERELEASE` marks prerelease builds.

File format versions (0.7.4 / 0.3.4 / 0.4.5) are **independent** of the application rolling version.

---

## 7. Code style (forward policy)

`.github/README-DEVELOPER.md`: JSF-inspired; `snake_case` variables, `lowerCamelCase` functions, `UpperCamelCase` classes; braces on new lines; 120-char lines; spaces not tabs; `//` comments only.

---

## 8. Why qmake still

The SUBDIRS graph and `*.pri` translation hooks are deep. A CMake port would be a project of its own; CI, Flatpak, and contributor docs all assume `Seamly2D.pro`. Architecture consumers should treat qmake as the **source of truth** for link order (`app.depends = libs`).

Related: [Module_Applications.md](./Module_Applications.md), [Module_FileIOParsing.md](./Module_FileIOParsing.md).
