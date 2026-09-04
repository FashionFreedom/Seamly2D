# Module: Geometry Engine

Parent: [Architecture.md](./Architecture.md) · Curves: [Module_BezierMath.md](./Module_BezierMath.md)

`src/libs/vgeometry` is **pure geometry**: no XML, no widgets, no formulas. Tools own IDs and formulas; this library stores points and curves as implicitly shared value types and offers construction/intersection helpers.

---

## 1. Why a separate library

Draft, piece modeling, visualization, layout, and DXF export all need the same primitives. Keeping math here means:

- `QSharedDataPointer` copies are cheap when tools snapshot `VContainer`
- Layout can offset a path without depending on `VTool*`
- Unit tests (`Seamly2DTest`) can exercise intersections without a GUI

---

## 2. Class hierarchy

```
VGObject                          // id, name, GOType, Draw mode, static vector math
├── VPointF                       // QPointF + label offsets mx/my + show-name
└── VAbstractCurve                // getPoints(), GetLength(), IntersectLine(), GetPath()
    ├── VAbstractArc
    │   ├── VArc                  // center + radius + f1/f2 or length
    │   └── VEllipticalArc        // rx, ry, rotation, f1/f2
    └── VAbstractBezier
        ├── VAbstractCubicBezier  // P1..P4; Hobby; CutSpline
        │   ├── VSpline           // angle / kAsm or C1/C2 length formulas
        │   └── VCubicBezier      // four VPointF anchors
        └── VAbstractCubicBezierPath
            ├── VSplinePath       // QVector<VSplinePoint>
            └── VCubicBezierPath
```

Supporting: `VSplinePoint` (point + handle angle/length formulas).

`GOType` (`vgeometrydef.h`): `Point`, `Arc`, `EllipticalArc`, `Spline`, `SplinePath`, `CubicBezier`, `CubicBezierPath`, plus filters `Curve`, `Path`, `AllCurves`.

`Draw` mode on the object: `Calculation` (draft) | `Modeling` (piece nodes) | `Layout`. Same math, different presentation stage.

Pattern: **Prototype / value object** with Qt implicit sharing (`VGObjectData`, `VPointF` private d-pointer, etc.). Virtual `CreateName()` on curves builds history-friendly names.

---

## 3. `VGObject` static algorithms

These are the constructive-geometry kernel used by point tools:

| API | Purpose |
|-----|---------|
| `BuildLine(p1, length, angle)` | Polar segment |
| `BuildRay` / `BuildAxis` | Infinite axis clipped to scene rect (intersect tools) |
| `IntersectionCircles(c1,r1,c2,r2)` | 0–2 points |
| `LineIntersectCircle` | Line–circle |
| `ContactPoints(p, center, r)` | Tangents from a point to a circle |
| `ClosestPoint(line, point)` | Foot of perpendicular |
| `addVector(p, p1, p2, k)` | `p + k * (p2-p1)` |
| `LineCoefficients` | Implicit line `ax+by+c=0` |
| `IsPointOnLineSegment` | Segment membership |
| `IsPointOnLineviaPDP` | Perp-dot product colinearity with `accuracyPointOnLine` |
| `GetReversePoints` | Reverse polyline (piece path reverse flag) |
| `GetLengthContour` | Polyline length |
| `flipTransform(axis)` | Mirror matrix (operations + layout forbid-flip) |
| `LineIntersectRect` | Clip helpers |

`PerpDotProduct` / `GetEpsilon` / `PointInCircle` are private.

**Why static:** no instance state; tools and visualization call the same functions. Numerical tolerance is centralized (`accuracyPointOnLine`) so “on the line” is consistent across tools.

---

## 4. Points — `VPointF`

Wraps `QPointF` plus:

- `mx`, `my` — label offset in scene coords (user-draggable; undo `MoveLabel`)
- show-name flag
- `idObject` — for nodes, the draft object this modeling point copies

Tools create a `VPointF`, `VContainer::AddGObject`, and often `AddLine(p1,p2)` which synthesizes `Line_*` and `AngleLine_*` variables ([Module_FormulaEngine.md](./Module_FormulaEngine.md)).

---

## 5. Arcs

### Circular `VArc`

Defined by center, radius, start angle `f1`, end angle `f2` **or** by arc length (`VToolArcWithLength`). Internally still a circular arc; length mode computes the second angle.

`VAbstractCurve` samples to a polyline for length, intersection, and `QPainterPath`.

### Elliptical `VEllipticalArc`

Two radii, rotation of axes, `f1`/`f2`. Used less often than circular arcs but first-class (`Tool::EllipticalArc`, node `VNodeEllipticalArc`).

---

## 6. Curve protocol (`VAbstractCurve`)

Shared operations all curves must support:

- `getPoints()` — polyline approximation
- `GetLength()` — arc length of that polyline (cubics also have analytic/GL length; see Bézier module)
- `IntersectLine` / `CurveIntersectAxis` — ray vs sampled polyline
- `GetPath()` — `QPainterPath` for the scene
- direction arrows for visualization

Intersections with “true” cubics are **not** solved as cubics. They are solved against the **flattened polyline**. That is a deliberate trade: robust, matches what is drawn, slightly approximation-dependent. Flattening quality is the Bézier subdivision depth ([Module_BezierMath.md](./Module_BezierMath.md)).

---

## 7. Cuts (point on curve at length)

`CutSpline`, `CutArc`, `CutSplinePath` tools call:

- `VAbstractCubicBezier::CutSpline(length, …)` — splits a cubic at arc-length parameter *t* (`GetParmT` + de Casteljau-style handle split)
- Arc cut: interpolate angle from length / radius

The result is a new `VPointF` plus (for splines) two new cubics’ control points so the original curve can be replaced by two segments in history.

---

## 8. How tools consume geometry

Example — line ∩ circle (`VToolPointOfContact` / `IntersectCirclesTool`):

1. Resolve parent point IDs from the container snapshot.
2. Evaluate radius/length **formulas** to `qreal`.
3. Call `VGObject::IntersectionCircles` / `LineIntersectCircle` / `ContactPoints`.
4. Disambiguate with `CrossCirclesPoint` / `VCrossCurvesPoint` / `HCrossCurvesPoint` enums stored in XML (first/second, highest/lowest, leftmost/rightmost).
5. `AddGObject` the resulting `VPointF`.

The geometry library never sees formulas. That split keeps it testable.

---

## 9. Modeling copies

Piece nodes (`VNodePoint`, `VNodeArc`, …) copy draft `VGObject`s into `Draw::Modeling` mode. They share the same classes; `idObject` points at the draft original. Reverse flags on `VPieceNode` use `GetReversePoints`.

---

## 10. File map

| File | Content |
|------|---------|
| `vgobject.*` | Base + static constructions |
| `vpointf.*` | Points |
| `vabstractcurve.*` | Curve protocol |
| `vabstractarc.*`, `varc.*`, `vellipticalarc.*` | Arcs |
| `vabstractbezier.*`, `vabstractcubicbezier.*` | Cubics (see Bézier module) |
| `vspline.*`, `vcubicbezier.*`, `vsplinepath.*`, `vcubicbezierpath.*` | Concrete curves |
| `vgeometrydef.h` | `Draw`, `GOType` |

Related: [Module_BezierMath.md](./Module_BezierMath.md), [Module_DrawingTools.md](./Module_DrawingTools.md), [Module_PatternPieces.md](./Module_PatternPieces.md).
