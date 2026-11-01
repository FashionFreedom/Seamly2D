---
name: Seamly suite desktop workspaces
description: Native desktop design context for Seamly2D, SeamlyMe and Seamly3D.
colors:
  atelier-window-light: "#f1eee6"
  atelier-window-dark: "#202a26"
  field-light: "#fcfbf7"
  field-dark: "#27332d"
  alternate-light: "#e8e5dc"
  alternate-dark: "#303c34"
  button-light: "#e7e7dd"
  button-dark: "#303d35"
  ink-light: "#243b32"
  ink-dark: "#eeeade"
  accent-light: "#2c604e"
  accent-dark: "#accbb2"
  accent-ink-light: "#ffffff"
  accent-ink-dark: "#182c22"
  border-light: "#8e9585"
  border-dark: "#768579"
  disabled-light: "#787f73"
  disabled-dark: "#879388"
  icon-light: "#29483d"
  icon-dark: "#e6e5dc"
  stage: "#181f28"
  stage-annotation: "#bdccdb"
  stage-units: "#91b5c6"
  linen: "#d7c9ad"
  axis-x: "#ef9586"
  axis-y: "#b4d69a"
  axis-z: "#92bcf4"
  handle-backing: "#16221e"
  contour-fill: "rgba(80,155,180,0.117647)"
  seam-a-light: "#1d7897"
  seam-a-dark: "#70c9ed"
  seam-b-light: "#a34831"
  seam-b-dark: "#f3a48d"
  range-start: "rgb(43,162,118)"
  range-end: "rgb(221,108,76)"
typography:
  workspace-name:
    fontSize: "18px"
    fontWeight: 600
  section:
    fontSize: "14px"
    fontWeight: 600
  primary-action:
    fontWeight: 600
  viewport-label:
    fontSize: "12px"
  handle-label:
    fontFamily: "sans-serif"
    fontSize: "12px"
    fontWeight: 700
rounded:
  control: "5px"
  field: "4px"
  scrollbar: "3px"
spacing:
  inspector-inset: "20px"
  inspector-section: "12px"
  compact-group: "8px"
  viewport-inset: "18px"
components:
  button-secondary:
    rounded: "{rounded.control}"
    padding: "7px 12px"
  button-primary:
    rounded: "{rounded.control}"
    padding: "7px 18px"
    typography: "{typography.primary-action}"
  tool-button:
    rounded: "{rounded.control}"
    padding: "4px 8px"
  numeric-field:
    rounded: "{rounded.field}"
    padding: "5px 6px"
  inspector-tab:
    padding: "12px 10px"
---

# Design System: Seamly suite

## Overview

**Creative North Star: "The precise atelier"**

This record covers Seamly2D (`src/app/seamly2d`), SeamlyMe (`src/app/seamlyme`) and Seamly3D (`src/app/seamly3d`). Shared rules preserve native Qt behavior, precise editing and visible task context. Each product retains its implemented visual language.

**Token scope:** all frontmatter colors, typography sizes, radii, spacing and component tokens describe the implemented Seamly3D atelier only. Seamly2D and SeamlyMe inherit Qt/platform metrics and their own existing resource artwork; these frontmatter values are not their implemented tokens. Shared behavior does not imply a shared authored palette.

### Seamly2D and SeamlyMe incumbent design

Seamly2D is a dense drafting workspace with menus, toolbars, a graphics work area and docks for tools, properties, groups, pieces and layout pages. SeamlyMe is a table-and-detail measurement editor with Measurements and Information tabs, search controls, gradation controls and a measurement-diagram dock. Preserve those distinct task structures, current icon resources and native control states when extending either application.

These two products were inspected in source (`mainwindow.ui` / `mainwindow.cpp`, `tmainwindow.ui` / `tmainwindow.cpp` and their icon resource manifests). The existing review captures concern Seamly3D only. No suite-wide atelier migration or visual verification of the other apps is asserted.

### Seamly3D implemented atelier

The creative north star above belongs to this implemented 3D workspace. The user-approved atelier refinement gives a precise garment workspace quiet warmth: ivory editing surfaces in light appearance, graphite-green in dark, a green primary action and coherent line icons surrounding the persistent navy stage and linen dress form.

