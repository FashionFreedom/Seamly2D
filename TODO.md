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
