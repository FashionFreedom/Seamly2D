# Seamly suite product context

<!-- impeccable:product-schema 1 -->

## Platform

adaptive

Native Qt desktop applications for macOS, Windows and Linux. Here adaptive means
desktop OS conventions, not iOS or Android. Seamly2D and SeamlyMe use Qt Widgets;
Seamly3D combines Qt Widgets and Qt Quick 3D. Existing Seamly3D review captures
verify macOS; they do not verify the other products or platforms.

## Users

People creating sewing patterns, maintaining body measurements and inspecting
assembled garment pieces. These tasks are evidenced by the applications. Audience
experience, commercial positioning and product-specific accessibility needs remain
unspecified.

## Product Purpose

The repository contains three complementary products, registered in
`src/app/app.pro`:

| Product | Responsibility | Source | Surface brief |
| --- | --- | --- | --- |
| Seamly2D | Parametric pattern drafting, pattern pieces and print layouts | `src/app/seamly2d` | `.impeccable/surfaces/seamly2d.md` |
| SeamlyMe | Individual and multisize measurements, formulas and measurement metadata | `src/app/seamlyme` | `.impeccable/surfaces/seamlyme.md` |
| Seamly3D | Editable three-dimensional assembly preview with explicit seams and dress form | `src/app/seamly3d` | `.impeccable/surfaces/seamly3d.md` |

Success means completing each task with explicit dimensions, traceable inputs and
visible results. This product record and DESIGN.md cover all three products, as
requested by the user. Surface-specific interaction contracts live in their briefs.

## Operating Context

Desktop keyboard and mouse, local documents, precise numeric dimensions, native
file dialogs, menus and Qt translation infrastructure. Measurement names and
formulas connect measurement data to parametric pattern construction. Evaluated
Seamly2D contours can be exported to the separate Seamly3D preview. Preserve units
and the meaning of data across these boundaries.

## Capabilities and Constraints

### Seamly2D

Preserve parametric drafting tools, points, lines, curves and arcs, formulas and
measurement references, pattern-piece editing, groups, tool properties, print
layouts, document operations, undo/redo, printing and existing exports. Preserve
the current mode-dependent actions and dockable desktop workspace. The working
tree also contains the Seamly3D export integration in `seamly3d_export.cpp`.

### SeamlyMe

Preserve individual and multisize measurement files (`.smis` and `.smms`), legacy
`.vit`/`.vst` import/conversion, known and custom measurements, formulas, values,
base size/height and increments, search, row ordering, measurement diagrams,
Information metadata, read-only handling, templates and existing import/export
commands including CSV export. Keep individual and multisize semantics distinct.
Personal metadata is user data, not demonstration content.

### Seamly3D

Preserve opening/saving/importing local `.sm3d` projects, OBJ export, examples,
adjustable body measurements, stand/contact controls, panel placement, seam
creation/editing/removal, persistent seam names and enabled state, assembly/pause/reset, camera navigation, picking, wireframe and
diagnostic metrics. The default solver is geometric relaxation. A separate experimental CPU XPBD
mode adds gravity, area mass, stretch/shear, approximate bending and sampled body/
floor contact. Neither mode is calibrated cloth simulation. Self-collision and
fabric layers remain unsupported. The dress-form example is synthetic,
not a fitted or validated garment.

Seamly3D also provides session undo/redo for authored inputs and a synchronized
scene list with visibility, isolation and inspection transparency. Inspection
does not alter project data, solver participation or mesh export. The seam editor
provides transactional Apply/Revert drafts and linked solid-A/dashed-B guides.
Guides intentionally show through geometry; pausing a saved seam excludes its
constraints from assembly. Projects save as v4 and read v1/v2/v3/v4. Seam endpoints are continuous original-
outline arc fractions, independent of adaptive mesh edge size (5–100 mm). The
Cloth tab exposes resolution, physical mode/parameters and simulation status.
Pieces supports top-centre pins; settings/pins are undoable authored inputs.

Documentation updates do not change file formats, geometry, measurement evaluation
or solver behavior. Future UI changes must preserve each product's capabilities.

## Brand Commitments

Retain the Seamly2D, SeamlyMe and Seamly3D identities and native desktop affordances.
The approved Seamly3D atelier refinement uses ivory/graphite surfaces, deep green
accents, vector icons, direct placement handles and quick camera views, retaining
the linen dress form and meaningful panel colors. Its implementation is specific
to Seamly3D. Seamly2D and SeamlyMe currently retain their existing Qt controls and
resource icons; no shared atelier implementation is claimed for those products.
English UI text uses existing translation infrastructure. Additional localization
and any suite-wide visual replacement remain separate implementation decisions.

## Evidence on Hand

- `src/app/app.pro`: three application targets.
- `src/app/seamly2d/mainwindow.ui`, `mainwindow.cpp` and `share/resources/toolicon.qrc`:
  drafting, pieces, layouts, docks and current action artwork.
- `src/app/seamlyme/tmainwindow.ui`, `tmainwindow.cpp` and
  `share/resources/seamlymeicon.qrc`: measurement table, details, metadata,
  diagram, file handling and current artwork.
- `src/app/seamly3d/README.md`, `atelier.h`, `window.cpp`, `viewport.cpp` and
  `AssemblyView.qml`: current 3D behavior and authored appearance.
- `.impeccable/review/`: existing Seamly3D captures and review records only.

Seamly2D and SeamlyMe context was extracted from source in this update; no new
runtime or screenshot verification is asserted.

## Product Principles

- Keep the active drawing, measurement data or 3D work visible during editing.
- Group controls around each product's actual tasks and current selection.
- Make units, formulas, selections, document state and action outcomes explicit.
- Preserve native keyboard, focus, dialog, translation and form behavior.
- Preserve data meaning between products and expose experimental preview limits.
