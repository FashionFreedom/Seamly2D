---
version: 1
slug: seamly3d
primary_target: src/app/seamly3d
related_targets: ["src/app/seamly3d/window.cpp", "src/app/seamly3d/AssemblyView.qml", "src/app/seamly3d/atelier.h", "src/app/seamly3d/viewport.cpp"]
---

# Seamly3D desktop workspace

Mode: Operate.
Target: `src/app/seamly3d/window.cpp`, `AssemblyView.qml`, existing desktop workspace.
This surface covers Seamly3D; suite-wide product/design context also covers
Seamly2D and SeamlyMe in their own briefs. Preserve all current functionality,
implement directly in the app. The user rejected the first pass as bland and its
folder/floppy icons as dated. They approved the proposed atelier refinement:
ivory/graphite surfaces, deep green accents, coherent vector icons, direct 3D
placement handles and quick camera views. Preserve Qt desktop behaviour and the
scene/inspector architecture. Code-led remains the selected workflow.

## Direction contract

FIRST VIEWPORT: the live 3D scene occupies at least two thirds of a normal desktop
window. A restrained native toolbar holds document and view actions, with Assemble
as the clear primary action. A 360–400 logical-pixel inspector presents Dress form,
Pieces and Seams as separate native tabs. Each tab has its own local scroll, so
changing a body measurement never requires scrolling past seam controls.

Signature interaction: click a garment panel in the 3D scene to select it and open
the Pieces inspector with its placement controls. A selected saved seam brings up
its two contour previews in the Seams inspector. Tab changes preserve selections.

Native material and typography: system text and Qt inputs, authored atelier light/
dark palette, navy 3D stage, linen mannequin and meaningful panel colours. No web
cards, fake macOS chrome, display fonts, decorative animation or fabricated data.

Information hierarchy: file/view commands above, 3D model central, contextual
editing on the right, concise status and seam/strain diagnostics below. Explanations
belong beside the relevant task; detailed experimental limitations remain available
from an explicit Preview limitations action, with a persistent short status cue.

Task grouping: body measurements/contact/stand under Dress form; selection,
arrangement and transforms under Pieces; paired contours, endpoints, direction,
length comparison and existing seams under Seams. All previous functions survive.

Adaptation: verify 1440×900 and 1100×760 desktop windows, light/dark. A draggable
splitter and locally scrolling inspector preserve access at compact widths. No
mobile screen is claimed. Keep main actions and status visible in every case.

Verification budget: one batched screenshot inspection across the viewport/state
matrix, one consolidated correction batch, at most one confirmation round. Native
code is outside the HTML/CSS detector; use the finish review and native tests.

## Approved refinement contract

THESIS: a precise garment workspace with the quiet warmth of an atelier.
OWN-WORLD: ivory editor surfaces in light appearance, graphite-green in dark,
deep green primary action, linen body and navy stage. System text and native Qt
keyboard/form semantics. All icons share an authored 24-unit, 1.8-unit round-stroke
vector grammar; no bitmap folder/floppy icons or glyph substitutes.
STORY: document actions above; Inspect/Move/Rotate and camera views directly above
the stage; contextual dimensions/seams alongside; factual diagnostics below.
FIRST VIEWPORT: retain a dominant live stage and three task tabs. Assemble is the
sole primary action. Scene tools form a distinct secondary strip. Bold section
headings and restrained surface changes establish hierarchy without card stacks.
FORM: established scene/inspector topology, strengthened by the approved material
palette and vector control family. No open concept tournament or random form seed
is needed for this user-directed refinement of the existing workspace.
SIGNATURE: select a panel, choose Move/Rotate, drag its projected 3D axis/ring to
edit its initial placement. Numeric fields stay synchronized; Escape restores the
pre-drag pose and modified flag. Rotation axes match the stored Euler convention.
Front/back/right/left/top/three-quarter views change orientation without edits to
project geometry. No decorative motion; model and control feedback carry motion.

## Increment: history and piece inspection

User approved the first roadmap increment: project undo/redo and a synchronized
piece list with visibility, isolate/show-all and transparency. Preserve the atelier
world and task-tab topology. No new visual-world/concept selection is involved.

