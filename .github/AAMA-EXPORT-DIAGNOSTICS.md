# AAMA export failures

AAMA export validates all piece block names and the contours it will write
before it opens the output file. Empty names and duplicate DXF block names
are rejected. Duplicate checks include case folding and the space-to-underscore
conversion used for R10/R12 block names.

Cutting contours and visible stitch contours must contain at least three
distinct, finite points and enclose finite, non-zero area. A failure identifies
the piece and contour. This check is not a full self-intersection or CAD
compliance validator.

Validation failures leave an existing output file untouched. The DXF writer
also checks stream open/flush/close status. A failed write is returned to the
application with the destination path; batch exports report failure instead
of returning success. A mid-write I/O failure can still leave a partial file.
The application does not claim an atomic replacement transaction.

This change does not add ASTM, grade-rule tables, or automatic mirrored-copy
semantics. Existing cut/stitch layers and successful AAMA geometry are retained.
