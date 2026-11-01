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
These operations reject changes that leave invalid references or formulas.
For nested edits, coordinated deletions, labels, final measurements, piece
properties or batch changes, use `apply_pattern_xml` with a complete document.
It validates a temporary copy in an isolated Seamly2D process and commits as one
undoable operation. DTD and entity declarations are rejected. A successful edit
does not save the file. Use a meaningful `label` to identify a batch in history.

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
