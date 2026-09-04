# Module: Rendering and Export

Parent: [Architecture.md](./Architecture.md) · Nesting: [Module_LayoutNesting.md](./Module_LayoutNesting.md)

Output is **Qt Graphics View** for interactive canvases, then the same scenes (or layout item trees) painted into format-specific `QPaintDevice`s. Apparel DXF (AAMA) is a structured walk of `VLayoutPiece`, not a screenshot.

---

## 1. Interactive rendering

| Class | Path | Role |
|-------|------|------|
| `VMainGraphicsScene` | `vwidgets` | Scene: selection, hover, origins, piece-lock signals |
| `VMainGraphicsView` | `vwidgets` | Zoom/pan/rubber-band; `GraphicsViewZoom` wheel/pinch |
| `MainWindow` | `seamly2d` | Owns `draftScene`, `pieceScene`; swaps `ui->view` |
| `MainWindowsNoGUI` | `seamly2d` | Layout generate, print, export (GUI + CLI) |

Modes (`Draw` in `vgeometrydef.h`):

| UI | `Draw` | Scene |
|----|--------|-------|
| Draft | `Calculation` | `draftScene` |
| Pieces | `Modeling` | `pieceScene` |
| Layout | `Layout` | `scenes[]` per page + `tempSceneLayout` |

Scene size constant: `SceneSize 50000` (`def.h`). Handle size 12 px.

### Scene items (`vwidgets`)

`VScenePoint` + `VGraphicsSimpleTextItem`, `VSimplePoint`/`VSimpleCurve`, `VCurvePathItem`, `VControlPointSpline`, `VScaledLine`/`VScaledEllipse`/`ArrowedLineItem`, `VPieceItem` → `VTextGraphicsItem`/`VGrainlineItem`, `ResizeHandle`, `NonScalingFillPathItem`, `SceneRect`.

Tools own persistent items; `Visualization` items are ephemeral ([Module_DrawingTools.md](./Module_DrawingTools.md)).

---

## 2. Export architecture

```
Scene or VLayoutPiece tree
        │
        ├─ SVG          SvgGenerator (vformat) — Qt SVG → DOM merge/cleanup/groups
        ├─ PDF          QPrinter PdfFormat + scene->render
        ├─ PDF tiled    PrintPages + VPoster (glue, row/col labels)
        ├─ Raster       QImage + scene->render  (PNG JPG BMP PPM TIF)
        ├─ OBJ          VObjPaintDevice / VObjEngine (Delaunay of fills)
        ├─ PS/EPS       temp PDF → external pdftops
        ├─ DXF Flat     VDxfPaintDevice / VDxfEngine (QPainter entities)
        └─ DXF AAMA     ExportToAAMA (layers + blocks per piece)
```

UI: `ExportLayoutDialog`, `ExportFormatCombobox` (probes `pdftops`), `ExportProgressDialog`.

---

## 3. Format matrix (`LayoutExportFormat` in `def.h`)

| Id | Format | Engine | Notes |
|----|--------|--------|-------|
| 0 | SVG | `SvgGenerator` | Group-aware DOM |
| 1 | PDF | `QPrinter` | |
| 2 | PDFTiled | `QPrinter` + `VPoster` | Layout only |
| 3–6, 37 | PNG JPG BMP PPM TIF | `QImage` | |
| 7 | OBJ | `VObjEngine` | Wavefront; `MAX_POINTS` 512 per path |
| 8–9 | PS EPS | PDF → `pdftops` | Hidden if binary missing |
| 10–18 | DXF Flat R10–R2013 | `VDxfEngine` | Optional binary |
| 19–27 | DXF AAMA R10–R2013 | `ExportToAAMA` | Apparel layers |
| 28–36 | DXF ASTM R10–R2013 | **stub** | Enum only; **not in UI** |

Draft/Piece export skips some layout-only formats (PDFTiled; OBJ often limited).

---

## 4. DXF (`src/libs/vdxf` + vendored `libdxfrw`)

**Flat:** `QPainter` recording → `DRW_LWPolyline` (newer ACAD) or `DRW_Polyline` (old), `DRW_Line`, `DRW_Ellipse`, `DRW_Text`, `DRW_Insert`. Units forced to mm in the flat path. Versions `DRW::Version` AC1006 (R10) … AC1027 (2013).

**AAMA:** structured layers — outline `"1"`, draw/intcut `"8"`/`"11"`, notches `"4"`, grainline `"7"`, text; **blocks per piece** + inserts. This is what apparel plotters expect.

**ASTM:** values exist for future work; UI list is commented out. Do not document it as a shipping exporter.

---

## 5. OBJ (`src/libs/vobj`)

`VObjEngine` fills paths, **Delaunay-triangulates** (`delaunay.cpp`), writes `v`/`f`. Coordinates normalized roughly to [-1,1]. Used for 3D-ish preview, not production markers. Path vertex cap 512.

---

## 6. SVG

`SvgGenerator` renders scenes to SVG then post-processes DOM (merge, cleanup, groups) so pieces stay selectable in Inkscape. Better than raw `QSvgGenerator` for multi-piece layouts.

---

## 7. Print

`PrintOrigin` / `PrintTiled` / `PrintPreview*` → `PrintPages`. Tiling: `VPoster` grid with glue allowance and sheet labels. Paper templates shared with layout settings.

---

## 8. CLI (`VCommandLine` / `vcmdexport`)

Enabled by `--basename` / `-b` (export mode). Same `MainWindowsNoGUI` path, headless.

| Option | Role |
|--------|------|
| `--basename` / `-b` | Output base name (enables export) |
| `--destination` | Folder |
| `--measurefile` | Override measurements |
| `--format` | `LayoutExportFormat` numeric id |
| `--binarydxf` | Binary DXF |
| `--textaspaths` | Outline text |
| `--exportonlydetails` | Pieces without nesting |
| `--pagetemplate`, `--pagew/h`, `--pageunits` | Paper |
| `--ignore-margins`, margin opts | Printer fields |
| `--rotate` | Increment (0 = off) |
| `--crop`, `--unite`, `--savelength` | Layout |
| `--gapwidth`, `--shiftlength`, `--groupping` | Gap / shift / bank case |
| `--gradationsize` / `--gradationheight` | Multisize |
| `--test` | Headless test |

Option name constants: `vmisc/commandoptions.h`. Builds `VLayoutGenerator` via helpers shared with `LayoutSettingsDialog`.

---

## 9. Why paint-device exporters

SVG, PDF, raster, Flat DXF, and OBJ all consume **already laid-out graphics**. One nest, many files. AAMA is the exception because plotters need layers, not a pretty picture.

Related: [Module_LayoutNesting.md](./Module_LayoutNesting.md), [Module_Applications.md](./Module_Applications.md).
