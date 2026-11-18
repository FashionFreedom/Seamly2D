# AAMA cutting data

The AAMA exporter writes one block and one INSERT for each exported piece.
Geometry uses millimetres, regardless of the pattern's construction units.
Global `Units: MM` text also declares that unit for legacy DXF versions that
cannot carry the modern unit header. Readers do not need a size heuristic.

Each piece block contains these plain TEXT fields on layer 1:

| Field | Source |
|---|---|
| `Piece Name: ...` | Native piece name |
| `Quantity: ...` | Native piece-label data's total cutting quantity |
| `Size: ...` | Selected multisize size, expressed in centimetres |

These field names are stable English interchange keys. They are independent
of label visibility, font fitting, translated words such as "Cut", and the
user's label template. Matching fields in the visible label are omitted from
the additional label text, so a custom template cannot override the native
quantity or selected size. Other visible label text is preserved. Flat DXF,
SVG, PDF and on-screen label drawing are unchanged.

Quantity is a total count. It does not specify how many copies are mirrored.
Permission to flip a piece does not imply that half its copies are mirrored.
No mirrored-count or fabric name is invented. A visible label can supply an
additional `Material: cotton` line when that convention is agreed with the
receiving application. There is currently no separate native fabric field.

The existing geometry layers remain:

| Layer | Contents |
|---|---|
| 1 | Cutting outline and piece text |
| 4 | Notch segments |
| 7 | Visible grain line |
| 8 | Visible stitch outline and internal drawing paths |
| 11 | Internal cutting paths |

Use a separate export for each required size with `--gsize` and `--gheight`,
and include those selections in the filename. These files contain evaluated
geometry, not a grade-rule table. This convention does not add ASTM support
or claim certification against an ASTM specification.
