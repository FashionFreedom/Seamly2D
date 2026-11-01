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

The socket parent must already exist, belong to the user, and have mode `0700`.
Keep its full path under 100 bytes on macOS. Give separate app instances separate
sockets. No listener starts without `SEAMLY_MCP_SOCKET`; quitting that app stops
it. An existing socket is never removed on startup. Remove a stale socket after
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

The server exposes **35 tools**. `tools/list` is the authoritative catalog with
input schemas and read/write annotations. Codex invokes these tools directly;
results are returned as structured data.

| Area | Tools |
| --- | --- |
| Inspect | `get_pattern_summary`, `list_objects`, `get_object`, `get_selection`, `get_preview`, `list_pieces`, `get_history` |
| Documents | `new_pattern`, `open_pattern`, `save_pattern`, `close_pattern`, `get_pattern_xml`, `apply_pattern_xml`, `get_pattern_schema` |
| Draft blocks | `add_draft_block`, `rename_draft_block`, `set_active_draft_block` |
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

The dedicated geometry tools cover common drafting. `create_tool` accepts a
complete XML element for a calculation/modeling/pieces/groups/images section.
`update_object` edits attributes by ID. `get_pattern_schema` serves the app's
current XSD, and `get_object` includes source XML. Use these for other supported
construction tools, pieces and their nodes, internal paths, seam allowances,
labels, groups or images.

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
explains this workflow. Editing a pattern is distinct from editing a linked
SeamlyMe measurement file: use an appropriate existing measurement file and
change the pattern's measurements reference when needed.

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

Without `SEAMLY_MCP_TEST_APP`, the GUI integration case is skipped. Without
`SEAMLY_MCP_TEST_SERVER`, the tests use the standalone development companion.
Tests need permission to bind a local socket. They use temporary documents,
settings and private sockets, and terminate only their own processes. On macOS,
the test's matching Qt installation supplies the offscreen GUI plugin only to
that GUI process; the companion retains its normal deployed framework paths.

Tests cover legacy/modern protocol isolation, schema validation, resource and
skill integrity, framing, geometry, variable errors, stale state, transactional
edits, mixed native/snapshot undo/redo, block changes, file operations and export.
