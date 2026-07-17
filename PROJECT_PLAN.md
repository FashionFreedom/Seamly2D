# Tagged SVG Handoff to SeamlyLayout

## Context

Seamly2D's built-in layout engine is being replaced by **SeamlyLayout**, a separate Rust/Qt6/C++ desktop app. The new workflow: the user clicks **Layout Mode** exactly as today, but instead of the built-in layout settings/nesting engine, Seamly2D auto-generates an SVG containing all pattern pieces and their internal objects — every element tagged with `data-*` attributes (per `status-docs/new-attributes.csv`) so SeamlyLayout's algorithm can identify and consume them — and opens SeamlyLayout with that file. Inside SeamlyLayout the user enters layout settings, generates the layout, optionally does manual adjustment, saves, views, and prints. The return leg (Seamly2D reading layouts back) is out of scope; viewing/printing live in SeamlyLayout.

Attributes: `data-type` (`pattern | piece | seamline | cutline | internal_path | grainline | notch | pattern_label | piece_label`, more later), `data-type-number` (per-piece counter per type), `data-parent` (piece → pattern id; sub-element → piece id), `data-name` (pattern/piece name). Per user decision: `data-name` = piece name, plus `data-letter` when a piece letter is set.

## Task tracking

- `TODO.md` at the repo root holds one task per step (Steps 1–7 plus setup/verification), each with checkbox subtasks.
- Each subtask is checked off in `TODO.md` as it is accomplished.
- When all subtasks of a task are complete, the task moves from `TODO.md` to `COMPLETED.md` at the repo root.

## Project rules (apply to all work in this plan)

- **New file naming**: new source files must NOT begin with `v` — begin them with `s` (e.g. `s..._generator.cpp`, not `v..._generator.cpp`). Existing `v*` files keep their names when edited.
- **License header on new files**: GPLv3-or-later header, copyright **2026 Seamly2D Project**, author **slspencer** (follow the existing header block style in `svg_generator.cpp`, updated year/author).
- **Documentation**: all new code and every function that is modified gets a Doxygen-compatible `@brief` (plus `@param`/`@return` where applicable) and inline comments sufficient for an intermediate-level programmer to follow the workflow.

## Current architecture (verified)

**SVG generation** — `src/libs/vformat/svg_generator.cpp`: `SvgGenerator` renders each piece's scene via `QSvgGenerator` into a buffer, loads into `QDomDocument`, sets `id` on the first `<g>` to the piece name (`addSvgFromScene`, 193–234); `mergeSvgDoms()` (59) merges per-piece groups into one SVG; `generate()` (244) writes the file; `cleanSvg()` strips Qt's empty groups and the spurious `M0,0` path.

**Piece items** — `src/libs/vlayout/vlayoutpiece.cpp`: `VLayoutPiece::GetItem()` (1090) builds a QGraphicsItem tree: root paints the seamline (`createMainItem`, 1254); children: allowance/"cutline" (1279), notches (1292), internal paths (1047), cutout paths (1060), labels (1116, piece + pattern), grainline (1217). Tagged via `setData(ObjectName=0, ...)`.

**Layout Mode entry** — `MainWindow::showLayoutMode()` (`src/app/seamly2d/mainwindow.cpp:3963`, wired at 5845): switches docks/actions, filters `pattern->DataPieces()` by `isInLayout()` (4006) with guard dialogs for zero pieces (3993, 4013), builds `pieceList = preparePiecesForLayout(pieces)` (4032; impl `mainwindowsnogui.cpp:778` → `VLayoutPiece::Create`), then — today — shows the built-in layout pages and auto-opens the layout settings dialog if no scenes exist (4070–4073) → `handleNewLayout` (1821) → `toolLayoutSettings` (`mainwindowsnogui.cpp:158`) → `VLayoutGenerator::Generate()`.

**Pieces export path** — `exportPiecesAs()` (`mainwindow.cpp:7095`) → `ExportData(pieceList, dialog)` (`mainwindowsnogui.cpp:259`) → `exportPiecesAsFlatLayout()` (342, arranges all pieces onto one flat paper) → `ExportScene()` (1610) → `exportSVG(name, paper, pieces.at(i))` (900 overload: fresh scene per piece, `addSvgFromScene` per piece, one merged file).

**External-app launch precedent** — SeamlyMe is launched detached: `QProcess::startDetached(seamlyme, arguments, workingDirectory)` with path from `qApp->seamlyMeFilePath()` (`mainwindow.cpp:2223, 6256, 6508, 6886`).

## Part 1 — Tagged SVG generation

QSvgGenerator emits `<g>` on painter state changes, not per item, so one full-scene render can't be mapped back to element types. Approach: make the piece root a pure container with each logical component a tagged direct child, then render one pass per component (siblings hidden) — each pass yields one `<g>` that maps 1:1 to a component. Deterministic, no DOM heuristics, reuses existing machinery; a few extra small vector renders per piece is negligible.

