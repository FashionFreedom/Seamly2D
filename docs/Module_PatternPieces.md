# Module: Pattern Pieces (Modeling)

Parent: [Architecture.md](./Architecture.md)

A **piece** is a closed garment part ready to cut: main path, optional seam allowance, notches, internal paths, grainline, and labels. Draft geometry stays in `Draw::Calculation`; pieces live in `Draw::Modeling` (`pieceScene`) and later in layout.

---

## 1. Why a second stage

Draft is an open construction graph (points hanging in space). Industry output is **closed outlines** with seam allowance and marks. Modeling copies draft objects into an ordered node list (`VPiecePath`) so SA offset, notches, and nesting see a polygon, not a DAG.

---

## 2. Class structure

```
VAbstractPiece                    // vlayout — SA equidistant, shared with layout
 └── VPiece                       // vpatterndb — full piece
        path: VPiecePath
        nodes: QVector<VPieceNode>
        float: VPieceLabelData, VPatternLabelData, VGrainlineData

VPieceNode                        // id + Tool type + reverse + SA formulas + notch + angle join
VPiecePath                        // PiecePath | CustomSeamAllowance | InternalPath

PatternPieceTool : VInteractiveTool + QGraphicsPathItem
InternalPathTool, AnchorPointTool
VNodePoint, VNodeArc, VNodeEllipticalArc, VNodeSpline, VNodeSplinePath
UnionTool
```

`VLayoutPiece::Create(VPiece, VContainer*)` snapshots geometry for nesting ([Module_LayoutNesting.md](./Module_LayoutNesting.md)).

---

## 3. Main path

`VPiece::GetPath()` is a list of `VPieceNode`s. Each node references a **modeling** object id (`NodePoint` etc.) which copies a draft `VGObject`.

`mainPathPoints(data)` walks nodes, samples curves, honors `GetReverse()` (`VGObject::GetReversePoints`). `isExcluded` skips a node from the cut without deleting it.

`GetUnitedPath` is used after `UnionTool` merges two pieces.

Path must be closed for SA and layout. The piece dialog (tabs: Properties, Paths, SeamAllowance, Labels, AnchorPoints, Grainline, Notches) is the editor.

---

## 4. Seam allowance

Three layers, all formula-driven:

| Level | Where | Role |
|-------|--------|------|
| Piece default | `getSeamAllowanceWidthFormula` | Global width |
| Per-node before/after | `VPieceNode` `FormulaSABefore` / `After` | Varies at corners (collar, hem) |
| Custom SA path | `CustomSARecord` + `VPiecePath` type `CustomSeamAllowance` | Non-parallel allowance (facing, foldback) |

`VAbstractPiece` builds an **equidistant** offset polygon (`seamAllowancePoints`). Join at vertices uses `PieceNodeAngle`:

| `PieceNodeAngle` | Behavior |
|------------------|----------|
| `ByLength` | Default offset join |
| `ByPointsIntersection` | Intersection of adjacent offset edges |
| `ByFirstEdgeSymmetry` / `BySecondEdgeSymmetry` | Symmetric corners |
| `ByFirstEdgeRightAngle` / `BySecondEdgeRightAngle` | Square corners |

Built-in SA can be toggled; layout uses either SA or main contour plus **layout gap** (separate from sewing SA).

---

## 5. Notches

On `VPieceNode`: `isNotch`, `NotchType`, `NotchSubType` (Straightforward / Bisector / Intersection), length, width, angle, count, offset, `showCutline` / `showSeamline`.

`VPiece::createNotchLines` emits `QVector<QLineF>` (and `NotchData`) for drawing and export (AAMA notch layer `"4"`).

Notches are **marks**, not DAG tools; they are attributes of the piece path so they grade with the outline.

---

## 6. Internal paths and anchors

- `InternalPathTool` + `VPiecePath` type `InternalPath` — pockets, drill holes, cutouts (`getInternalPaths()`).
- `AnchorPointTool` — points used to position float items (labels, grainline center/ends).

`InsertNodes` inserts additional nodes into an existing piece path (undo + lite parse).

---

## 7. Grainline

`VGrainlineData`: length, rotation, arrow type, visibility, anchors (center / top / bottom). On the piece scene: `VGrainlineItem` (movable). On layout/export: `VGraphicsFillItem` path.

Grainline is a **float item**: positioned in piece space, not a construction tool.

---

## 8. Labels

| Class | Role |
|-------|------|
| `VPieceLabelData` | Piece name, letter, annotation; font; placeholder tokens |
| `VPatternLabelData` | Pattern-level info (company, customer, date, size, measurement file name, …) |

`VTextManager` fits text into a box (font shrink). `VLabelTemplate` XML ([Module_FileIOParsing.md](./Module_FileIOParsing.md)) defines line layouts.

Interactive: `VTextGraphicsItem` + `ResizeHandle`. Export option **text-as-paths** outlines glyphs for cutters that do not have the user’s fonts.

Placeholders can include measurement file name/ext and current size/height.

---

## 9. Piece flags (layout + UI)

| Flag | Undo / API |
|------|------------|
| Include in layout | `TogglePieceInLayout`, `isInLayout` |
| Lock | `TogglePieceLock`, `isLocked` |
| Color | `SetPieceColor` |
| United | `SetUnited` after union |

`PiecesWidget` dock lists pieces and these flags.

---

## 10. Union

`UnionTool` merges two pieces along a shared edge. New SA/grainline defaults come from settings. Result is a new `VPiece` with a united path; originals may remain depending on the dialog. Referens on shared nodes is updated.

---

## 11. From piece to layout

```
PatternPieceTool / VPiece
    → MainWindowsNoGUI::preparePiecesForLayout
    → VLayoutPiece::Create(piece, tool->getData())
         main path, SA, notches, internals, labels, grainline
    → SetLayoutAllowancePoints()   // extra offset = layout gap
    → VLayoutGenerator
```

Piece-mode export can dump pieces **without** nesting (`--exportonlydetails`).

---

## 12. Why nodes copy draft objects

If the piece path referenced draft IDs directly, deleting a construction tool would silently break the cut. Modeling nodes:

- Increment referens on the draft tool
- Carry reverse/exclude/SA/notch **piece-specific** data the draft point must not own (the same point can appear on two pieces with different notches)

Related: [Module_DrawingTools.md](./Module_DrawingTools.md), [Module_LayoutNesting.md](./Module_LayoutNesting.md), [Module_ExportRender.md](./Module_ExportRender.md).