Qt Widgets retain native keyboard, focus, dialog and form behaviour while the application authors their palette and surface styling. Qt Quick 3D carries the model and direct placement interaction. Density comes from task grouping, compact controls and proximity, with local inspector scrolling keeping the work visible. macOS light/dark captures verify the built appearance; Windows and Linux remain desktop build targets, with no mobile interface implied.

**Key Characteristics:**
- Authored ivory/graphite-green surfaces with a restrained green action hierarchy.
- Native system text, forms and keyboard conventions.
- One coherent round-stroke vector icon family.
- Persistent scene with direct handles and explicit numeric alternatives.

Seamly3D evidence: `PRODUCT.md`, `.impeccable/surfaces/seamly3d.md`, `src/app/seamly3d/README.md`, `atelier.h`, `window.cpp`, `viewport.cpp`, `AssemblyView.qml`, `model.cpp` and `clothsolver.cpp`. The current correction set has nineteen captures in `.impeccable/review/physics-fix/` (thirteen 1440×900 desktop and six 1100×760 compact, light/dark, Retina 2×). `build/seamly3d-physics-native-tests.txt` records 44 passed, zero failed, zero skipped on native Metal, including rounded-corner percentage quantization and Cloth controls. The initial `.impeccable/review/physics-finish-review.md` required functional-instability and stale-design fixes. `.impeccable/review/physics-finish-verdict.md` records ship for those two resolved fixes and their round-one regression verification, with no introduced regressions visible across the nineteen corrected captures. That verdict does not establish a broader surface or solver audit, calibration, convergence or arbitrary-project stability. Built code is authoritative.

## Colors

**Suite scope.** Seamly2D and SeamlyMe use their existing Qt palette/control styles and resource artwork. Preserve semantic drawing colors, selection, validation and disabled states from their own source. No fixed suite accent or light/dark palette has been extracted for them.

The remainder of this section describes Seamly3D.

Warm editor neutrals and green accents frame a fixed navy scene; semantic scene colours stay distinct from interface states. Frontmatter colour suffixes name light/dark appearance, not lighter/darker stops of a generated ramp.

### Primary

Deep green (`accent-light`) fills Assemble in light appearance. Sage (`accent-dark`) with dark green text preserves the same action hierarchy in dark appearance. The highlight role also supplies focus borders, selected list rows and the selected tab underline. Assemble is the sole filled primary action.

### Secondary

Axis X, Y and Z use salmon, pale green and blue, respectively, with dark casing and opaque dark letter backplates. Side A cyan/solid and Side B coral/dashed encode the paired seam ranges in contour previews and scene guides. Contours adapt their ink to appearance; the fixed navy stage uses the dark-appearance range inks. Distinct start/end colours, quarter-perimeter percentage labels and explicit fields retain endpoint meaning. Garment panel colours are project data, not a brand colour scale.

### Neutral

Atelier window, field, alternate and button roles define the editing layers in both appearances. Ink, border and disabled roles are explicitly authored in `Atelier::palette()`; stylesheet roles are resolved to those colours so appearance changes refresh consistently. The icon inks are separately authored for light/dark, with reduced alpha when disabled. Stage remains navy in both appearances; annotation and units have separate cool pale inks. Linen is the dress-form material.

**The Atelier Roles Rule.** Extend the authored light/dark palette through Qt roles and the existing style function; retain native control behaviour and permitted native checkbox/tab adaptations.

**The Semantic Colour Rule.** Keep geometry, contour endpoints and axis colours tied to their editing meanings, with labels or fields carrying the same information.

## Typography

**Suite scope.** Retain Qt/platform text, translated labels, numeric precision and units. Seamly2D tool/property labels and SeamlyMe table headers, names, formulas and details keep their existing metrics. The explicit hierarchy below is Seamly3D-specific, not a suite-wide type scale.

Widgets and QML annotations inherit Qt's system font family. No display face, custom brand font, family stack or proportional display scale is established. The Canvas handle letters explicitly use generic sans-serif; that limited rendering choice does not define a widget font family.