### Step 1 — Shared data keys: `src/libs/vlayout/vlayoutdef.h`

```cpp
enum class PieceItemType : int
{
    ObjectName  = 0,  // existing convention (piece name on root)
    ItemType    = 1,  // SVG data-type: seamline|cutline|notch|internal_path|grainline|piece_label|pattern_label
    PieceLetter = 2   // piece letter for data-letter
};
```

Remove duplicated `static const int ObjectName = 0;` in `svg_generator.cpp:38` and `vlayoutpiece.cpp:82`.

### Step 2 — Restructure piece item tree: `src/libs/vlayout/vlayoutpiece.cpp` / `.h`

- `GetItem()` (1090): root becomes an empty `QGraphicsPathItem` container (no path/pen); keep `setData(ObjectName, GetName())`, add `PieceLetter`.
- New `createSeamlineItem(QGraphicsItem *parent)` — body of `createMainItem()` (1254) as a child item; tag `"seamline"`; remove `createMainItem()`.
- Tag children: `createAllowanceItem` → `"cutline"`; `createNotchesItem` → `"notch"`; `createInternalPathItem` and `createCutoutPathItem` → `"internal_path"` (CSV defines no cutout type yet).
- `createLabelItem` (1116): add `const QString &type` param; wrap per-line text items in an empty `QGraphicsPathItem` group tagged with `type`. Call with `"piece_label"` (`d->pieceLabel`) and `"pattern_label"` (`d->patternInfo`).
- `createGrainlineItem` (1217): tag `"grainline"`.
- `VLayoutPiece::Create` (426): store `piece.GetPatternPieceData().GetLetter()` in `VLayoutPieceData` with getter/setter.

### Step 3 — DXF text traversal regression guard: `src/app/seamly2d/mainwindowsnogui.cpp`

`PrepareTextForDXF` (1244) / `RestoreTextAfterDXF` (1277) scan only direct children for text items; label lines become grandchildren after Step 2 — make the scan recursive. Fix pre-existing bug at 1285 (`paperItems.at(i)` → `.at(j)`).

### Step 4 — `SvgGenerator`: `src/libs/vformat/svg_generator.cpp` / `.h`

- Constructor: add `const QString &patternName`; members `m_patternName`, `m_pieceCount = 0`; pattern root id `"pattern-1"`.
- Factor render block of `addSvgFromScene` (195–216) into `QDomDocument renderSceneToDom(QGraphicsScene *scene)`.
- `addSvgFromScene(scene, item)` with `item != nullptr`: `++m_pieceCount`, `pieceId = "piece-<n>"`. Render once for `<svg>` root attrs; replace its first `<g>` with a fresh piece group: `id=pieceId`, `data-type="piece"`, `data-type-number=m_pieceCount`, `data-parent="pattern-1"`, `data-name` = piece name, `data-letter` if non-empty. Then per direct child of `item`: hide siblings, `renderSceneToDom`, take first `<g>`, `cleanSvg`, skip if empty; `type = child->data(ItemType)`, `n = ++typeCounters[type]` (local `QHash<QString,int>`); set `data-type`, `data-type-number=n`, `data-parent=pieceId`, `id = pieceId+"-"+type+"-"+n`; append via `importNode`. Restore visibility. (`item == nullptr` whole-scene path unchanged.)
- `mergeSvgDoms()` (59): create `<g id="pattern-1" data-type="pattern" data-type-number="1" data-name=m_patternName>` under the root; append each piece group into it via `importNode` (also fixes current cross-document `appendChild`).
- Robustness (each pass now has ≥1 empty container): `removeEmptyOriginPath` (141) — remove **all** `M0,0`/empty-`d` paths, remove parent group only if emptied; `removeEmptyGroups` (108) — use `group.parentNode().removeChild(group)` so nested empty groups are removed.

### Step 5 — Export callers: `src/app/seamly2d/mainwindowsnogui.cpp`

Both `exportSVG` overloads (893, 900): pass `doc->GetPatternName()` (`vabstractpattern.cpp:1391`) to the constructor. Manual SVG export (Export Pieces / Export Layout) thereby gains the same attributes.

## Part 2 — Auto-generate + hand off on Layout Mode entry

### Step 6 — Programmatic tagged-SVG generation: `src/app/seamly2d/mainwindowsnogui.cpp` / `.h`

New `bool MainWindowsNoGUI::generatePiecesSvg(const QString &filePath)`:

- Reuses the flat-arrangement core of `exportPiecesAsFlatLayout()` (342) to place all `pieceList` pieces onto one paper, but without the `ExportLayoutDialog` — factor the dialog-independent core out (paper construction + piece items) so both the dialog path and this call share it. Format fixed to SVG, text as real text (not paths).
- Runs `SvgGenerator` (with pattern name) over the per-piece scenes exactly like the `exportSVG` overload at 900.
- Returns success/failure for the caller's error dialog.

