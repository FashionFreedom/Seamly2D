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
