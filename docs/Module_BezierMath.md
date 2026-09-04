# Module: Bézier and Spline Math

Parent: [Architecture.md](./Architecture.md) · Primitives: [Module_GeometryEngine.md](./Module_GeometryEngine.md)

Cubic curves are how Seamly2D draws fashion lines (armholes, necklines, princess seams). The implementation is a mix of **classic cubic Bézier flattening**, **arc-length parameterization**, and **Hobby-style handle lengths** so patternmakers can work in angles and “strength” rather than raw control points.

---

## 1. Why cubics, not NURBS or clothoids

- Qt `QPainterPath` and DXF already speak cubics / polylines.
- Patternmaking needs **G1-ish** fashion curves, not CAD NURBS weights.
- Every numeric field is a **formula**; four points + two angles + two lengths is already a lot of tokens.

NURBS would complicate formulas, XML, and export with little drafting benefit.

---

## 2. Types

| Class | User tool | Definition |
|-------|-----------|------------|
| `VSpline` | `VToolSpline` | Endpoints + start/end **angle** + `kAsm1`/`kAsm2` or explicit `c1`/`c2` lengths |
| `VCubicBezier` | `VToolCubicBezier` | Four `VPointF` (P1, C1, C2, P4) |
| `VSplinePath` | `VToolSplinePath` | Polyline of `VSplinePoint` (each has angles + handle lengths) |
| `VCubicBezierPath` | `VToolCubicBezierPath` | Chain of four-point cubics |

All inherit `VAbstractCubicBezier` or `VAbstractCubicBezierPath`.

`VSpline` is the “patternmaker spline”: you pick two points and pull handles by **angle** (tangent direction at the endpoints) and **length** (how far the control point sits). `VCubicBezier` is the illustrator-style four-click curve.

---

## 3. Flattening — `PointBezier_r`

`VAbstractCubicBezier::PointBezier_r` recursively subdivides a cubic until it is flat enough, emitting polyline vertices (`px`, `py`).

`GetCubicBezierPoints(p1,p2,p3,p4)` is the public wrapper. `VAbstractCurve::getPoints()` uses this so:

- Length ≈ sum of chord lengths (`LengthBezier`)
- Intersections = polyline ∩ line/curve
- Display = `QPainterPath` through the samples

**Why recursive subdivision (de Casteljau-style) rather than fixed N samples:** curvature varies; a neckline needs more points at the tight end. The `level` argument caps recursion.

There is also `CubicBezierLengthGL` — Gauss–Legendre-style length of the true cubic, used when a more accurate length is needed than the polyline (handle solvers, `GetParmT`).

---

## 4. Arc-length parameter *t*

Cubic Bézier is parameterized by *t* ∈ [0,1] in **parameter space**, not distance. Cutting “3 cm along the curve” requires inverting length:

- `LengthT(t)` — length from 0 to *t*
- `GetParmT(length)` — find *t* such that `LengthT(t) ≈ length` (search)

`CutSpline(length, spl1p2, spl1p3, spl2p2, spl2p3)` uses that *t* and splits the cubic into two cubics (standard Bézier subdivision), returning the split point.

`VToolCutSpline` / `VToolCutSplinePath` expose this as a construction tool.

---

## 5. Hobby handle lengths

`HobbyHandleLengths(p1, p4, angle1Deg, angle2Deg, tensionStart=1, tensionEnd=1)`

Given endpoints and tangent **angles** (degrees), estimate control-point distances using John Hobby’s MetaPost-style heuristic (tension defaults 1.0). This is what makes “angle + strength” splines look fair without the user placing C1/C2 in the plane.

`SolveHandleLengths` / `SolveHobbyTension` invert the problem: given a **target length in pixels/units**, solve for handle lengths or tensions (`mode` selects which unknown). Used when the UI or a formula specifies curve length rather than handle length.

**Why Hobby:** it is the standard “nice cubic from tangents” in 2D drawing; cheaper and more stable than fitting a true Euler spiral, and it stays a cubic (exportable).

---

## 6. `VSpline` vs `VCubicBezier` (why both)

| | `VSpline` | `VCubicBezier` |
|--|-----------|----------------|
| Control | Angles + kAsm/C lengths at existing points | Four independent points |
| Formulas | `angle1`, `angle2`, `c1Length`, `c2Length` | Coordinates of C1/C2 as points (which themselves have history) |
| Typical use | Armhole between two known block points | When control points must snap to other constructions |

Paths (`VSplinePath`) join multiple `VSpline` segments. Continuity is whatever the shared point’s outgoing/incoming angles specify — not automatically C2.

---

## 7. Derived variables

When `VContainer::AddSpline` / `AddCurveWithSegments` runs, it registers:

| Variable | Prefix / name |
|----------|----------------|
| Total length | curve object name |
| Segment length | `name_segN` |
| Handle lengths | `c1Length_`, `c2Length_` |
| Handle angles | `angle1_`, `angle2_` |

So later formulas can say “half the armhole” without re-measuring. See [Module_FormulaEngine.md](./Module_FormulaEngine.md).

---

## 8. Visualization and handles

`VControlPointSpline` (`vwidgets`) draws C1/C2. Visualization classes `VisToolSpline`, `VisToolCubicBezier`, `VisToolSplinePath`, `VisToolCubicBezierPath` live under `vtools/visualization/path/`. While placing, they evaluate formulas via `FindLength`/`FindVal` and rebuild the cubic each mouse move.

Dragging a handle writes a `MoveSpline` / `MoveSplinePath` undo command (updates XML, lite parse).

---

## 9. Intersections

`VToolPointOfIntersectionCurves` and `VToolCurveIntersectAxis` intersect **flattened** polylines (`VAbstractCurve::CurveIntersectAxis`). Ambiguity (several hits) uses `VCrossCurvesPoint` / `HCrossCurvesPoint` (highest/lowest, leftmost/rightmost).

This can miss a glancing true-cubic intersection that the polyline skipped; in practice subdivision is dense enough for garment-scale curves.

---

## 10. Design summary

| Choice | Why |
|--------|-----|
| Flatten then intersect | Robust, matches pixels, simple |
| Hobby lengths | Fair curves from angles; patternmaker UX |
| Dual spline/cubic-bezier tools | Two authoring styles, one math core |
| Arc-length cut | Seam notches and “point at 3 cm along armhole” |

Related: [Module_DrawingTools.md](./Module_DrawingTools.md) (tool wrappers), [Module_ExportRender.md](./Module_ExportRender.md) (paths become DXF polylines / SVG).