### Step 7 — Rewire `MainWindow::showLayoutMode()`: `src/app/seamly2d/mainwindow.cpp`

Keep the existing guards (zero pieces / zero in-layout pieces, 3993–4020) and `pieceList = preparePiecesForLayout(pieces)` (4032). Then, instead of showing the built-in layout pages and auto-clicking `layoutSettings_ToolButton` (4070–4073):

1. Require a saved pattern file (consistent with other flows); write the tagged SVG to the pattern's directory as `<pattern-basename>.pieces.svg` via `generatePiecesSvg()`.
2. Launch SeamlyLayout detached, following the SeamlyMe pattern: `QProcess::startDetached(seamlyLayoutPath, {svgPath}, workingDir)`. Add `seamlyLayoutFilePath()` to the application class (`src/app/seamly2d/core/application_2d.*`, mirroring `seamlyMeFilePath()`) plus a preferences entry for the executable path.
3. If the executable isn't found or generation fails: message box explaining the problem; stay in the prior mode.

- The built-in layout UI (`layoutPages_DockWidget`, layout settings dialog, `VLayoutGenerator` pipeline, in-app layout print/preview actions) is no longer triggered from Layout Mode but its code stays in place; physical removal is a follow-on cleanup once SeamlyLayout is fully adopted. Command-line export (`DoExport`) is untouched.

## Resulting SVG shape

```xml
<svg width="..." viewBox="..." xmlns="http://www.w3.org/2000/svg">
  <g id="pattern-1" data-type="pattern" data-type-number="1" data-name="Summer Blouse">
    <g id="piece-1" data-type="piece" data-type-number="1" data-parent="pattern-1"
       data-name="Front Bodice" data-letter="A">
      <g id="piece-1-seamline-1" data-type="seamline" data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-cutline-1"  data-type="cutline"  data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-notch-1"    data-type="notch"    data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-internal_path-1" data-type="internal_path" data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-piece_label-1"   data-type="piece_label"   data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-pattern_label-1" data-type="pattern_label" data-type-number="1" data-parent="piece-1">...</g>
      <g id="piece-1-grainline-1"     data-type="grainline"     data-type-number="1" data-parent="piece-1">...</g>
    </g>
    <g id="piece-2" data-type="piece" data-type-number="2" data-parent="pattern-1" data-name="Back Bodice">...</g>
  </g>
</svg>
```

## Design decisions

- **Id scheme**: `pattern-1`, `piece-<n>`, `piece-<n>-<type>-<m>` — unique, XML-valid. Breaking change for manual SVG export: piece `id` was previously the raw piece name; the name moves to `data-name` (strictly better for downstream tools). Call out in PR.
- **Handoff file**: `<pattern-basename>.pieces.svg` beside the pattern file — persistent and debuggable; trivial to change to a temp file later if preferred.
- **SeamlyLayout code**: available for reference in the `seamlyLayout/` subdirectory (Rust crates + Qt 6.10/QML frontend, built via `seamlyLayout/qt_frontend/qd.ps1`). It must stay out of the Seamly2D qmake build. Its `svg_dom` crate parses generic SVG DOM and defines no `data-*` expectations yet, so the attribute contract in this plan is the source of truth — document it in `status-docs/` and mirror it into `seamlyLayout/docs/` so both apps develop against the same spec. The launch mechanism remains a preferences-configurable executable path (pointing at the built SeamlyLayout frontend).
- **Notches**: one `data-type="notch"` group per piece for now; per-notch splitting is a small follow-on in `createNotchesItem` if the algorithm needs individual notches.
- **Counters**: per-type counters local per piece; piece counter on the SvgGenerator instance (one instance = one file = one pattern).

## Verification (Windows, Qt 6.8.3, MSVC 2022)

1. Build on branch `svg-update` (targets `vlayout`, `vformat`, `seamly2d` recompile).
2. Export a baseline SVG (Export Pieces) before changes for visual diffing.
3. **Test pattern**: `seamlyLayout\input\richmond-shirt_v1_v061-test.sm2d` (should exercise multiple pieces, internal paths, notches, grainline, piece + pattern labels):
   - **Layout Mode click**: verify `<basename>.pieces.svg` appears next to the pattern file; SeamlyLayout launches (or, until it's installed, point the preferences path at a stub executable and verify it receives the SVG path argument; verify the missing-executable error dialog otherwise).
   - **Manual exports**: Piece mode → Export Pieces → SVG (text-as-paths on and off); Export Layout → SVG on an existing saved layout if applicable.
4. Inspect the SVG: every group under `pattern-1` has `data-type`/`data-type-number`/`data-parent`; ids unique; pattern/piece groups carry `data-name` (+ `data-letter`); no empty groups or `M0,0` paths.
5. Visual regression: new vs baseline SVG in a browser/Inkscape — geometry, colors, line weights, label text, stacking order match.
6. Sibling-format regression: export same pattern to DXF (labels intact → validates Step 3), PDF, PNG.