### Hierarchy

- **Workspace name:** the frontmatter workspace-name size and semibold weight identify the application in the toolbar.
- **Section:** the frontmatter section size and semibold stylesheet weight establish task groups; labels are also marked bold in widget setup.
- **Body and field labels:** inherited Qt font and platform metrics.
- **Notes:** inherited font reduced by one point, with a minimum of ten points; explanatory labels wrap.
- **Primary action:** semibold emphasis within the inherited button size.
- **Viewport and handle labels:** compact explicit pixel sizes from frontmatter; handle letters are bold for spatial identification.

**The Native Type Rule.** Express hierarchy through the implemented system text, section emphasis and proximity; keep any font-family claim limited to what the source declares.

## Layout

### Seamly2D

Keep the graphics work area central and expose task-dependent drafting, piece and layout commands through the existing menus/toolbars. Tool properties, toolbox, groups, pieces and layout-page docks retain their visibility and docking behavior. Adapt desktop windows using existing docking, resizing and scrolling; preserve access to the drawing and current mode. Source: `src/app/seamly2d/mainwindow.ui` and dock/action setup in `mainwindow.cpp`.

### SeamlyMe

Keep the Measurements table, search and selected-row Details together. Preserve the separate Information tab for document/person metadata, gradation toolbar for multisize context and movable/closable Measurement diagram dock. Numeric columns and formula fields must remain understandable at compact desktop sizes through existing layout and scroll behavior. Source: `src/app/seamlyme/tmainwindow.ui` and `tmainwindow.cpp`.

### Seamly3D

A fixed document toolbar spans the window above a horizontal splitter and bottom status bar. The expanding left scene has its own secondary toolbar for Inspect/Move/Rotate, camera presets, Fit view and Mesh. The right inspector contains Dress form, Pieces, Seams and Cloth tabs. This topology preserves the established workspace while sharpening action hierarchy.

The inspector is constrained to 400–480 logical pixels, initially 440; the initial split is 1000/440 in a 1440×900 window. Application minimum is 960×640 and viewport minimum is 400×350. The reviewed 1100×760 compact layout uses the splitter and local scroll rather than a mobile transformation. The stage dominates normal-width windows; the two-thirds target is not a compact-width invariant.

The one-pixel splitter handle separates non-collapsible children. Each inspector tab owns a frameless resizable scroll area with horizontal scrolling disabled. Seams keeps Add or Apply/Revert and inline feedback in a fixed footer outside that scroll area, separated by a one-pixel top border; actions and recovery remain visible at compact heights. Insets, section gaps, smaller groups and viewport annotations use frontmatter spacing roles. Paired seam controls sit side by side; the placement grid aligns axes with position and rotation columns. Toolbar padding is 7×14 logical pixels, with 4×12 for scene tools; both use a flat continuous surface.

**The Persistent Scene Rule.** Keep task editing beside the scene, with local inspector scrolling and persistent toolbar/status access.

## Elevation & Depth

Seamly2D and SeamlyMe use existing native dock frames, table/grid boundaries and Qt control depth; no authored suite shadow scale is established. The following authored treatment describes Seamly3D.

Editing surfaces are flat at rest: tonal layers, one-pixel borders, section spacing and a two-pixel selected-tab underline establish hierarchy. There is no authored UI shadow vocabulary. Native control subparts may retain platform depth. The 3D scene uses lighting, rough materials, a floor and cast shadows to explain spatial relationships; these rendering properties do not become UI elevation tokens.

## Shapes

Seamly2D and SeamlyMe retain platform/Qt control shapes and their existing icon resources. Preserve recognizable tool silhouettes and state indicators. The following radii and round-stroke icon family apply to Seamly3D.

Buttons and tool buttons share modest rounded corners; fields use a slightly tighter radius, both recorded in frontmatter. Fields, buttons, lists and the splitter use restrained borders. Tabs use a flat underline treatment while native subparts can remain visible. The inspector uses sections, a flat checkable group and frameless scroll areas. Contour previews trace original sewing outlines with quarter-perimeter percentage labels and marked continuous endpoints.