FIRST VIEWPORT: document toolbar adds compact Undo/Redo actions with standard
platform shortcuts. Pieces inspector starts with a bounded, checkable scene list,
Isolate selected / Show all and a transparent-selection checkbox. The existing
placement dropdown becomes an internal selection adapter; there is one visible
piece-selection list. Existing placement and arrangement controls follow in the
same locally scrolling page. All other functions and tabs remain accessible.

SIGNATURE: scene picking and list selection follow each other. Unchecking a row
hides only its rendering; hidden pieces remain selectable in the list and keep
participating in assembly/export. Hidden selections have no draggable handles.
Transparency is inspection-only. Camera, selection and inspection do not dirty
the project or enter history. Inspection resets on opening a project and survives
ordinary edits/undo. One completed gizmo drag creates one history command; Esc
creates none. Undo/redo restores authored inputs and regenerates the preview,
not a saved simulation frame. Save marks the current history position clean;
new edits after undo drop redo. Open clears history; geometry import is undoable.

## Increment: seam editing

User requested continuing the roadmap after history/inspection. Implement the
next block: named seam identities, in-place range/direction edits, persisted
include-in-assembly flag, linked 2D/3D guides and transactional undoable updates.
Keep the established atelier world. Code-led extension, no new identity/comp round.

FIRST VIEWPORT: Seams starts with the saved list and New/Remove actions, followed
by Name and Include in assembly, the paired contour editors, lengths and context
appropriate Add or Apply/Revert actions. Add/Apply/Revert and inline feedback stay
pinned in a fixed footer outside the local scroll area, so compact-height edits
always retain visible actions and recovery. Saved entries
carry names and explicit Paused text; stable IDs appear in tooltips. The 3D stage
retains dominance. Side A is cyan/solid and B coral/dashed, with dark casing for
contrast. A restrained legend explains that guides show through geometry.

SIGNATURE: choosing a seam loads its two ranges and reveals both on the deformed
model. Editing remains a draft until Apply, save or switching seam/tab; valid
switches commit one history command. Invalid drafts show an inline reason, block
save/switch and can be reverted without changing stored inputs. Document undo and
assembly are unavailable during a changed draft. New seam mode uses explicit Add.
Pausing a seam retains its ID/name/ranges in the file but removes its constraints
and ordinary sewing tubes from assembly; selecting it still reveals editing
guides. Hidden pieces stay hidden. Leaving Seams hides the guides. Include/name/
range/direction edits share undo/redo. File format v3 preserves semantics; v1/v2
migrate deterministically, while older companion builds reject v3.

## Increment: continuous seams, controllable mesh and physical draping

User explicitly authorized these three functions after the seam editor. Preserve
Atelier and desktop Operate mode. Extend in code; no new comp or quality-bar card.
Four tabs: Dress form, Pieces, Seams, Cloth. Inspector minimum 400, normal 440,
maximum 480 logical pixels; the stage remains dominant at normal desktop width.

Seams uses continuous original-outline arc-length percentages with six decimal
places, original segment projection on click, quarter-perimeter labels and green/
red endpoints. Solid cyan A / dashed coral B and pinned draft footer are retained.
Changing mesh size must preserve seam endpoints and lengths; v4 stores normalized
fractions and migrates v1/v2/v3 indices. Changed outlines invalidate relationships
through the existing import summary. Named source edges remain future work.

Cloth presents Mesh resolution (5–100 mm, actual counts), then geometric/physical
mode, mass, stretch/bend softness, damping, friction and factual solver status.
Physical mode is visibly experimental; no self-collision, fabric layers or calibrated
material claim. Native numeric inputs and local scroll remain the access mechanism.
Pieces adds Fix top centre during draping: the middle half of the highest source
edge is held, corners remain free for seams. Reset restores initial state; settings
and pins use undo/redo, simulation frames do not. Physical status reports simulated
time, maximum speed and distinct rest/time-limit/instability stop reasons; a time
limit is explicitly not convergence. Retain all previous navigation/inspection.

Backend: CPU XPBD, four substeps/eight iterations per fixed 1/60 s frame, area mass,
edge stretch/shear, approximate distance bending, sampled body/floor contact and
approximate friction. Metal remains rendering. Source contours/corners retained;
conforming adaptive subdivision with explicit budgets, not guaranteed mesh quality.
