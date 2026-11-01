# Seamly3D assembly preview

Seamly3D is an experimental companion application for assembling evaluated
Seamly2D sewing contours into an inspectable 3D surface. It uses Qt Widgets for the editor and Qt Quick 3D for the viewport,
with Metal on macOS and Qt's platform RHI backend elsewhere. No Blender
installation, third-party physics engine or network service is required. The pattern file and its formulas remain owned by Seamly2D.

## Try it

1. Build the repository normally. `src/app/app.pro` builds the companion alongside
   Seamly2D. On macOS the complete companion app is also copied into Seamly2D's
   `Contents/Helpers` directory, preserving its own Qt Quick frameworks and QML imports. On Linux it has its own `make install` target. Windows
   installers include `seamly3d.exe` beside `Seamly2D.exe`.
   Qt 6 with Qt Quick 3D, Quick Widgets, Qt QML and Shader Tools is required.
2. Open a pattern in Seamly2D and choose **File → Open in Seamly3D…**. Choose a new
   `.sm3d` destination. The current evaluated geometry, including unsaved edits,
   is exported. All pieces included in the layout are exported once. Create
   explicit pieces for mirrored/cut copies; label quantities are not expanded.
3. In Seamly3D open the **Seams** inspector and select pieces A and B. Click twice near each sewing contour to
   select continuous start/end positions, or use the numeric fields. Arcs follow increasing
   perimeter percentages, wrapping through zero. Equal endpoints select the entire perimeter.
   The green marker is the start; the red marker is the end.
4. Check the arc lengths, choose whether side B's correspondence is reversed,
   then **Add seam**. Select a saved seam to edit its name, endpoints, direction
   and **Include in assembly** state in place. **Apply changes** commits one undo
   command; **Revert changes** restores the stored relation. Valid edits also apply
   when saving or switching seams/tabs; invalid edits block that transition and
   leave the project unchanged. **New seam** returns to explicit creation mode.
   Paused seams remain saved but add no assembly constraints. Disjoint arcs on
   the same piece are supported; overlapping self-seams are rejected.
   Side A is a solid cyan guide and side B a dashed coral guide in both editors
   and the 3D view. The 3D guides show through geometry for inspection, follow the
   current preview and respect hidden pieces. They do not represent stitched mesh.
5. In the **Pieces** inspector use **Initial placement** to translate and rotate individual pieces.
   Use **Assemble** to run/pause the preview, **Reset assembly** to restore those
   placements, **Fit view** (F) to frame it, and **Mesh** to inspect triangles.
   Click a piece to highlight it and select it in the placement and A-side
   controls and open the Pieces inspector. Left-drag orbits; right/middle-drag pans; the wheel zooms and
   double-click fits the scene. The active graphics backend is shown in the status bar.
   There is no automatic
   inference of sewing relationships or initial spatial arrangement.
6. Save the assembly project. It stores contours, initial placements and seams.
   Reopening restores these inputs; press **Assemble** to regenerate the result.
   **Export current mesh as OBJ…** exports the currently displayed deformation
   in millimetres, with a separate object per piece. It does not weld seams or
   bake a calibrated physical garment.

The right inspector separates **Dress form**, **Pieces**, **Seams** and **Cloth**, preserving
selections when switching tabs. Drag its divider to resize it; each tab scrolls
independently in smaller windows. The native toolbar keeps file/view actions and
**Assemble** visible. The status bar shows piece/seam counts and current diagnostics;
**Experimental preview** opens the detailed limitations. The editor follows the
system light/dark appearance with its atelier palette, retaining the dark 3D stage.

The atelier palette uses ivory and deep green in light appearance and
graphite-green in dark appearance, with one family of authored vector icons.
The scene toolbar separates **Inspect**, **Move** and **Rotate**. Select a panel,
then drag an axis to translate its initial placement or a ring to rotate it.
The numeric placement fields remain synchronized. Starting a drag pauses and
resets the active preview to the initial placements; **Esc** cancels that
drag and restores its previous pose and modified state. Move axes are world
X/Y/Z; rotation rings follow the same Rz × Ry × Rx Euler convention as the numeric
fields. Edge-on axes/rings are hidden because their screen projection cannot
support reliable dragging; orbit the view or use the numeric fields instead.

The camera selector offers **Three-quarter**, **Front**, **Back**, **Right**,
**Left** and **Top**, preserving project geometry, zoom and pan. Keys **0–5** select
those views while the canvas has focus, so number entry in fields is unaffected.
**V**, **G** and **R** select Inspect, Move and Rotate. These are orthographic
views; no perspective projection is introduced by the three-quarter preset.