**The One Stroke Family Rule.** Use the authored 24-unit vector grid with 1.8-unit round caps and joins for document, scene and task icons; retain text labels beside toolbar icons except the compact Undo/Redo pair, whose QAction text, tooltips and standard shortcuts remain available. Toolbar icons render at 18 logical pixels.

## Components

### Seamly2D drafting tools and docks

Existing QAction menus/toolbars and toolbox tools use `share/resources/toolicon.qrc`. Retain labels, shortcuts, tooltips, checked/enabled states and mode-dependent availability. Tool-property forms, group/piece controls and layout pages remain native Qt widgets tied to the current document and selection. Preserve canvas selection and undo/redo feedback.

### SeamlyMe measurement editor

The QTableWidget presents measurement names, numbers, full names, values, formulas, base values and size/height increments as applicable to document type. Search, ordering and deletion act on this context. Details exposes the selected measurement's editable fields and function wizard; Information contains document/person metadata. Keep read-only state, formula feedback, selection and individual/multisize differences explicit. Icons remain in `share/resources/seamlymeicon.qrc`; the diagram dock explains the selected measurement.

### Seamly3D component scope

All component descriptions below refer to the authored 3D workspace.

### Toolbar and buttons

Document actions sit above the workspace; scene modes form a distinct secondary strip above the stage. The shared vector family replaces platform folder/floppy artwork. Tool buttons acquire the button surface on hover; checked tools add a border. Push buttons use button/alternate surface states and highlight borders for focus or press. Assemble uses the highlight/highlighted-text pair and changes to Pause while running. Disabled states use authored muted ink; disabled Assemble returns to the button surface. Its minimum footprint is 104×28 logical pixels, not a universal size. Space runs/pauses only while the canvas has focus; the focused scene list retains Space to toggle its visibility check. F fits, and V/G/R choose scene modes. Undo/Redo form a compact icon-only pair in the document toolbar and labelled actions in Edit, using the same round-stroke vector family and platform shortcuts.

### Inspector navigation

Four document-mode Qt tabs expose Dress form, Pieces, Seams and Cloth with vector icons, labelled text and a highlighted selected underline. Selecting a garment opens Pieces and synchronizes its selection; tab changes retain selection. Native keyboard and focus handling remain. Native checkbox and residual tab adaptations may remain outside the scene list; they are not new palette primitives. The scene-list visibility indicator instead uses the authored Base/Text pair independently of selected-row colours, preserving a readable checked state in both appearances.

### Inputs and selection

Native combo boxes and numeric spin boxes use the authored field surface, border, tighter corner and highlight focus border. Lists use the field surface and highlight selection. Disabled controls use muted ink and window backgrounds. Labels explicitly carry centimetres, millimetres or degrees; numeric placement uses one decimal place. Accessible names distinguish piece selection and axis controls, and axis labels have buddies. Selection determines enabled actions.

### Scene pieces and inspection

The Pieces tab begins with one bounded checkable scene list (90–140 logical pixels high), followed by paired Isolate selected / Show all buttons and Transparent selected piece. Names elide in the middle; tooltips retain the name and visibility/transparency state. This is the sole visible placement-piece selector; its internal combo-box adapter is hidden. Scene picking and list selection synchronize the current piece and numeric placement controls; picking opens Pieces and tab changes preserve selection.

Row selection and visibility checks carry separate meanings. Hidden rows remain selectable and keep numeric controls, while hidden geometry has no draggable handles. The list uses standard Qt item selection, hit testing and keyboard interaction. Its Fusion-based proxy style customizes only the visibility indicator, using atelier Base for the box and Text for the tick independently of row highlight. Do not let selected-row colour inversion erase a visibility state.

Visibility, isolation and per-piece transparency are viewport inspection state. Show all restores visibility while retaining transparency choices. Transparent surfaces suppress mesh/sewing overlays to keep the inspection clear. Inspection survives ordinary edits and undo/redo, resets when opening a project, and never changes project dirty state, history, solver participation or mesh export.

**The Inspection Separation Rule.** Keep scene visibility and transparency separate from authored project inputs; selection must remain available even when a piece is hidden.

