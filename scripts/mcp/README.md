# Seamly2D MCP integration

The `seamly2d-mcp` C++/Qt companion controls a running Seamly2D instance on
**macOS or Linux**. It uses Qt Core/Network and is included in the Mac app bundle.
A portable plugin packages its launcher and a pattern-authoring skill. The native
server also serves that skill from compiled resources.

## Build and connect

Build the normal qmake app project. On macOS the companion is deployed to
`Seamly2D.app/Contents/MacOS/seamly2d-mcp` and uses the frameworks inside that app.
Distribute the whole `.app`. Linux installs the companion to `PREFIX/bin` with
`make install`; provide the matching Qt libraries as for the GUI.

From the repository root, start a development instance explicitly:

```sh
mkdir -p build/mcp-runtime
chmod 700 build/mcp-runtime
SEAMLY_MCP_SOCKET="$PWD/build/mcp-runtime/app.sock" \
  ./build/src/app/seamly2d/bin/Seamly2D.app/Contents/MacOS/Seamly2D
```

Register its companion with a local Codex client:

```sh
codex mcp add seamly2d -- \
  "$PWD/build/src/app/seamly2d/bin/Seamly2D.app/Contents/MacOS/seamly2d-mcp" \
  --socket "$PWD/build/mcp-runtime/app.sock"
```

Alternatively use [codex.example.toml](codex.example.toml). Replace its absolute
paths and reconnect the client. On Linux use `seamly2d-mcp` or the standalone
`build/src/app/seamly2d_mcp/bin/seamly2d-mcp`. Direct development binaries depend
on the installed Qt libraries; the deployed Mac copy uses the bundled ones.

The top toolbar's **MCP** button enables the bridge and checks the bundled MCP
executable, tool discovery and a read-only document request. **Waiting** means
the bridge is active; **Connected** means an MCP client has an open session.
The button's own check does not count as a client. Its menu can disable MCP or
copy the socket path. The path comes from `SEAMLY_MCP_SOCKET`, the last successful
path, or `~/.seamly2d/mcp/app.sock`; a new default directory is created privately.

For startup through `SEAMLY_MCP_SOCKET`, the socket parent must already exist,
belong to the user, and have mode `0700`.
Keep its full path under 100 bytes on macOS. Give separate app instances separate
sockets. A listener starts only through `SEAMLY_MCP_SOCKET` or an explicit click
on the MCP button; quitting that app stops it. An existing socket is never removed on startup. Remove a stale socket after
a crash only when its app is no longer running. App and companion run as the same
user on the same computer. Discovery works while the GUI is closed; document
calls return a connection error until it is running.

## Plugin and skill

[plugins/seamly2d](../../plugins/seamly2d) contains the portable `plugin.json`,
`mcp.json`, launcher and `skills/seamly2d-pattern`. To use the plugin's default
socket with the installed Mac app:

```sh
mkdir -p "$HOME/.seamly2d/mcp"
chmod 700 "$HOME/.seamly2d/mcp"
SEAMLY_MCP_SOCKET="$HOME/.seamly2d/mcp/app.sock" \
  /Applications/Seamly2D.app/Contents/MacOS/Seamly2D
```

The companion defaults to this socket when neither `--socket` nor
`SEAMLY_MCP_SOCKET` is supplied. The plugin launcher uses the installed Mac app
(or `SEAMLY2D_APP` when set), and `seamly2d-mcp` from PATH on Linux.

