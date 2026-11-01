---
name: seamly2d-pattern
description: Create, inspect, edit and export sewing patterns in the local Seamly2D desktop app using its MCP tools. Use for cartamodelli and parametric garment drafting, rather than ordinary image drawing.
---

Use the Seamly2D MCP tools for the requested pattern work. The app must be open
with its local bridge enabled. If connection fails, report the needed setup;
do not substitute clicks or edit the saved file behind the open application.

Read `get_pattern_summary` first. Lengths use its document unit. Coordinates
increase right/down; angles are counterclockwise, so 90 points up. Formula strings
use internal English identifiers and decimal dots regardless of the UI locale.
Read `list_variables` and evaluate uncertain formulas before creating geometry.

Every mutation requires the most recent `state_token`. Review current state
after a stale-state error; never blindly repeat an edit after a timeout. Requests
may have completed even if their replies were lost. Undo/redo includes manual
edits, so inspect `get_history` before changing unrelated work.

Prefer dedicated point, line, curve, arc and variable tools for ordinary drafting.
Use object IDs from reads; IDs and names are different. For other tool types,
pieces, seam allowances, internal paths, groups, images or metadata, read
[advanced editing](references/advanced.md). Keep dependency order in calculations.

Verify the resulting objects/formulas and requested dimensions using the MCP
read tools. Save/export only to the user's intended destination. Replacing unsaved work or overwriting a different existing
file requires an explicit tool flag; do not infer that choice from a read request.