### Project history

Undo/Redo use platform shortcuts and native enabled/action text feedback in Edit and the document toolbar. Session history holds up to 50 authored-input edits: placement, dress-form changes, arrangement, seams, geometry import, mesh resolution, physical settings and top-centre pins. Undo/redo stops assembly, restores inputs and regenerates the preview; simulation frames, camera, selection and inspection do not enter history.

One completed handle drag is one command; Escape restores its initial pose and modified state without a command, and no-op edits add none. Numeric entry commits on editing completion. Save marks the current history position clean without clearing undo; undo away from that position marks the document modified, redo back clears it, and a new edit after undo drops the redo branch. Opening another project clears history. Examples remain unsaved until saved; history is not serialized. A changed saved-seam draft disables document Undo/Redo and Assemble until Apply or Revert resolves it; applying creates one authored-input command.

**The Authored History Rule.** Record completed changes to authored inputs, preserve the saved clean position, and rebuild the preview when restoring history.

### Saved seam editor and contour endpoints

Saved seams occupy a bounded list (70–100 logical pixels high) above New seam / Remove selected, Name and Include in assembly. Entries retain stable IDs in tooltips, use editable names and append explicit Paused text when excluded. Selecting one restores its pieces, endpoint ranges and direction for in-place editing. New seam mode uses explicit Add seam.

Changed saved-seam fields form a transactional draft. Apply changes validates and records the update as one history command; Revert changes restores stored inputs. Save and switching seam or inspector tab automatically apply a valid draft. Invalid drafts keep a specific inline reason and correction/Revert instruction, disable Apply and block save or switching. The fixed footer holds these actions and feedback outside local scrolling. Document Undo/Redo and Assemble remain unavailable while the saved-seam draft differs from stored inputs.

Side A and Side B previews use palette-bound backgrounds, original outlines and three-pixel cyan/solid and coral/dashed ranges. Start/end markers combine green/red colours, quarter-perimeter percentage labels and native percentage fields with six decimal places. Clicking projects onto the original outline segments; endpoint selection is continuous rather than snapped to a sample grid. Equal endpoints mean the whole perimeter. Endpoints and undeformed seam lengths remain independent of mesh density through original-outline arc fractions and interpolated boundary anchors. Corresponding guides follow the deformed geometry in the scene with dark casing and solid/dashed identity. The legend explicitly states that guides show through geometry; hidden pieces produce no guides, and leaving Seams hides them.

Include in assembly persists with the seam name, ID, ranges and direction. Paused seams retain their editing guides when selected while contributing neither assembly constraints nor ordinary sewing tubes. This authored pause is separate from viewport-only piece visibility. Projects save as v4 and read v1/v2/v3/v4; legacy seams receive deterministic identity and old sample indices migrate to normalized original-outline fractions. The file retains `meshSize`, simulation parameters and per-piece `pinTop` alongside names, identities and enabled state. Geometry import retains relationships only for unchanged outlines and reports removed relationships. Neither solver establishes calibrated cloth or garment-fit behaviour.

**The Seam Draft Recovery Rule.** Keep Apply/Revert and inline recovery visible outside scrolling; commit valid saved-seam drafts atomically and retain invalid drafts for correction or reversion.

### Cloth resolution, draping and pins

Cloth follows the same native section headings, numeric forms, wrapped notes and local scrolling as the existing inspector. Mesh resolution precedes Assembly and draping. Maximum edge uses millimetres (5–100 mm); the adjacent readout reports actual vertex and triangle counts. The mode selector explicitly distinguishes Geometric assembly from Physical draping · experimental. Surface mass (g/m²), Stretch softness, Bend softness, Damping (1/s) and Contact friction retain their visible units or explanatory tooltips; they are uncalibrated parameters, not fabric presets.

Adaptive conforming subdivision shares edge midpoints, preserves source corners and maintains boundary parameter maps for exact interpolated seam references. Near-corner percentage rounding must not create microscopic triangles merely to materialize an anchor; an existing boundary edge can support the exact interpolation. Budgets are explicit: 512 combined corners/anchors per piece, 12,000 vertices/24,000 triangles per piece, 40,000 vertices/80,000 triangles per project and 32,768 stitches. Budget failures retain the current assembly. These bounds do not guarantee aspect ratios or minimum angles.

