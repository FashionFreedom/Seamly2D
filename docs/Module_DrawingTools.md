# Module: Drawing Tools Catalog

Parent: [Architecture.md](./Architecture.md)

`src/libs/vtools` is the **instruction** surface: every construction the user can place. The `Tool` enum in `vmisc/def.h` has sentinel `LAST_ONE_DO_NOT_USE == 54`. Abstract holders are not user-facing.

Each real tool: XML factory in `VPattern::ParseTool*`, a `VTool*` class, a dialog, a `VisTool*` preview, and usually a property-browser mapping.

---

## 1. Tool class hierarchy

```
QObject
└── VDataTool                         // VContainer snapshot + _referens
    └── VAbstractTool                 // doc, id, CheckFormula, AddRecord, vis
        ├── VInteractiveTool          // DialogTool
        │   ├── VDrawTool             // QGraphicsItem on draft scene
        │   │   ├── VAbstractPoint
        │   │   │   ├── VToolSinglePoint
        │   │   │   │   ├── VToolBasePoint
        │   │   │   │   ├── VToolLinePoint → EndLine, AlongLine, Normal, …
        │   │   │   │   ├── VToolCut → CutSpline, CutArc, CutSplinePath
        │   │   │   │   └── intersections, triangle, contact, …
        │   │   │   └── VToolDoublePoint → VToolTrueDarts
        │   │   ├── VToolLine
        │   │   ├── VAbstractSpline → Arc, Spline, CubicBezier, paths, ElArc
        │   │   └── VAbstractOperation → Move, Rotation, Mirror*
        │   ├── PatternPieceTool
        │   └── VAbstractNode → VNodePoint/Arc/Spline/…
        └── UnionTool
```

**Template Method:** `static Create(..., Document parse)`. FullParse builds the item and `AddRecord`; LiteParse updates geometry only.

**Command:** creation goes through `AddToCalc` undo ([Module_UndoCommands.md](./Module_UndoCommands.md)).

---

## 2. Abstract / UI-only enum values (not stored as that type)

| Enum | Meaning |
|------|---------|
| `Arrow` | Cursor / select mode |
| `SinglePoint`, `DoublePoint`, `LinePoint`, `AbstractSpline`, `Cut` | Base classes |
| `Midpoint` | UI alias; stored as `AlongLine` |
| `ArcIntersectAxis` | UI alias; stored as `CurveIntersectAxis` |
| `LAST_ONE_DO_NOT_USE` | Sentinel (must stay last) |

---

## 3. Point tools — single point

| Enum | Class | Construction |
|------|--------|----------------|
| `BasePoint` | `VToolBasePoint` | Origin of a draft block (no parents) |
| `EndLine` | `VToolEndLine` | Base + **length** + **angle** formulas |
| `AlongLine` | `VToolAlongLine` | Point on P1–P2 at length (midpoint = half) |
| `ShoulderPoint` | `VToolShoulderPoint` | Shoulder / dart-style third point |
| `Normal` | `VToolNormal` | Perpendicular from a line at length |
| `Bisector` | `VToolBisector` | Angle bisector from three points |
| `Height` | `VToolHeight` | Foot of perpendicular to a line |
| `LineIntersect` | `VToolLineIntersect` | Two lines (four points) |
| `LineIntersectAxis` | `VToolLineIntersectAxis` | Line ∩ axis from point+angle |
| `CurveIntersectAxis` | `VToolCurveIntersectAxis` | Curve ∩ axis |
| `PointOfIntersection` | `PointIntersectXYTool` | X of P1, Y of P2 |
| `PointOfContact` | `VToolPointOfContact` | Circle–line contact |
| `PointOfIntersectionArcs` | `VToolPointOfIntersectionArcs` | Two arcs |
| `PointOfIntersectionCircles` | `IntersectCirclesTool` | Two circles (radii formulas) |
| `PointOfIntersectionCurves` | `VToolPointOfIntersectionCurves` | Two curves |
| `PointFromCircleAndTangent` | `IntersectCircleTangentTool` | Tangent from point to circle |
| `PointFromArcAndTangent` | `VToolPointFromArcAndTangent` | Tangent to arc |
| `Triangle` | `VToolTriangle` | Third vertex of a right triangle |
| `CutSpline` | `VToolCutSpline` | Point at length along spline |
| `CutSplinePath` | `VToolCutSplinePath` | Point at length along path |
| `CutArc` | `VToolCutArc` | Point at length along arc |

Disambiguation enums (stored in XML): `CrossCirclesPoint`, `VCrossCurvesPoint`, `HCrossCurvesPoint`, `AxisType` (`vabstractpattern.h`).

After creating two points, many tools call `VContainer::AddLine` so `Line_*` / `AngleLine_*` become formula tokens.

