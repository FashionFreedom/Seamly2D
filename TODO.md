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

## Task 12 — Local debug-build script for seamly2d (Qt 6.10.x + VS 18 Community)

A PowerShell script that produces a debug `seamly2d.exe` on this machine, mirroring seamlyLayout's `qt_frontend/qd.ps1` precedent. Verified toolchain (see 2026-07-17 release build): qmake from `C:\Qt\6.10.1\msvc2022_64`, MSVC env from `C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat`, parallel build via `C:\Qt\Tools\QtCreator\bin\jom\jom.exe`.

- [ ] Create `scripts/sd.ps1` (s-prefix naming rule; "seamly2d debug", mirroring seamlyLayout's `qd.ps1`) with the project's GPLv3-or-later header (2026 Seamly2D Project, slspencer) and inline comments
- [ ] Auto-locate the newest Qt `6.10.x\msvc2022_64` kit under `C:\Qt` and the VS 18 Community `vcvars64.bat`; fail early with a clear message naming what is missing (the vcvars vswhere warning is harmless — suppress or note it)
- [ ] Shadow-build into a dedicated debug dir separate from the release `build/` tree (e.g. `seamly2d-build-debug/`, already ignored by the `*-build-*` gitignore pattern — or add the chosen dir to `.gitignore`): `qmake ..\Seamly2D.pro CONFIG+=debug` then jom (fall back to nmake if jom is absent)
- [ ] Verify the script end-to-end on this machine: debug `seamly2d.exe` lands under `<debug-build-dir>\src\app\seamly2d\bin\` with the Qt debug DLLs deployed beside it, and the executable launches
- [ ] Document usage (a `.SYNOPSIS` comment block in the script; mention the script in `CLAUDE.md` build notes)