Pieces adds Fix top centre during draping near its inspection controls. This authored physical-mode pin holds the central half of the highest original edge at its initial placement, leaving corners free for seams; a horizontal top edge receives a midpoint even at coarse resolution. A single highest point is pinned when there is no horizontal top edge. Pins and Cloth settings participate in undo/redo; simulation frames do not. Reset restores the initial state and zero velocities.

Physical draping uses the CPU XPBD backend with SI gravity, fixed 1/60 s frames, four substeps and eight iterations per substep. Area-weighted mass, triangle-edge stretch/shear, approximate opposite-vertex distance bending, seams, sampled dress-form/floor contact, damping and approximate tangent friction drive the model; Metal remains rendering. The Cloth status shows simulated time, maximum speed and distinct rest, time-limit or instability reasons. Rest requires one second below 2 mm/s; the 20 s time limit explicitly does not mean convergence. The clock can advance slower than wall time. Self-collision, fabric layers, calibrated materials and fit prediction are unsupported. Preserve the visible experimental qualifier and accessible limitations.

**The Factual Draping Rule.** Show actual mesh counts, simulation time, speed and stop reasons; keep geometric and physical modes explicit and never present a time limit, small gap or low motion as validated fit or convergence.

### Placement handles and camera views

A selected panel exposes world-axis Move handles or rotation rings aligned to the stored Euler convention. Coloured strokes have dark casing; X/Y/Z letters sit on solid dark backplates so linen and bright cloth cannot wash them out. Hover thickens the axis/ring and enlarges its endpoint. Dragging edits initial placement while numeric fields synchronize; Escape restores the pre-drag pose and modified state. These are editing controls, not decorative motion.

Three-quarter, Front, Back, Right, Left and Top presets change camera orientation without editing project geometry. Orbit, wheel zoom, alternate-button pan, double-click fit and click selection remain available. Numeric fields provide precise placement alternatives.

### Viewport and status

The fixed navy stage, linen dress form, project panel colours, selected-panel emissive highlight, optional mesh and sewing lines express model state. Inset annotations show selection, units and tool-specific camera/drag guidance. Bottom status separates project summary, seam-gap/strain metrics, graphics backend and the persistent Experimental preview cue. Preview limitations opens native contextual help. Metrics do not assert validated garment fit.

## Do's and Don'ts

### Do:

- **Do** use the matching product surface brief and incumbent source before editing.
- **Do** preserve Seamly2D docking/modes and SeamlyMe table, formula, gradation and read-only semantics.
- **Do** in Seamly3D, extend the authored atelier light/dark roles while preserving Qt system fonts, control behaviour and keyboard conventions.
- **Do** keep the model visible while task controls scroll locally.
- **Do** use the shared round-stroke vector family with clear action labels.
- **Do** keep direct handles, numeric units and continuous contour endpoints legible and synchronized.
- **Do** preserve commands, selections and accessible preview limitations when extending the workspace.
- **Do** in Seamly3D, keep visibility indicators legible independently of row selection and retain the focused list's Space-key interaction.
- **Do** in Seamly3D, keep inspection outside authored-input history and group each completed handle drag into one command.
- **Do** in Seamly3D, preserve original-outline seam references across mesh changes and show factual experimental draping status.
- **Do** in Seamly3D, keep seam actions and inline recovery pinned while ranges scroll, and preserve solid/dashed guide identity alongside colour.

### Don't:

- **Don't** treat Seamly3D frontmatter tokens or review captures as implemented or verified Seamly2D/SeamlyMe design.
- **Don't** replace the native workspace with web card stacks, fabricated window chrome or display typography.
- **Don't** in Seamly3D, substitute bitmap document artwork or glyph icons for its authored vector family.
- **Don't** treat panel colours or permitted native checkbox/tab colours as fixed brand tokens.
- **Don't** infer mobile layouts or calibrated garment-fit claims from this desktop preview.