### History and piece inspection

**Edit → Undo / Redo**, also available in the document toolbar, uses the platform
shortcuts (Cmd+Z / Cmd+Shift+Z on macOS). History retains up to 50 project edits:
numeric placements, complete handle drags, dress-form changes, arrangement,
adding, editing, pausing or removing seams and importing updated geometry. A completed drag creates
one command; Esc and no-op drags create none. Number entry commits on editing
completion instead of recording each digit. Undo restores authored inputs and
regenerates the preview, not the preceding simulation frame. Camera navigation,
selection and simulation frames are outside document history.

Saving marks the current history position clean without discarding undo. Undo
away from it marks the document modified; redo back to it clears that flag.
Editing after undo discards the redo branch. Opening another project clears
history; geometry import is an undoable edit. Example projects remain unsaved
until explicitly saved. History is session-only and is not serialized in `.sm3d`.

The **Pieces** inspector starts with a checkable scene list synchronized with 3D
selection. Uncheck a row to hide it; **Isolate selected** hides the other pieces;
**Show all** restores their visibility. **Transparent selected piece** reduces
the selected surface opacity for inspection. Hidden selections keep their numeric
controls but have no draggable handles. Inspection changes do not dirty the
project, enter undo history, alter solver participation or filter OBJ export.
They survive ordinary project edits and undo/redo, and reset when opening a
project. Show all restores visibility and leaves transparency choices intact.

**File → Open assembly example** opens an editable two-panel pouch project. Its top remains open;
this is a solver demonstration rather than a validated sewing pattern.

To update a pattern, export from Seamly2D to a **different** `.sm3d` file, then
choose **File → Import updated geometry…** in the existing assembly. Placements
are preserved for matching piece IDs from the same source pattern. Seams survive
only when both evaluated contours are exactly unchanged. Changed/deleted pieces
invalidate associated seams, and importing a different source clears them.
Existing assembly files containing seams or authored dress form settings cannot
be overwritten by the exporter.
The source `.sm2d` is never written by Seamly3D.

## Dress form

New windows show a parametric dress form inspired by a conventional linen-covered
sewing mannequin: torso, short neck, metal collar, pole and base. There is no
external model download or asset dependency. Enable **Use dress form** in the Dress form inspector of an existing
project to display it; version-1 projects keep it disabled on opening.

The controls use centimetres for bust, waist, hips, neck circumference, shoulder
width, total torso height and contact clearance. Internal geometry remains in
millimetres. The waist plane is y=0, the front is +Z and the torso extends from
-0.42 to +0.58 times its height. Cross-sections are elliptical and interpolated
with shape-preserving cubic curves. Circumferences set the ellipse sizes; this
is a stylized adjustable form, not an anatomical reconstruction or an inference
of body shape from a photograph. Measurements do not automatically come from a
SeamlyMe file. The stand is visual only and can be hidden.

**Place selected piece** puts the placement-selected panel in front, right, back
or left of the form. **Arrange all pieces around form** distributes the panels
around it with an initial 60 mm gap and their upper edges near the shoulders.
These commands replace initial panel placements without changing sewing
relationships or the pattern's dimensions. Use the numeric placement controls
for fine adjustment, then **Assemble**. The arrangement is a starting point,
not an inferred garment construction.

**Basic body contact** checks cloth vertices, edge midpoints and triangle
centroids against the sampled elliptical loft after each solver iteration,
including neck and bottom caps and the selected gap. Rendering and contact share
the same profile. This is sampled contact, not continuous triangle collision:
faces/edges can still cross the form between checked points,
large steps may tunnel, and fabric can self-intersect. Geometric assembly has no gravity or friction; the separate physical mode
adds gravity and approximate contact friction. Neither mode implements stuffing
or calibrated draping. A small seam gap does not
prove fit. Initial intersections are not automatically repaired correctly; use
the arrangement controls before assembling.

The project extension is `.sm3d`, consistent with Seamly2D `.sm2d`.

Projects are saved in `.sm3d` version 4; versions 1, 2, 3 and 4 can be loaded.
Version 2 introduced mannequin settings; version 3 adds seam identity, names and
active state. Legacy seams receive deterministic identities and names and remain
active. Version 4 stores continuous arc fractions, mesh edge size, cloth parameters
and pin flags. Older Seamly3D builds reject version 4 rather than discarding data.
A form-only project can be saved with no cloth pieces. Importing updated pattern
geometry preserves the current form. OBJ export continues to export cloth only.

**File → Open dress form example**, or `--dress-form-demo`, opens two rectangular
panels with side seams around the form. This is an editable contact demonstration,
not a validated garment pattern. The ordinary **Example** remains the small pouch.

