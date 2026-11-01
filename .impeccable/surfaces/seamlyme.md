---
version: 1
slug: seamlyme
primary_target: src/app/seamlyme
related_targets: ["src/app/seamlyme/tmainwindow.cpp", "src/app/seamlyme/tmainwindow.ui"]
---

# SeamlyMe desktop measurement workspace

Mode: Operate.

## Scope and evidence

Covers `src/app/seamlyme`, including `tmainwindow.ui`, `tmainwindow.cpp` and
`share/resources/seamlymeicon.qrc`. The user requested Impeccable product/design
context for all suite products. This is source-based incumbent documentation;
no new visual implementation or runtime screenshot review is claimed.

## Task and hierarchy

Create and maintain individual and multisize measurements used by pattern work.
Keep Measurements table, search and selected-row Details together. Preserve the
Information tab, gradation toolbar and Measurement diagram dock. Names, values,
formulas, base values and size/height increments retain their document-type meaning.

## Design contract

Use the existing Qt text, table headers, inputs, action artwork, selection and
focus behavior. Preserve known/custom measurements, function wizard, ordering,
read-only state, document metadata, templates, import/conversion and CSV export.
Keep units and formula feedback explicit and distinguish individual/multisize
fields. Personal metadata remains document data. Seamly3D's atelier palette and
vector family are not implemented tokens for this measurement editor.

## Adaptation and verification

Native desktop keyboard/mouse workspace. Preserve table/detail access and diagram
docking using resizing and scrolling. Future UI work must inspect individual and
multisize documents, selection, formula errors, read-only state and Information
in the actual app. Existing 3D review captures do not verify this surface. No
mobile layout is established.
