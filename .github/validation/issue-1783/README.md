# PR #1783: independent DXF validation, 24 October 2026

The reports below were downloaded directly from [ApparelWerks DXF Inspector](https://aw.fyi/dxf) after its full analysis completed. They are unmodified service-generated PDFs, with images. These exports are ANSI/AAMA-292 files; they are not ASTM D6673 exports or a claim of complete standards compliance.

## Reports and results

| Case | PDF report | Result |
| --- | --- | --- |
| Baseline, hidden labels, R12 | [PDF](reports/baseline-hidden-r12.pdf) | Piece names and units missing; quantities/sizes absent from the piece data. Ten departures. |
| This PR, hidden labels, R12 | [PDF](reports/fields-hidden-r12.pdf) | Names, quantity 2, size 50, and explicit millimetres recognized. Seven remaining departures. |
| This PR, visible labels, R12 | [PDF](reports/fields-visible-r12.pdf) | Same native metadata, plus visible material text. Seven departures and one permitted optional-text extension. |
| This PR, visible labels, AC1027 | [PDF](reports/fields-visible-ac1027.pdf) | Same metadata. Eight departures and one optional-text extension; the extra departure is the modern DXF version. |

The baseline hidden export also lacks the style system-text block. Adding the independent Units field removes that finding as well as the missing Units and Piece Name findings. The remaining R12 departures concern style name, sample size, grade-rule table identifier, creation date/time, author, and notch representation. They also occur in the baseline exporter and are not fixed by this narrow piece-metadata change. Quantity and size are recognized by the inspector's piece table; they do not constitute grade rules.

## Reproduce

All inputs are original synthetic data under the Apache-2.0 license in `inputs/LICENSE`, not customer or factory patterns. Use the native files and measurement table in [inputs](inputs/). The hidden variant preserves its original relative measurement reference in `inputs/variants/hidden-labels/skirt-block.sm2d`. The fixture is not a fit-tested garment block.

From this directory, with an absolute private state path set in `SEAMLY_DATA_DIR`:

```sh
QT_QPA_PLATFORM=offscreen seamly2d --basename fields-visible-r12 \
  --destination ./out --format 20 --exportOnlyDetails --gsize 50 --gheight 176 \
  inputs/skirt-block.sm2d
```

Create `out` first. Use format 27 for AC1027, and the hidden input path for the hidden-label case. Upload the resulting DXF to the inspector and download its PDF. [exports](exports/) contains the exact files submitted and per-export manifests with SHA-256 hashes. `SHA256SUMS.txt` covers the evidence files.

Baseline source: `651ddec` (latest develop plus #1782; no export changes). Tested metadata source: `2fa9ba8`, rebuilt on corrected #1782. Toolchain: Qt 6.11.1, MinGW GCC 13.1.0, Windows 11. All application runs used the offscreen platform.

The focused QtTest run in [qt-tests.txt](qt-tests.txt) passed 18 checks, with four NTFS permission-bit cases skipped. Four export-specific cases cover hidden and visible labels in both DXF versions. The visible template deliberately supplies incorrect names, sizes, and quantities; the test requires exactly one correct native field of each kind and preserves unrelated label text. Separate real NTFS ACL checks for #1782 passed 4/4.

PDF reports remain authored by ApparelWerks LLC. The input-data license does not relicense those reports or the Seamly application.
