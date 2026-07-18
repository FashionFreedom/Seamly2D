# COMPLETED — Tagged SVG Handoff to SeamlyLayout

Tasks moved here from `TODO.md` when all their subtasks are complete.

## Task 10 — Export label text as real SVG text (not paths or path outlines) (2026-07-18)

Labels exported as glyph outlines even with "text as paths" off: `QGraphicsSimpleTextItem` with a pen set paints text through a `QTextLayout` outline format, so `QSvgGenerator` never received a text draw call (0 `<text>` in the baseline).

- [x] Replace the `textAsPaths == false` branch of `createLabelItem()` with a text item that paints through `QPainter::drawText()` — new `SvgTextItem` class (`src/libs/vlayout/svg_text_item.h/.cpp`) subclassing `QGraphicsSimpleTextItem` with a `paint()` override, so the SVG paint engine's `drawTextItem()` emits real `<text>` elements
- [x] Preserve current label appearance: font family/pixel size, bold/italic per line, label color, per-line alignment, middle-eliding to label width, mirroring and rotation transforms, line spacing (all outside the changed branch; fill color now comes from the brush, no outline pen)
- [x] Keep the `textAsPaths == true` branch unchanged (explicit vector outlines remain available)
- [x] Verify `PrepareTextForDXF` / `RestoreTextAfterDXF` (`collectTextItems()`) still find and convert the new item type — `SvgTextItem` shares `QGraphicsSimpleTextItem::Type`; DXF flat export of the richmond pattern emits 64 TEXT entities with the label strings and no `%&?_?&%` placeholder leak
- [x] Verify exports (richmond pattern, CLI `--exportOnlyDetails`): SVG has 64 `<text>` inside the 23 correctly `data-*`-tagged `piece_label`/`pattern_label` groups (0 paths in label groups; font-family/size/fill carried); `--text2paths` yields 0 `<text>` / 63 outline paths in the same groups; DXF/PDF/PNG all valid; Layout Mode `.pieces.svg` shares the same render path (`arrangePieceItemsFlat(textAsPaths=false)` → `SvgGenerator`)
- [x] Update the label bullet of the `data-*` contract in `status-docs/svg-data-attributes.md` and the mirror in `seamlyLayout/docs/status-docs/svg-data-attributes.md`
- [x] Doxygen briefs + inline comments on all touched functions; unit tests `tst_svgtextitem.cpp` added to `Seamly2DTest` (`<text>` emission, font styling, multi-line, DXF-discovery cast) — run in CI (`linux-test`); the local Windows debug suite hangs at startup (pre-existing, unrelated)

Note: the `textAsPaths == true` branch emits filled glyph *outlines*, which remains the behavior for outline fonts; an optional single-stroke (Hershey) alternative is tracked separately in Task 22.

## Task 11 — Add `cut_path` to the SVG component groups (2026-07-18)

A cut path is a closed internal path that is cut out of the piece and can have its own seam allowance. The data model already separated them (`VLayoutPiecePath::isCutPath()`; stored as `m_cutoutPaths` on `VLayoutPiece`), but `createCutoutPathItem()` (`src/libs/vlayout/vlayoutpiece.cpp`) tagged them `internal_path` as a placeholder because the SVG spec defined no dedicated type.

- [x] Tag `createCutoutPathItem()` items with `data-type="cut_path"` instead of `"internal_path"` (placeholder comment removed); cut paths get their own per-piece counter and `piece-<n>-cut_path-<m>` ids automatically via `addComponentGroups()` (also updated the `ItemType` doc comment in `vlayoutdef.h`)
- [x] Add `cut_path` to the type list in `status-docs/new-attributes.csv` and document its semantics (closed, cut out, may carry a seam allowance) in `status-docs/svg-data-attributes.md` and the mirror in `seamlyLayout/docs/status-docs/svg-data-attributes.md`
- [x] Verify export with a pattern containing a cutout internal path (richmond has none — verified with a copy of the richmond pattern with the "Left Cut" internal path of piece "Front" flipped to `cut="true"`, CLI `--format 0 --exportOnlyDetails`): the cutout appears as `piece-1-cut_path-1` (`data-type="cut_path"`, own counter starting at 1, `data-parent="piece-1"`), the piece's 4 plain internal paths keep `data-type="internal_path"` with their own counter, all other pieces unaffected
- [x] Regression: tagged SVG inspection passes (all 12 pieces, ids/counters/parents correct); the CLI export writes the same `*_pieces.svg` the Layout Mode handoff uses (same `GetItem()` → `SvgGenerator` path), so `.pieces.svg` carries the new type; PDF and PNG exports of the cutout pattern succeed, and DXF/PDF/PNG never read `PieceItemData::ItemType`, so they are unaffected by the tag change
- [x] Doxygen briefs + inline comments on all touched functions; unit tests `tst_svgcomponenttags.cpp` added to `Seamly2DTest` (item-tree tagging: 1× internal_path + 2× cut_path; end-to-end `SvgGenerator` export: ids, per-type counters, `data-parent`) — all pass locally (run with `QT_PLUGIN_PATH` set per the Task 23 root-cause finding; verified via the QTest file logger) and run in CI (`linux-test`)