## What this milestone establishes

- A native companion with direct launch from Seamly2D and versioned file exchange.
- Evaluated, physical-scale sewing contours, distinct from cut/seam-allowance edges.
- Concave-polygon meshing, manual arc pairing and positional assembly constraints.
- Piece placement, interactive 3D inspection, persistence and mesh export.
- Mean sewing gap and mean relative mesh-edge strain. A low gap can coexist with
  substantial distortion: neither metric establishes fit or sewing correctness.

The default **Geometric assembly** mode relaxes positions without gravity.
**Cloth → Physical draping · experimental** runs a separate CPU XPBD backend:
fixed 1/60 s frames, four substeps, eight constraint iterations per substep,
SI-unit gravity, area-weighted mass, triangle-edge stretch/shear constraints,
approximate opposite-vertex bending, seam constraints, sampled body contact,
floor contact, damping and approximate tangent friction. Metal remains the renderer;
the physics is CPU code, not a native Apple cloth simulation service.
The formulation follows [Macklin et al., XPBD (2016)](https://mmacklin.com/xpbd.pdf),
implemented locally without copying a third-party engine.

In **Cloth**, set maximum triangle edge length (5–100 mm) and inspect vertex/triangle
counts. Changing resolution rebuilds the preview and is undoable. The mesher keeps
original corners and exact authored seam endpoints, then splits long edges with
conforming shared midpoints. Seam endpoints and undeformed lengths remain invariant
across densities (tested to 0.001 mm for positions). This is not an aspect-ratio or
minimum-angle guarantee: thin contours may yield slender triangles or an explicit
meshing error. Original outlines remain unchanged in the file. Clicking a contour
projects onto its original segments rather than snapping to a 64-point grid.
Percentage fields allow six decimal places; serialized fractions use doubles.

**Fix top centre during draping** in Pieces holds the middle half of the highest
original edge at its initial placement; top corners remain free for sewing. If the
highest contour feature is a single point, that point is fixed. At least a midpoint
is inserted for a horizontal top edge even on a coarse mesh. Pins are project inputs,
not inspection. Reset restores initial placement and zero velocities. Pinning
incompatible positions can conflict with seams/body contact; inspect strain and gap.
**File → Open draping example** opens synthetic pinned panels for this mode.

Parameters are not calibrated fabric presets. There are no self-collisions,
continuous collision detection, fabric layers, imported avatars, pressure or stuffing.
Surfaces can intersect, tunnel or collapse, especially with poor placement or
conflicting constraints. The app cannot predict fit or replace a physical toile.
Draping stops after one second below a 2 mm/s motion threshold, at 20 simulated
seconds, or on numerical instability. Low motion is not proof of constraint
convergence. A time-limit stop explicitly says it is not convergence. Simulated
time, maximum speed and stop reason are visible in Cloth; Reset permits a new run.
The fixed simulation clock may advance slower than wall time on dense projects.

Contours with built-in seam allowance are rejected because a separate sewing
line cannot be determined reliably. Holes and internal sewing paths are outside
this milestone. Projects are limited to 32 cloth pieces, 2,048 original contour points
per piece, 256 seams and an 8 MB input file. Meshing accepts up to 512 combined
original corners and authored endpoints per piece. Runtime budgets are 12,000
vertices/24,000 triangles per piece, 40,000 vertices/80,000 triangles per project
and 32,768 stitches. Budget failures leave the current assembly unchanged.

## Architecture

`model.{h,cpp}` is compiled into both applications: millimetre-based exchange,
transactional validation, atomic saves, contour resampling, deterministic meshing
and the geometric preview solver. `clothsolver.{h,cpp}` holds the separate
physical backend; both modes use the same mesh and seam anchors. Seamly2D's `seamly3d_export.cpp` obtains `MainPathPoints`
from the existing `VContainer`; it does not parse or re-evaluate pattern formulas.
The source uses y-down pixel coordinates, converted once to millimetres with y up.

`Assembly` uses original-outline arc fractions, boundary index/parameter maps and
interpolated boundary anchors. Refinement updates those maps, so rendering and both
solvers follow the same authored paths. All edges share subdivision vertices;
constraint ordering is deterministic. Geometric assembly stops after 2,400
iterations per run; physical stopping is based on its separate simulation clock.
`Viewport` embeds a QQuickWidget running `AssemblyView.qml`. `ScenePiece` uploads
accepted positions, area-weighted vertex normals, triangle indices, wireframe
edges and sewing tubes through QQuick3DGeometry. Stable render objects are
retained while positions change. Qt Quick 3D provides a depth buffer, two-sided
rough materials, lighting, shadows and triangle picking. QPainter remains only
in the 2D contour selectors. Selection, camera and render buffers do not modify
the project or the solver. The floor is visual and has no physical collision.

On macOS the application requests Metal unless `QSG_RHI_BACKEND` explicitly
selects another Qt RHI backend. Other platforms use Qt's default. A renderer
startup failure is displayed, and a software/null backend cannot be reported as
a successful 3D capture. Missing graphics support does not affect `--validate`,
which creates no viewport. `--render` waits up to ten seconds for an actual frame
and returns a nonzero exit status if initialization or image writing fails.

The `.sm3d` JSON format has `format: "seamly3d"`, `version: 4`, `units: "mm"`,
`source`, `pieces`, `seams`, and `dressForm`. Pieces have stable string IDs, names, original
outlines (`[[x,y],…]`), translations and Euler rotations (degrees, X then Y then Z).
Seams reference IDs and `startA/endA/startB/endB` as normalized arc-length fractions in `[0,1)`, plus a `reverse`
boolean. Each seam also has a unique `id`, a nonempty `name` (up to 128
characters) and an `enabled` boolean. `meshSize` is millimetres; `simulation`
stores mode, density, compliances, damping and friction. Each piece has `pinTop`.
Legacy integer indices migrate by dividing by 64; identity and direction remain
unchanged. Geometry imports retain seams only for unchanged outlines and flag
removed relationships in the existing import summary. The closing outline point is implicit. Invalid files fail before any
current project or mesh is replaced. There is no live socket or automatic
round-trip modification of the pattern.

## Build and validation

Standalone companion (from the repository root):

```sh
mkdir -p build/seamly3d
cd build/seamly3d
qmake ../../src/app/seamly3d/seamly3d.pro
make -j4
```

Tests, including editor interactions (from the repository root):

```sh
mkdir -p build/seamly3d-test
cd build/seamly3d-test
qmake ../../src/test/Seamly3DTest/Seamly3DTest.pro
make -j4
QT_QPA_PLATFORM=offscreen bin/Seamly3DTests
# On a workstation with a graphics session (Metal on macOS):
SEAMLY3D_GPU_TESTS=1 bin/Seamly3DTests
```

On macOS the standalone executable is
`build/seamly3d/bin/seamly3d.app/Contents/MacOS/seamly3d`.
Use `--demo` to open the sample, `--validate project.sm3d` for a noninteractive
validation/meshing check, and `--steps 1000` to exercise constraint convergence.
`--render preview.png --steps 200 --demo` captures the application window and
exits; this requires a graphics session. CPU validation and the default test
suite can run with `QT_QPA_PLATFORM=offscreen`. Renderer/editor GPU acceptance is
explicitly opt-in and includes occlusion, triangle picking, camera controls,
mesh buffers, retained selection and live editor updates. Headless execution
skips the native renderer/editor cases instead of treating a blank image as
a successful render.

The macOS deploy step scans the companion's QML directory as well as its linked
frameworks. Windows staging includes the companion's deployed QML/plugins, and
AppImage deployment points its QML scanner at the same source directory. The
local Homebrew Qt Quick libraries currently target macOS 27; this local build
therefore does not establish compatibility with older macOS releases despite
the repository's lower default deployment target. Release builds must use a Qt
SDK built for their intended minimum macOS version.

Next milestones should add named source-edge references, mesh-quality improvement,
continuous/self-collision handling, fabric layers and validated material presets. The companion boundary allows that backend to evolve independently.

## Compatibility note for the initial branch

This increment starts from `main`, whose pattern reader supports `.sm2d` through
0.7.4. Files produced by a newer branch (for example 0.7.6) must be evaluated by
that newer reader before exporting their geometry; the companion does not
silently downgrade them. Existing in-app connections can supply evaluated sewing
contours through their read-only piece APIs for development checks. The `.sm3d`
exchange itself is independent of the pattern XML version.

The macOS bundles include a [Qt path configuration](https://doc.qt.io/qt-6/qt-conf.html);
the nested companion retains its own framework and QML deployment. Windows installer
workflows include its executable; Linux installs it alongside the other binaries.
Windows/Linux packaging and macOS DMG creation require validation on their release
environments; local verification covers macOS source builds and runtime checks.

## File-based demo suite

Twelve synthetic v4 projects and an English test guide are available in
[the sample directory](../share/samples/seamly3d/README.md). They cover free fall,
pins, sewing, dress-form contact, two mesh densities, four panels, a concave outline,
a deliberately incorrect sewing direction and rounded endpoints near corners.
The sample report records solver checkpoints and native Metal opening checks.
