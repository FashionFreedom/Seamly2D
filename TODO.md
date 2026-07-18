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