## Task 0 — Setup

- [x] Copy approved plan to `PROJECT_PLAN.md`
- [x] Create `TODO.md` and `COMPLETED.md` tracking files
- [x] Create project `CLAUDE.md` and `.claude/` settings
- [x] Export baseline SVG from the test pattern via the installed Seamly2D CLI (`--format 0 --exportOnlyDetails`, measurements passed with `--mfile`) → `status-docs/baseline/richmond-shirt-baseline_pieces.svg` (2026-07-17)

## Task 1 — Shared data keys (`src/libs/vlayout/vlayoutdef.h`)

- [x] Add `PieceItemData::Key` data-key enum (ObjectName / ItemType / PieceLetter) — wrapped in a namespace to avoid collision with `VDrawTool::ObjectName`
- [x] Remove duplicated `static const int ObjectName = 0;` from `svg_generator.cpp` and `vlayoutpiece.cpp`

## Task 2 — Restructure piece item tree (`src/libs/vlayout/vlayoutpiece.cpp` / `.h`)

- [x] `GetItem()`: root becomes empty container item; set ObjectName + PieceLetter data
- [x] Add `createSeamlineItem()` child (from `createMainItem()` body), tag `"seamline"`; remove `createMainItem()`
- [x] Tag `createAllowanceItem` → `"cutline"`, `createNotchesItem` → `"notch"`, internal/cutout path items → `"internal_path"`
- [x] `createLabelItem`: add type param, wrap text lines in a tagged group (`"piece_label"` / `"pattern_label"`)
- [x] Tag `createGrainlineItem` → `"grainline"`
- [x] `VLayoutPiece::Create`: store piece letter in `VLayoutPieceData` with getter/setter
- [x] Doxygen briefs + inline comments on all touched functions

## Task 3 — DXF text traversal guard (`src/app/seamly2d/mainwindowsnogui.cpp`)

- [x] Make `PrepareTextForDXF` / `RestoreTextAfterDXF` scan descendants recursively (shared `collectTextItems()` helper)
- [x] Fix pre-existing `paperItems.at(i)` → `.at(j)` bug

## Task 4 — SvgGenerator data-* attributes (`src/libs/vformat/svg_generator.cpp` / `.h`)

- [x] Constructor: add `patternName` param; members `m_patternName`, `m_pieceCount`
- [x] Factor render block into `renderSceneToDom(QGraphicsScene*)`
- [x] Per-piece path: piece group with `id` / `data-type="piece"` / `data-type-number` / `data-parent` / `data-name` / `data-letter`
- [x] Per-component render passes in `addComponentGroups()` (hide siblings), tag each `<g>` with `data-type`, `data-type-number`, `data-parent`, structured `id`
- [x] `mergeSvgDoms()`: wrap piece groups in `<g id="pattern-1" data-type="pattern" data-name=...>` (piece exports only); use `importNode`
- [x] Robustness: remove all `M0,0`/empty-`d` paths; fix nested empty-group removal; clean origin paths before empty groups
- [x] Doxygen briefs + inline comments on all touched functions

## Task 5 — Export callers (`src/app/seamly2d/mainwindowsnogui.cpp`)

- [x] Pass `doc->GetPatternName()` to `SvgGenerator` in both `exportSVG` overloads

## Task 6 — Programmatic tagged-SVG generation (`src/app/seamly2d/mainwindowsnogui.cpp` / `.h`)

- [x] Factor dialog-independent core out of `exportPiecesAsFlatLayout()` (`arrangePieceItemsFlat()`)
- [x] Add `generatePiecesSvg(const QString &filePath)` producing the tagged SVG from `pieceList` (text kept as real text; success checked via the file on disk)

## Task 9 — Launch SeamlyLayout development build from Layout Mode (branch `run-seamlyLayout`)

- [x] Layout Mode buttons run `C:\Users\susan\Projects\Seamly2D-private\seamlyLayout\qt_frontend\build\Debug\SeamlyLayout.exe`: added the development-build location as a lookup fallback in `Application2D::seamlyLayoutFilePath()` (after the settings override and the install-directory check)
- [x] Doxygen brief + inline comments updated on the touched function

