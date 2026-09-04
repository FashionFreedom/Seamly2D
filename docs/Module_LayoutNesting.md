# Module: Layout and Nesting

Parent: [Architecture.md](./Architecture.md)

`src/libs/vlayout` packs closed pieces onto paper. It is an **edge-combination heuristic**, not a commercial marker optimizer. Goal: a usable tiled print / cutter nest with a configurable gap, optional rotation and flip.

---

## 1. Why this algorithm

True NP-hard nesting (irregular bin packing) is too slow/interactive-hostile for a desktop CAD click. Seamly2D:

1. Sorts pieces into a **bank** (by area groups or descending size).
2. For each piece, tries to **glue an edge** to the current sheet contour (and optional rotations).
3. Scores candidates by **bounding-square** of the united contour (`VBestSquare`).
4. Commits the best, grows `VContour`, repeats.

It is deterministic enough for CLI tests (`CollectionTest`) and abortable (`m_stopGeneration` atomic).

---

## 2. Class structure

| Class | Role |
|-------|------|
| `VLayoutGenerator` | Orchestrator: bank → papers → graphics; signals Start/Arranged/Finished/Error |
| `VBank` | Piece queue; `Cases` grouping; `GetTiket` / `Arranged` / `NotArranged` |
| `VLayoutPaper` | One sheet; spawns `VPosition` jobs |
| `VContour` | Growing outline of already placed pieces + paper edge |
| `VPosition` : `QRunnable` | Try one (sheetEdge × pieceEdge) pair + rotations |
| `VBestSquare` | Keep best (size, transform, mirror, Combine vs Rotation) |
| `VLayoutPiece` | Layout-ready geometry + `GetItem()` |
| `VAbstractPiece` | Shared SA offset |
| `VTextManager` | Label font fit |
| `VPoster` | Tiled print grid / glue marks |
| `VGraphicsFillItem` | Grainline for export |

`LayoutErrors`: `NoError`, `PrepareLayoutError`, `ProcessStoped`, `EmptyPaperError`.

---

## 3. Generator loop

`VLayoutGenerator::Generate()`:

```
m_bank->Prepare()
optional strip optimization:
    if page height >= 2 * (biggestDiagonal * multiplier + gap)
        shrink virtual height to tile strips

while bank has pieces:
    paper = VLayoutPaper(height, width)
    while LeftArrange() > 0:
        index = GetTiket()
        if paper.arrangePiece(piece):
            Arranged(index)
        else:
            NotArranged(index)   // try later / next sheet
    if paper.Count() == 0 → EmptyPaperError
    else append paper

if stripOptimized: gatherPages()
if unitePages: unitePages()   // merge until QIMAGE_MAX (32768)
```

`GetTiket` (ticket) pops the next candidate according to `Cases`:

| `Cases` | Strategy |
|---------|----------|
| `CaseThreeGroup` | Three area buckets |
| `CaseTwoGroup` | Two buckets |
| `CaseDesc` | Descending size |

CLI `--groupping` maps to this.

---

## 4. Placing one piece — `VPosition::run`

For a given sheet-edge index and piece-edge index:

1. Copy the `VLayoutPiece` (workpiece).
2. `edgesIntersect`: align the two edges (`combineEdges`); if the **layout-allowance** path does not hit `ContourPath`, `saveCandidate(..., BestFrom::Combine)`.
3. If rotation enabled (or sheet contour still empty): `rotate(rotationIncrement)` through 0–360°, same test, `BestFrom::Rotation`.
4. Optional **mirror** if flipping is allowed; if `ForbidFlipping`, a 180° rotation often compensates.

`saveCandidate`:

```
newContour = sheet.UniteWithContour(piece, sheetEdge, pieceEdge, type)
size = boundingRect(newContour)
bestResult.NewResult(size, ..., transform, mirror, type)
```

`VBestSquare` prefers smaller bounding size; `saveLength` biases toward shorter sheet length (fabric grain / roll).

Collision test uses **layout allowance** (equidistant offset of SA or main path by `layoutGap`), not the sewing seam line. Pieces never sit closer than the gap.

---

## 5. Parallelism

`VLayoutPaper` fans out `VPosition` runnables on Qt’s thread pool (one job per edge pair). `m_stop` atomic aborts mid-search (`DialogLayoutProgress`).

---

## 6. Paper, shift, crop, unite

- Page size/units/orientation/margins: `LayoutSettingsDialog`, `PaperSizeFormat` (A0–A4, Letter, Legal, Tabloid, ANSI C–E, rolls 24–44 in, Custom).
- `SetShift`: starting offset from paper origin.
- `autoCrop`: shrink paper item to content.
- Strip optimization: fake shorter pages then `gatherPages()` to a roll.
- `unitePages`: concatenate sheets for a single huge image/PDF (capped at `QIMAGE_MAX`).

---

## 7. `VLayoutPiece::GetItem`

Builds a `QGraphicsItem` tree:

1. Main path
2. Allowance + notches (children)
3. Internal / cutout paths
4. Labels (`createLabelItem` — `QGraphicsTextItem` or path glyphs if `textAsPaths`)
5. Grainline (`VGraphicsFillItem`)

Interactive Piece mode uses `VTextGraphicsItem` / `VGrainlineItem` instead; layout items are dump-only.

---

## 8. Limits (honest)

- Not globally optimal; first-fit + local edge glue can leave holes.
- Heavy rotation increments explode CPU (every edge pair × angles × threads).
- ASTM marker semantics are **not** implemented here (that would be a different exporter).

Related: [Module_PatternPieces.md](./Module_PatternPieces.md), [Module_ExportRender.md](./Module_ExportRender.md).