---

## 4. Double point

| Enum | Class | Role |
|------|--------|------|
| `TrueDarts` | `VToolTrueDarts` | Dart apex pair (two new points) |

---

## 5. Line

| Enum | Class | Role |
|------|--------|------|
| `Line` | `VToolLine` | Segment between two **existing** points (does not create a point) |

---

## 6. Curves and arcs

| Enum | Class | Geometry |
|------|--------|----------|
| `Spline` | `VToolSpline` | `VSpline` |
| `CubicBezier` | `VToolCubicBezier` | `VCubicBezier` |
| `SplinePath` | `VToolSplinePath` | `VSplinePath` |
| `CubicBezierPath` | `VToolCubicBezierPath` | `VCubicBezierPath` |
| `Arc` | `VToolArc` | `VArc` by angles |
| `ArcWithLength` | `VToolArcWithLength` | `VArc` by length |
| `EllipticalArc` | `VToolEllipticalArc` | `VEllipticalArc` |

Math: [Module_BezierMath.md](./Module_BezierMath.md), [Module_GeometryEngine.md](./Module_GeometryEngine.md).

---

## 7. Operations (transform a selection)

| Enum | Class | Effect |
|------|--------|--------|
| `Rotation` | `VToolRotation` | Rotate copies around a point by formula angle |
| `MirrorByLine` | `VToolMirrorByLine` | Reflect across a line |
| `MirrorByAxis` | `VToolMirrorByAxis` | Reflect across H/V axis through a point |
| `Move` | `VToolMove` | Translate by polar or Cartesian formulas |

Base: `VAbstractOperation` / `VAbstractMirror`. They produce **prefixed copies** of selected objects (new IDs, `GenerateSuffix`) so the originals stay. Operation objects increment referens on sources.

---

## 8. Pieces / modeling (see also [Module_PatternPieces.md](./Module_PatternPieces.md))

| Enum | Class | Role |
|------|--------|------|
| `Piece` | `PatternPieceTool` | Closed piece on piece scene |
| `InternalPath` | `InternalPathTool` | Internal / cutout / custom SA path |
| `AnchorPoint` | `AnchorPointTool` | Anchor for labels/grainline |
| `NodePoint` | `VNodePoint` | Modeling copy of a draft point |
| `NodeArc` | `VNodeArc` | |
| `NodeElArc` | `VNodeEllipticalArc` | |
| `NodeSpline` | `VNodeSpline` | |
| `NodeSplinePath` | `VNodeSplinePath` | |
| `Union` | `UnionTool` | Unite two pieces along a shared edge |
| `InsertNodes` | dialog-driven | Insert nodes into an existing piece path |

---

## 9. Groups and images

| Enum | Implementation |
|------|----------------|
| `Group` | XML groups in `VAbstractPattern` + undo `AddGroup` / `AddGroupItem` / `MoveGroupItem` / `DelGroup` |
| `BackgroundImage` | `src/libs/tools` `ImageTool` + `ImageItem` + `AddImage` undo |

These do not feed `Calculator`.

---

## 10. Visualization (placement preview)

`Visualization` base (`Mode::Creation` | `Show`):

- `visualization/line/` — `VisLine`, `VisToolEndLine`, `VisToolAlongLine`, `VisToolMove`, …
- `visualization/path/` — splines, arcs, cubics, `PatternPieceVisual`, `InternalPathVisual`

`RefreshGeometry()` uses current mouse `scenePos` + selected IDs + `FindLength`/`FindVal` on formulas. Tools hold `QPointer<Visualization> vis` and `ShowToolVisualization<VisT>(show)`.

`Vis` enum in `def.h` tags graphics item types (scaled line/ellipse, control-point handles, …).

---

## 11. Dialogs

One `Dialog*` per tool under `vtools/dialogs/tools/` (AlongLine, Arc, Bisector, PatternPiece, Union, Rotation, Mirror*, Move, TrueDarts, InternalPath, AnchorPoint, InsertNodes, group dialogs, …). Support dialogs: `EditFormulaDialog`, `EditLabelTemplateDialog`, `DialogUndo`.

Interactive creation: dialog collects IDs + formulas → undo `AddToCalc` inserts XML → Full or Lite parse instantiates the tool.

---

## 12. Why this many tools instead of a generic “constraint”

Each tool is a **named construction** a patternmaker already knows (bisector, true darts, shoulder point). A generic “distance + angle” tool would be `EndLine`. Special tools encode textbook recipes and pick the correct intersection branch (`CrossCirclesPoint`, etc.) so the DAG stays stable when measurements grade.

Related: [Module_DependencyTree.md](./Module_DependencyTree.md), [Module_PatternPieces.md](./Module_PatternPieces.md).