## Task 8 — Verification (2026-07-17)

- [x] Build `vlayout`, `vformat`, `seamly2d` — built clean on branch `run-seamlyLayout` (which carries the `svg-update` work) with qmake + jom, Qt 6.10.1 msvc2022_64 kit and the MSVC toolset from VS 18 Community (the only Qt/VS installed on this machine); all apps, libs and tests link
- [x] Layout Mode click produces tagged SVG; SeamlyLayout launches with it — verified in the running GUI (Shift+L on the loaded richmond test pattern): `<basename>.pieces.svg` written beside the pattern file, SeamlyLayout development build launched detached with the SVG path as its argument, and the generated SVG passes the full structural inspection
- [x] Manual SVG exports carry the attributes — CLI `--format 0 --exportOnlyDetails` with and without `--text2paths`, both fully tagged (12 pieces; seamline/cutline/internal_path/grainline/piece_label/pattern_label groups)
- [x] SVG inspection — script-verified: every group under `pattern-1` has `data-type`/`data-type-number`/`data-parent`, pattern/piece groups carry `data-name`, per-type counters and structured ids correct, all ids unique, no empty groups, no `M0,0`/empty-`d` paths
- [x] Visual diff vs baseline (`status-docs/baseline/richmond-shirt-baseline_pieces.svg`) — canvas/viewBox identical; all 53 geometry paths (seamlines, cutlines, internal paths, grainlines) byte-identical; stroke colors/line weights identical; only the 64 label glyph-outline paths differ (font outline rendering of the older installed baseline build), same count/colors/placement
- [x] DXF / PDF / PNG export regression — flat DXF (AC1027) has 2305 polylines including label outlines (validates the recursive text traversal of Task 3), AAMA DXF keeps 34 TEXT label entities, PDF valid (%PDF-1.4, proper EOF), PNG valid 7318×3423
- [x] `data-*` contract documented in `status-docs/svg-data-attributes.md` and mirrored to `seamlyLayout/docs/status-docs/svg-data-attributes.md`

## Task 12 — Local debug-build script for seamly2d (Qt 6.10.x + VS 18 Community) (2026-07-17)

- [x] Create `scripts/sd.ps1` (s-prefix naming rule; "seamly2d debug", mirroring seamlyLayout's `qd.ps1`) with the project's GPLv3-or-later header (2026 Seamly2D Project, slspencer) and inline comments
- [x] Auto-locate the newest Qt `6.10.x\msvc2022_64` kit under `C:\Qt` and the VS 18 Community `vcvars64.bat`; fail early with a clear message naming what is missing (vcvars output — including the harmless vswhere warning — is suppressed; failure still caught via exit code)
- [x] Shadow-build into `seamly2d-build-debug/` at the repo root (covered by the `*-build-*` gitignore pattern), separate from the release `build/` tree: `qmake Seamly2D.pro CONFIG+=debug` then jom (falls back to nmake if jom is absent)
- [x] Verified end-to-end on this machine: debug `seamly2d.exe` (29.7 MB, `-MDd`) lands in `seamly2d-build-debug\src\app\seamly2d\bin\` with the Qt debug DLLs (Qt6Cored.dll, Qt6Guid.dll, Qt6Widgetsd.dll, ...) deployed by the windeployqt post-link step, and the executable launches to its main window
- [x] Usage documented: `.SYNOPSIS`/`.DESCRIPTION`/`.EXAMPLE` comment-based help in the script (incl. optional `-Run` switch to launch after build); `CLAUDE.md` gained a "Build Notes" section mentioning the script

## Task 7 — Rewire Layout Mode entry (`src/app/seamly2d/mainwindow.cpp`, `core/application_2d.*`)

- [x] `showLayoutMode()`: guards + `preparePiecesForLayout` kept; `exportPiecesToSeamlyLayout()` writes `<basename>.pieces.svg` beside the pattern file (replaces the built-in layout-settings auto-click)
- [x] Add `seamlyLayoutFilePath()` to `Application2D` (settings override first, then app directory) + `paths/seamlyLayoutApp` settings key with getter/setter
- [x] Preferences → Paths: "SeamlyLayout Application" row (file picker; empty = auto-detect next to seamly2d.exe)
- [x] Launch SeamlyLayout detached with the SVG path argument (SeamlyMe pattern)
- [x] Error dialogs for unsaved pattern / failed generation / missing executable; on failure `showLayoutMode()` reverts to the prior mode (`exportPiecesToSeamlyLayout()` returns bool)
- [x] Doxygen briefs + inline comments on all touched functions
