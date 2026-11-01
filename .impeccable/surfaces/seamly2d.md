---
version: 1
slug: seamly2d
primary_target: src/app/seamly2d
related_targets: ["src/app/seamly2d/mainwindow.cpp", "src/app/seamly2d/mainwindow.ui"]
---

# Seamly2D desktop drafting workspace

Mode: Operate.

## Scope and evidence

Covers `src/app/seamly2d`, including `mainwindow.ui`, `mainwindow.cpp` and
`share/resources/toolicon.qrc`. The user requested Impeccable product/design context
for the other products as well as Seamly3D. This brief records existing source;
no visual replacement or new screenshot review has been performed.

## Task and hierarchy

Create and edit parametric sewing patterns, prepare pieces and generate print
layouts. Keep the graphics work area visible, with menus/toolbars for the active
mode and docks for toolbox, tool properties, groups, pieces and layout pages.
Selection and mode determine the available commands and properties.

## Design contract

Preserve existing Qt controls, system text, icons, drawing semantics, keyboard
shortcuts, docking and visibility behavior. Keep dimensions, measurement references
and formulas explicit. Preserve document operations, undo/redo, printing, exports
and the current Seamly3D export integration. Use the product's existing control
states and tool resources as visual authority. Seamly3D atelier tokens are scoped
to that application; a future shared treatment must be implemented and reviewed.

## Adaptation and verification

Native desktop keyboard/mouse workspace; use docking, resizing and scrolling at
compact sizes. For future UI work, inspect actual draft, piece and layout states,
selection, disabled tools and property forms in the built app. Existing 3D captures
do not verify this surface. No mobile layout is established.