Repo-local discovery is configured in `.agents/plugins/marketplace.json` at the
repository root. Its source path is relative to the repo root.
[marketplace.example.json](marketplace.example.json) is also provided for reuse. Restart the desktop client and install Seamly2D
from the local marketplace in a new chat. Use the official
[plugin packaging guidance](https://developers.openai.com/plugins/build/plugins)
for your client/workspace. No account configuration, marketplace installation or
publication is performed automatically. Local plugin availability depends on the host. Codex calls the tools directly
and uses the skill for pattern-authoring workflows.

The same skill is also exposed with `skills/list`, `skills/get` and
`resources/read` for clients supporting Skills over MCP. Its file manifest
contains byte sizes and SHA-256 digests. Legacy clients can use the packaged
filesystem skill or read it as a resource. Reading a skill resource is distinct
from activating it through a host's skill-loading mechanism.

## Tools

The server exposes **46 tools**. `tools/list` is the authoritative catalog with
input schemas and read/write annotations. Codex invokes these tools directly;
results are returned as structured data.

| Area | Tools |
| --- | --- |
| Inspect | `get_pattern_summary`, `list_objects`, `get_object`, `get_selection`, `get_preview`, `list_pieces`, `get_history` |
| Documents | `new_pattern`, `open_pattern`, `save_pattern`, `close_pattern`, `get_pattern_xml`, `apply_pattern_xml`, `get_pattern_schema` |
| Draft blocks | `add_draft_block`, `rename_draft_block`, `set_active_draft_block` |
| Measurement files / sizes | `get_measurements_status`, `get_measurements_file`, `write_measurements_file`, `attach_measurements`, `detach_measurements`, `set_gradation` |
| Pieces / seam allowance | `get_piece`, `create_piece`, `update_piece`, `delete_piece`, `set_seam_allowance` |
| Variables | `list_variables`, `evaluate_formula`, `set_variable`, `delete_variable` |
| Drawing | `create_point`, `create_line`, `create_curve`, `create_arc`, `create_tool`, `update_object`, `delete_object` |
| View/history | `set_selection`, `set_view`, `undo`, `redo` |
| Export/layout | `get_export_formats`, `export_pattern`, `export_layout` |

Lengths use the document unit. Coordinates increase right/down; angles are
counterclockwise: `0` points right and `90` up. Formula strings use internal
English variable/function names and decimal dots, regardless of the UI locale.
Custom variables begin with `#`. IDs and names are different; use IDs from reads.
Object listings are paginated, up to 200 per call.

Every mutation requires the latest `state_token` from a read. Read and review
state after a stale-state error. Never retry blindly after a disconnect/timeout:
the operation might already have completed. Undo/redo covers manual operations,
too. Close dialogs/menus and finish active drawing tools before using the bridge.
Drawing edits require Draft mode, a writable document and the history cursor at
the end. Views/selections do not dirty the model.

Replacing unsaved work requires `discard_changes=true`. Saving or exporting over
a different existing destination requires `overwrite=true`. Saves use an absolute
`.sm2d` path. Edits do not implicitly save. The server exposes no arbitrary shell
or script execution tool.

## Advanced model features

The dedicated geometry and piece tools cover common drafting. `create_tool` accepts a
complete XML element for a calculation/modeling/pieces/groups/images section.
`update_object` edits attributes by ID. `get_pattern_schema` serves the app's
current XSD, and `get_object` includes source XML. Use these for other supported
construction tools, pieces and their nodes, internal paths, seam allowances,
labels, groups or images.

Mirror by line/axis, move (with optional rotation), rotation and true darts are
available through `create_tool` and `update_object`. Generated objects have their
own IDs; `get_object` returns the owning XML tool in `element_id`. Use that ID to
edit the operation rather than one of its generated outputs. See the skill's
advanced reference for the XML types and required output IDs.

`apply_pattern_xml` handles coordinated/nested edits and all persistent pattern
metadata as a single undoable transaction. Read the full current XML, modify it,
and submit it with its state token and a meaningful `label`. Follow dependency
order and assign globally unique IDs. DTD/entity declarations are rejected.
Documents are limited to 4 MiB for transactional edits/opening.

Validation loads a temporary copy in an isolated Seamly2D process, using temporary
settings and logs and absolute measurement references. It checks the format,
references and formulas before replacing the live snapshot. Invalid custom
variable and non-finite construction formulas are errors in this validator;
normal interactive formula-editing behavior is unchanged. Opening old files
uses the normal converter on the temporary copy and does not rewrite the source.
Files that the app itself cannot parse/convert are rejected.

The skill's [advanced reference](../../plugins/seamly2d/skills/seamly2d-pattern/references/advanced.md)
explains the advanced XML workflow.

## Measurement files and gradation

`get_measurements_file` reads individual `.smis` and multisize `.smms` files with
the native SeamlyMe format/converters and returns calculated entries plus a
`file_token`. Old formats are converted on private copies. Reading, attaching and
validating a pattern do not normalize or rewrite the source measurement file.

`write_measurements_file` creates files or updates/removes named entries.
Individual entries use `formula`; multisize entries use `base`, `size_increase`
and `height_increase`. Names use native standard identifiers or an `@` prefix
for custom measurements. A new file requires `type` and `unit`; multisize files
also require `base_size` and `base_height` in file units. Existing files require
both `overwrite=true` and their latest `file_token`. Files locked by SeamlyMe
must be closed there first. Read-only tables and invalid/non-finite formulas are
rejected. An edit to the currently attached file is validated against the pattern
before publication and reloads its geometry after the atomic write.

Measurement-file writes persist immediately and are **not** pattern undo commands.
`attach_measurements`, `detach_measurements` and `set_gradation` are undoable
pattern edits. Attach takes the path and latest file token; detaching refuses to
break formulas. `get_measurements_status` reports the attached table, enabled
sizes/heights and current selection. `set_gradation` selects and persists a size
and height in **centimetres**, independently of the pattern/file units; file-entry
values and optional `get_measurements_file` evaluation dimensions use file units.
Layout exports default to the active selection unless `gsize`/`gheight` are supplied.
The saved selection is also restored by normal app startup in cm/mm patterns.
External table edits set `needs_sync`; read and attach the table again to refresh
the calculation before drawing. MCP file writes reload it directly.

## Pieces and seam allowances

`create_piece` takes an ordered `nodes` array of `{ "object_id": ... }` references
to calculation points or curves in the active draft block, with optional
`reverse`/`excluded` flags. It creates modeling nodes and IDs automatically.
`update_piece` replaces the contour or changes name/layout/flipping/lock flags.
`delete_piece` removes the piece and unshared modeling nodes while retaining
construction geometry. These changes are validated and undoable.

`get_piece` returns source attributes, contour nodes with both source object IDs
and modeling `node_id`s, and calculated outlines/areas in document units (areas
in squared document units). `set_seam_allowance` sets `enabled`, `built_in` and
`width`, plus optional per-node `before`/`after` formulas keyed by those `node_id`s.
Formulas can use `CurrentSeamAllowance` for the current global width. Inspect the
resulting outline before exporting pieces.

## Export and layout

`export_pattern` renders the current scene to SVG, PDF or PNG. `export_layout`
runs the existing layout/export engine on a temporary snapshot, with optional
`pieces_only`. It supports the format IDs returned by `get_export_formats`,
including DXF variants and tiled PDF where available. Inspect `list_pieces` first.

Supply an empty absolute `output_directory`, a plain `basename`, and a numeric
`format`. The `options` object accepts the value/flag option names returned by
`get_export_formats`; values are strings and flags use `"true"`/`"false"`.
For example, `pagew`, `pageh`, `pageunits` configure custom paper; `pageformat`
selects a standard template. Custom page units supply zero margins unless those
margin options are explicitly provided. The app's normal consistency checks still apply.
Worker files are staged privately and published only after successful generation.
The live pattern and its saved file are unchanged by export. Physical printer
jobs and application preference dialogs are not exposed as MCP tools.

## Protocol and skills

The native server supports modern MCP **`2026-07-28`** with `server/discover`,
per-request version/capability metadata, complete results and catalog cache fields.
It also accepts legacy initialization for `2024-11-05`, `2025-03-26`, `2025-06-18`
and `2025-11-25`. Modern requests do not inherit capabilities from previous calls.
Unsupported versions return the supported-version list.

The `io.modelcontextprotocol/skills` extension provides workflow discovery and
verified skill resources. Clients without that extension can use the skill
packaged in the plugin. The tools return structured results directly to Codex.

References: [versioning](https://modelcontextprotocol.io/specification/2026-07-28/basic/versioning),
[discovery](https://modelcontextprotocol.io/specification/2026-07-28/server/discover),
[Skills](https://modelcontextprotocol.io/extensions/skills/overview).

## Verification

The native Qt Test suite is included in normal test builds. Configure it with
the same qmake architecture/configuration options as the app, then run:

```sh
SEAMLY_MCP_TEST_SERVER="$PWD/build/src/app/seamly2d/bin/Seamly2D.app/Contents/MacOS/seamly2d-mcp" \
SEAMLY_MCP_TEST_APP="$PWD/build/src/app/seamly2d/bin/Seamly2D.app/Contents/MacOS/Seamly2D" \
  ./build/src/test/McpTest/bin/McpTest
```

Without `SEAMLY_MCP_TEST_APP`, the GUI integration cases are skipped. Without
`SEAMLY_MCP_TEST_SERVER`, the tests use the standalone development companion.
Tests need permission to bind a local socket. They use temporary documents,
settings and private sockets, and terminate only their own processes. On macOS,
the test's matching Qt installation supplies the offscreen GUI plugin only to
that GUI process; the companion retains its normal deployed framework paths.

Tests cover legacy/modern protocol isolation, schema validation, resource and
skill integrity, framing, geometry, variable errors, stale state, transactional
edits, mixed native/snapshot undo/redo, block changes, file operations and export.
Five additional integration cases verify mirror by line and axis, move/rotation
and true darts against known coordinates, including edits, invalid references,
generated-object reads, subsequent tool IDs, undo/redo and save/reopen. The Linux
CI supplies the app/companion paths so these cases run instead of being skipped.

Additional integration cases cover individual measurements in cm/mm/inches,
multisize tables in cm/mm, gradation with cm/mm patterns, protected file writes,
active-file recalculation and save/reopen. Piece cases cover point/curve contours,
modeling-node allocation/cleanup, global and per-node seam allowances, undo/redo,
geometry/area checks and SVG piece export in cm/mm.
