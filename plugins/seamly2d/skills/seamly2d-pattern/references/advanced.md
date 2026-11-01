# Advanced pattern edits

`get_pattern_schema` returns the app's current XSD. `get_object` includes source
XML and attributes; `get_pattern_xml` exposes the complete in-memory document.
The saved format is distinct from MCP and its version is reported in the XML.

Use `create_tool` to append a complete element to `calculation`, `modeling`,
`pieces`, `groups` or `images` in the active draft block. Include nested children
when the schema requires them. Use a fresh globally unique numeric ID (inspect
the whole XML, not only geometric objects). A piece can reference modeling nodes
as well as calculation objects. Never invent a tool type or required attribute:
consult the schema, examples already in the document and the app's source when
necessary. The schema constrains structure; parsing also validates references.

`update_object` changes attributes by ID and `delete_object` removes an element.
Generated geometry has a separate object ID. `get_object` returns its geometry
and the owner's XML; use its `element_id` when updating/deleting the owning tool.
These operations reject changes that leave invalid references or formulas.
For nested edits, coordinated deletions, labels, final measurements, piece
properties or batch changes, use `apply_pattern_xml` with a complete document.
It validates a temporary copy in an isolated Seamly2D process and commits as one
undoable operation. DTD and entity declarations are rejected. A successful edit
does not save the file. Use a meaningful `label` to identify a batch in history.

## Mirror, move, rotate and true darts

Create these with `create_tool` in `calculation`, then edit the owning element
with `update_object`. The native app performs their geometry calculations.

| Construction | XML type | Main attributes |
| --- | --- | --- |
| Rotate | `operation` / `rotation` | `center`, `angle`, `suffix` |
| Move and optionally rotate | `operation` / `moving` | `angle`, `length`, `rotationAngle`, `center`, `suffix` |
| Mirror about a line | `operation` / `flippingByLine` | `p1Line`, `p2Line`, `suffix` |
| Mirror about an axis | `operation` / `flippingByAxis` | `center`, `axisType` (`1` vertical, `2` horizontal), `suffix` |
| True darts | `point` / `trueDarts` | `baseLineP1`, `baseLineP2`, `dartP1`, `dartP2` (apex), `dartP3`, `point1`, `point2`, `name1`, `name2` |

Each operation needs matching `source` and `destination` lists of `item`
elements, with an `idObject` on every item. Source IDs identify existing objects;
destination IDs reserve fresh generated objects. A true-darts tool reserves two
output IDs in `point1`/`point2`. All generated IDs must be unique across the
document, including other operation outputs. Inspect the complete XML before
allocating them. Check the resulting coordinates and formulas before saving.

Custom variables begin with `#`. Measurement values come from the linked
measurement file: changing the root `measurements` path through a validated
document edit attaches a different measurement set. Do not overwrite a linked
measurement file as a side effect of editing a pattern.

`new_pattern`, `open_pattern` and `close_pattern` refuse to replace unsaved work
unless `discard_changes=true`. `save_pattern` supports Save As with an absolute
path. `export_pattern` renders the visible scene to SVG/PDF/PNG. For actual piece
exports or nesting, use `get_export_formats` and `export_layout`; it runs the
existing engine on a temporary snapshot and publishes files to an empty output
directory. `pieces_only` skips nesting. Inspect the result before printing.

## Measurement tables and sizes

Use `get_measurements_file` to obtain entries, units and a file token. The file
can be individual (`.smis`, formula strings) or multisize (`.smms`, base values
and increments). Custom names start with `@`. `write_measurements_file` upserts
named entries and removes names listed in `remove`; it preserves the remaining
entries and personal metadata. Existing files require their latest `file_token`
and `overwrite=true`, in addition to the current pattern `state_token`. Writes
are atomic and refuse SeamlyMe locks, read-only tables, broken dependencies and
invalid formulas. Creating a table requires its `type`/`unit` and, for multisize,
`base_size`/`base_height` in file units. It does not open SeamlyMe's editor.

File writes persist immediately and cannot be reversed with pattern undo.
Editing the attached file first validates the new table against the current
pattern, then reloads its geometry. `attach_measurements` takes the read file's
path/token; `detach_measurements` refuses to break formulas. Both are undoable.
`get_measurements_status` reports the table and active/allowed gradations.
When `needs_sync` is true after an external edit, read its latest file token and
attach the table again before drafting; this refreshes the live calculation.
`set_gradation` selects and persists size/height in cm and supports undo/redo.
Read the file unit separately when supplying measurement values or file evaluation
dimensions. Export defaults to the active size/height.

## Pieces and margins

Prefer `create_piece` over XML: supply the name and an ordered `nodes` array of
calculation point/curve `object_id`s, with optional `reverse`/`excluded` flags.
The tool creates modeling nodes and globally unique IDs. Use `update_piece` for
name, layout/flipping/lock flags or a replacement contour, and `delete_piece` to
remove a piece while keeping construction geometry. Unshared contour modeling
nodes are cleaned up; shared references are preserved.

`get_piece` reports nodes, attributes and calculated main/seam-allowance outlines,
areas and bounds in document units (areas in squared units). `set_seam_allowance`
sets enabled/built-in flags and the global width formula. Its optional `nodes`
array takes modeling `node_id`s from `get_piece` plus `before`/`after` formulas;
these are distinct from source calculation IDs. `CurrentSeamAllowance` refers to
the current global width in these formulas. Widths must be finite and non-negative.
These operations validate the pattern and participate in undo/redo. Inspect the
outline and export a copy before printing.
