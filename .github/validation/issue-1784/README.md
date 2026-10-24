# PR #1784: independent DXF validation, 24 October 2026

The reports are unmodified PDFs downloaded from [ApparelWerks DXF Inspector](https://aw.fyi/dxf) after full analysis, including images. Successful output is still AAMA, with existing compliance departures. This PR validates input geometry/names and reports failed writes; it does not claim complete AAMA/ASTM conformance.

| Case | PDF report | Result |
| --- | --- | --- |
| Baseline R12 | [PDF](reports/baseline-visible-r12.pdf) | Eight existing departures and one optional-text extension. |
| This PR R12 | [PDF](reports/validation-visible-r12.pdf) | Same existing departures. |
| This PR AC1027 | [PDF](reports/validation-visible-ac1027.pdf) | Nine departures and one optional-text extension; the extra departure is the modern DXF version. |

The R12 departures concern style name, sample size, grade-rule table identifier, missing units text, creation date/time, author, and notches drawn as segments. The optional material text is reported as a permitted extension. These findings also occur without this PR. PR #1783 addresses independent piece metadata and units text separately.

## Successful-output comparison

The original [input skirt and measurement table](inputs/) are synthetic Apache-2.0 test data, not factory or customer patterns, and are not fit-tested garment blocks. The input-data license is in `inputs/LICENSE`. [exports](exports/) holds the exact submitted files and manifests, including SHA-256 hashes. Reports remain authored by ApparelWerks LLC.

Baseline source: `651ddec` (latest develop plus the state-isolation PR #1782; no exporter changes). Diagnostics source: `ec25550` (this PR merged with latest develop). Local export build: `b7cdc0a`, which adds #1782's isolation commits only to keep testing out of the operator's profile. Those isolation commits are not part of the public #1784 branch. The vdxf, CLI export-diagnostics and DXF test sources are identical to `ec25550`.

Toolchain: Qt 6.11.1 / MinGW GCC 13.1.0 on Windows 11. Every app/test run used `QT_QPA_PLATFORM=offscreen`.

Both R12 and AC1027 successful exports have exactly equal parsed named-block data before/after this PR: cut/stitch/internal contours, grainline/notch segments, text, and entity counts. Raw bytes vary due to timestamps, block ordering and entity handles. [geometry-comparison.json](geometry-comparison.json) records the result. Re-run the independent comparison with:

```sh
python compare_exports.py exports/baseline-visible-r12_pieces.dxf exports/validation-visible-r12_pieces.dxf
python compare_exports.py exports/baseline-visible-ac1027_pieces.dxf exports/validation-visible-ac1027_pieces.dxf
```

The simple parser is an evidence check, not a full DXF reader or standards validator.

## Failure handling and ASTM

The [focused QtTest log](qt-tests.txt) reports 10 passes, no failures or skips. It exercises valid cut/stitch layers, empty/two-point/collinear/non-finite contours, duplicate normalized names, failed writes, empty exports and invalid page size. Invalid geometry keeps a pre-existing output file unchanged.

The application rejects `--format 29` (R12 ASTM) with exit 65 and no output in both baseline and diagnostics builds; see [astm-rejection.txt](astm-rejection.txt). There is no genuine ASTM export to submit for an ASTM report. No AAMA file was renamed or represented as ASTM.

To repeat an AAMA export, create an output directory and set `SEAMLY_DATA_DIR` to an absolute private state directory when using #1782 locally, then run:

```sh
QT_QPA_PLATFORM=offscreen seamly2d --basename validation-visible-r12 \
  --destination ./out --format 20 --exportOnlyDetails --gsize 50 --gheight 176 \
  inputs/skirt-block.sm2d
```

Use format 27 for AC1027. `SHA256SUMS.txt` covers the evidence files.
