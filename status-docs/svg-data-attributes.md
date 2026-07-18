# SVG `data-*` Attribute Contract — Seamly2D → SeamlyLayout

**Status:** implemented (Seamly2D branch `run-seamlyLayout`)
**Producer:** Seamly2D — `SvgGenerator` (`src/libs/vformat/svg_generator.cpp`) fed by the piece item tree built in `VLayoutPiece::GetItem()` (`src/libs/vlayout/vlayoutpiece.cpp`)
**Consumer:** SeamlyLayout (SVG parsed via its `svg_dom` crate)
**Source spec:** `status-docs/new-attributes.csv` — this document is the authoritative, expanded contract; keep both apps developing against it.

## When tagged SVGs are produced

1. **Layout Mode handoff** — clicking Layout Mode in Seamly2D writes `<pattern-basename>.pieces.svg` next to the saved pattern file, then launches SeamlyLayout with that path as its single command-line argument.
2. **Manual piece exports** — Piece mode → Export Pieces → SVG carries the same attributes (with or without "text as paths").

Whole-scene exports (draft blocks) keep the legacy untagged single-group structure; only piece-based exports are tagged.

## Document shape

```xml
<svg width="..." height="..." viewBox="..." xmlns="http://www.w3.org/2000/svg" ...>
  <g id="pattern-1" data-type="pattern" data-type-number="1" data-name="Pattern Name">
    <g id="piece-1" data-type="piece" data-type-number="1" data-parent="pattern-1"
       data-name="Front Bodice" data-letter="A">
      <g id="piece-1-seamline-1" data-type="seamline" data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-cutline-1"  data-type="cutline"  data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-notch-1"    data-type="notch"    data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-internal_path-1" data-type="internal_path" data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-cut_path-1"      data-type="cut_path"      data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-grainline-1"     data-type="grainline"     data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-piece_label-1"   data-type="piece_label"   data-type-number="1" data-parent="piece-1">…</g>
      <g id="piece-1-pattern_label-1" data-type="pattern_label" data-type-number="1" data-parent="piece-1">…</g>
    </g>
    <g id="piece-2" data-type="piece" data-type-number="2" data-parent="pattern-1" data-name="Back Bodice">…</g>
  </g>
</svg>
```

## Attributes

| Attribute | Applies to | Value |
|---|---|---|
| `data-type` | every tagged `<g>` | One of `pattern`, `piece`, `seamline`, `cutline`, `internal_path`, `cut_path`, `grainline`, `notch`, `piece_label`, `pattern_label`. More types may be added later; consumers must ignore unknown types gracefully. |
| `data-type-number` | every tagged `<g>` | Per-scope 1-based counter for that `data-type`. The pattern is always `1`; pieces count up across the file; component counters reset per piece and per type. |
| `data-parent` | `piece` and component groups | For a piece: the pattern group's `id` (`pattern-1`). For a component: the owning piece group's `id` (e.g. `piece-3`). The pattern group has no `data-parent` (it is the root). |
| `data-name` | `pattern`, `piece` | Pattern name, or piece name. Omitted when empty. |
| `data-letter` | `piece` | The piece letter, only when one is set on the piece. |

## `id` scheme

- Pattern: `pattern-1` (one pattern per file).
- Piece *n*: `piece-<n>` (n = `data-type-number` of the piece).
- Component: `<pieceId>-<type>-<m>` (e.g. `piece-2-internal_path-3`), where *m* is that type's counter within the piece.

All ids are unique and XML-valid by construction. **Breaking change vs. pre-contract exports:** the piece `id` was previously the raw piece name; the name now lives in `data-name`.

## Guarantees

- Every `<g>` under `pattern-1` carries `data-type`, `data-type-number`, and `data-parent`.
- No empty `<g>` elements and no spurious `M0,0` / empty-`d` paths (Qt generator artifacts are stripped).
- Components that paint nothing (e.g. a piece without notches or internal paths) are simply absent — consumers must not assume every type exists in every piece.
- Component geometry is emitted in the merged document's single coordinate space (the flat-arranged paper; `viewBox` in scene units at the generator resolution). No transforms are introduced beyond what Qt's SVG generator emits inside the groups.
- Label groups contain real `<text>` elements when "text as paths" is off (label lines are rendered by `SvgTextItem`, `src/libs/vlayout/svg_text_item.cpp`, which paints through `QPainter::drawText()` so Qt's SVG engine emits `<text>` with the label's `font-family`, `font-size`, `font-weight`/`font-style` and fill color), and `<path>` glyph outlines when on (`--text2paths` / "text as paths"); the Layout Mode handoff always keeps real text.

## Semantics / notes

- **`seamline`** — the sew line of the piece.
- **`cutline`** — the seam-allowance outline (cut line). Pieces drawn without a seam allowance may have no `cutline` group.
- **`notch`** — all notches of a piece in one group for now; per-notch splitting is a possible follow-on if the nesting algorithm needs individual notches.
- **`internal_path`** — one group per plain (non-cutout) internal path of the piece.
- **`cut_path`** — one group per internal *cutout* path: a closed path that is cut out of the piece (a hole) and may carry its own seam allowance. Distinguished in the pattern data by `VLayoutPiecePath::isCutPath()`; the nesting algorithm may treat cutout interiors as usable area, unlike `internal_path` markings.
- **`grainline`** — grainline arrow geometry.
- **`piece_label` / `pattern_label`** — the on-piece label text blocks. One `<text>` element per label line (or one `<path>` per line in text-as-paths mode); per-line bold/italic, alignment, middle-eliding to the label width, mirroring and rotation are preserved in either mode.
- Counters are per SvgGenerator instance: one instance = one file = one pattern.
