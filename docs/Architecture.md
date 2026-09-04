# Seamly2D Software Architecture

**Product:** Seamly2D — parametric vector patternmaking CAD for clothing  
**License:** GPLv3+ (application); third-party components retain their own licenses  
**Platforms:** Windows 10/11 (x64, ARM64), macOS 13+, Linux (AppImage, Flatpak)  
**Stack:** C++17-class Qt 6 (Widgets + Graphics View + XML + PrintSupport), qmake `SUBDIRS`  
**Document status:** High-level system design. Algorithm-level detail lives in the linked module files.

This document is the architectural entry point. It maps **every identified feature** and traces the three pillars:

| Pillar | Meaning in this codebase |
|--------|--------------------------|
| **Input** | Anthropometric measurements and size tables (`.smis` / `.smms`, legacy `.vit` / `.vst`) |
| **Instruction** | Constructive geometry, history-ordered dependency graph, formulas, pattern pieces |
| **Output / Render** | Draft / Piece / Layout scenes, nesting, print, and export (SVG, PDF, DXF, OBJ, raster, PS/EPS) |

---

## 1. System Overview

### 1.1 What the software is

Seamly2D is **not** a general drawing program and **not** a constraint CAD (no geometric constraint solver). It is a **formula-driven, history-ordered parametric CAD** for garment patterns:

1. A measurement file supplies body dimensions (one person, or a graded size/height table).
2. The patternmaker constructs points, lines, curves, and pieces with tools whose parameters are **formulas** over those measurements and over previously created geometry.
3. Changing a measurement, a custom variable, or an earlier tool causes a **replay** of the construction history (`LiteParse` / `FullParse`), which updates the canvas and the cut pieces.
4. Pieces are nested onto paper and exported or printed.

Two desktop applications share the same libraries:

| Application | Role |
|-------------|------|
| **Seamly2D** (`src/app/seamly2d`) | Pattern editor: Draft, Pieces, Layout; CLI export |
| **SeamlyMe** (`src/app/seamlyme`) | Measurement editor: individual and multisize tables |

Historical lineage: the project forked from **Valentina**. Legacy file extensions (`.val`, `.vit`, `.vst`) and many `V*` class names remain; current Seamly extensions are `.sm2d`, `.smis`, `.smms`.

### 1.2 Architectural pattern

The system is a **layered object-oriented Qt application** combining several well-known patterns:

```
┌─────────────────────────────────────────────────────────────────┐
│  Applications (Presentation)                                    │
│  MainWindow / TMainWindow  ·  dialogs  ·  docks  ·  CLI         │
├─────────────────────────────────────────────────────────────────┤
│  Interactive tools + undo (Controller)                          │
│  VAbstractTool family  ·  QUndoStack / VUndoCommand             │
│  Visualization previews  ·  property browser                    │
├─────────────────────────────────────────────────────────────────┤
│  Document + domain model                                        │
│  VPattern / VAbstractPattern (XML DOM + history)                │
│  VContainer (geometry + variables + pieces)                     │
│  MeasurementDoc  ·  VPiece                                      │
├─────────────────────────────────────────────────────────────────┤
│  Engines                                                        │
│  vgeometry  ·  Calculator/qmuparser  ·  vlayout  ·  export      │
├─────────────────────────────────────────────────────────────────┤
│  Infrastructure                                                 │
│  ifc (Xerces XSD + converters)  ·  vmisc  ·  settings  · i18n   │
└─────────────────────────────────────────────────────────────────┘
```

| Pattern | Where it appears | Why |
|---------|------------------|-----|
| **Document–View** (Qt Graphics View, not classic MVC) | `VPattern` + `VContainer` are the document; `VMainGraphicsScene` / `VMainGraphicsView` are views; tools mutate the document | A CAD canvas with selectable items maps naturally onto `QGraphicsScene` |
| **Command** | `src/libs/vtools/undocommands` on `QUndoStack` | Every construction edit is undoable and persisted as XML deltas |
| **Implicit DAG / history replay** | `m_history` + XML sibling order + `LiteParse` | Patternmaking is constructive; ordered replay replaces a constraint solver |
| **Copy-on-write snapshots** | `VContainer` / geometry via `QSharedDataPointer` | Tool *N* sees the world after tools *1…N* without deep copies |
| **Strategy / paint device** | `VDxfPaintDevice`, `VObjPaintDevice`, `SvgGenerator`, `QPrinter` | Same scene graph, many export backends |
| **Converter pipeline** | `VAbstractConverter` subclasses, version-by-version | Files from 0.1.x still open; schema is the contract |
| **Template Method** | `VAbstractTool::Create(..., Document parse)` | Full parse builds scene items; lite parse only recalculates |
| **Singleton-style app** | `VAbstractApplication` / `qApp` macro | Locale, settings, translations, logging |

There is **no** separate service layer, **no** database, and **no** plugin ABI. Features are compiled-in libraries.

### 1.3 Runtime modes (the three stages)

`enum class Draw { Calculation, Modeling, Layout }` (`vgeometrydef.h`) maps to the UI:

| UI mode | `Draw` | Scene | What the user does |
|---------|--------|-------|--------------------|
| **Draft** | `Calculation` | `draftScene` | Construct parametric geometry |
| **Pieces** | `Modeling` | `pieceScene` | Assemble closed pieces, seam allowance, notches, grainline, labels |
| **Layout** | `Layout` | per-page `QGraphicsScene` list | Nest pieces, print, export |

The same geometric objects change **presentation**, not identity: a draft spline becomes a piece-path node, then a nested outline.

---

## 2. Directory Structure Map

```
Seamly2D.pro                 # root SUBDIRS: src, out
├── src/
│   ├── libs/                # ★ core logic — all domain engines
│   ├── app/
│   │   ├── seamly2d/        # pattern editor
│   │   ├── seamlyme/        # measurement editor
│   │   └── share/samples/   # bundled .sm2d / .smis / .smms
│   └── test/                # Seamly2DTest, CollectionTest, ParserTest, TranslationsTest
├── share/translations/      # .ts catalogs (Weblate)
├── dist/                    # NSIS, desktop files, MIME, packaging
├── scripts/                 # version, translations, helpers
├── out/                     # packaging / deploy qmake project
├── docs/                    # this architecture set
└── .github/                 # CI, issue templates, developer README
```

### 2.1 Core logic libraries (`src/libs`)

| Library | Responsibility | Pillar |
|---------|----------------|--------|
| **ifc** | XML DOM (`VDomDocument`), Xerces XSD validation, schema resources, version converters, `VAbstractPattern`, exceptions | Input + Instruction (persistence) |
| **vformat** | `MeasurementDoc` I/O; SVG generator; label templates | Input + Output |
| **vpatterndb** | `VContainer`, variables, formulas, pieces, measurement catalog (246 names), PM systems | Input + Instruction |
| **vgeometry** | Pure geometry value types: points, arcs, cubics, paths | Instruction |
| **vtools** | All drafting/piece tools, dialogs, visualization, undo commands | Instruction |
| **qmuparser** | Vendored formula parser (muParser fork) | Instruction |
| **vlayout** | Nesting / packing, tiled poster print, layout piece items | Output |
| **vdxf** | DXF paint engine + vendored libdxfrw (Flat + AAMA) | Output |
| **vobj** | Wavefront OBJ via Delaunay triangulation of fills | Output |
| **vwidgets** | Graphics view/scene, scene items, comboboxes, calculator, pen toolbar | Output (canvas) + UI |
| **vpropertyexplorer** | Property grid for live tool options | UI |
| **vmisc** | Units, enums (`Tool`, `LayoutExportFormat`), settings, logging, CLI option names, diagrams | Cross-cutting |
| **tools** | Background/reference **images** in Draft (not the drafting toolbox) | Instruction |
| **fervor** | GitHub-release auto-updater | App |
| **vtest** | Shared test helpers | Quality |
| **xerces-c** | Bundled headers/libs for Windows/macOS; system lib on Linux | Input (validate) |

**qmake dependency direction (simplified):**

```
qmuparser, vpropertyexplorer
        ↓
ifc, vgeometry, vmisc
        ↓
vpatterndb, vformat
        ↓
vwidgets, vtools, vlayout, vdxf, vobj, tools
        ↓
app (seamly2d, seamlyme)
        ↓
test
```

`app.depends = libs`. Tests depend on libs. `out` (packaging) depends on `src`.

### 2.2 Application layout (`src/app/seamly2d`)

| Path | Role |
|------|------|
| `main.cpp` | Boot, resources, `Application2D` |
| `core/` | `Application2D`, `VCommandLine` (CLI export), formula property editors |
| `xml/vpattern.*` | Concrete pattern document: parse dispatch, tool factory |
| `mainwindow.*` | GUI: modes, toolbars, docks, file/measurement binding |
| `mainwindowsnogui.*` | Headless layout, print, export (shared with CLI) |
| `dialogs/` | App-level dialogs (preferences, layout, history, variables, …) |

SeamlyMe: `application_me.*`, `tmainwindow.*`, `vlitepattern.*` (thin pattern reader to import required measurement names).

---

## 3. Component Architecture

### 3.1 Component interaction

```
                    ┌────────────┐     spawn / IPC      ┌────────────┐
                    │  Seamly2D  │◄────────────────────►│  SeamlyMe  │
                    └─────┬──────┘                      └──────┬─────┘
                          │                                    │
                          │ VPattern.MPath()                   │ MeasurementDoc
                          ▼                                    ▼
                    ┌──────────────┐                    .smis / .smms
                    │  VContainer  │◄──── MeasurementVariable ─┘
                    │  gObjects    │
                    │  variables   │
                    │  pieces      │
                    └──────┬───────┘
           ┌───────────────┼────────────────┐
           ▼               ▼                ▼
     vgeometry        Calculator        VPiece
     (VGObject)       (qmuparser)       (path, SA, notches)
           │               │                │
           └───────────────┼────────────────┘
                           ▼
                    VAbstractTool / history
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
         draftScene   pieceScene    VLayoutGenerator
              │            │            │
              └────────────┼────────────┘
                           ▼
                    Export / Print
              SVG PDF DXF OBJ raster PS/EPS
```

### 3.2 Key types (integration, not APIs)

| Type | Home | Role in the system |
|------|------|--------------------|
| `VDomDocument` | ifc | QDomDocument + Xerces validate + save |
| `VAbstractPattern` | ifc | Pattern XML tags, draft blocks, groups, history cursor, measurement path |
| `VPattern` | seamly2d | Parses XML into tools; `FullParse` / `LiteParse` / `LiteBlockParse` |
| `VContainer` | vpatterndb | ID → geometry; name → variable; pieces and piece paths |
| `VGObject` | vgeometry | Base of all geometric primitives |
| `VInternalVariable` | vpatterndb | Measurements, custom vars, derived lengths/angles |
| `VFormula` / `Calculator` | vpatterndb | Localized formula → qmuparser → number |
| `VAbstractTool` | vtools | One history node: XML + scene item + container snapshot |
| `VPiece` | vpatterndb | Closed garment piece with SA, notches, labels, grainline |
| `VLayoutPiece` / `VLayoutGenerator` | vlayout | Nesting input and orchestrator |
| `MeasurementDoc` | vformat | Load/save measurement XML into `VContainer` |
| `MeasurementVariable` | vpatterndb | One body dimension; graded value for multisize |

### 3.3 How components stay decoupled

- **Geometry does not know about XML or Qt widgets.** `vgeometry` is math + `QPointF`/`QPainterPath`.
- **ifc does not know about specific tools.** It stores generic history records (`VToolRecord`: id, `Tool` enum, draft-block name). `VPattern` in the app is the factory that maps XML element types to `VTool*`::`Create`.
- **Export does not re-implement geometry.** Layout and draft scenes are painted through `QPainter` into format-specific `QPaintDevice`s (or AAMA walks `VLayoutPiece` for layered apparel DXF).
- **Measurements are not embedded in the pattern.** The pattern stores a **path** (`<measurements>`). Body data lives in a sibling file so one pattern can retarget many size tables.

---

## 4. Data Flow / Lifecycle

End-to-end path: **load measurements → apply geometric instructions → update the canvas → nest → export**.

### 4.1 Pillar 1 — Input: measurements and size tables

```
.smis / .smms  (or legacy .vit / .vst)
        │
        ▼
 IndividualSizeConverter / MultiSizeConverter
        │  step through XSD versions → current
        ▼
 Xerces validates against
   individual v0.3.4  or  multisize v0.4.5
        │
        ▼
 MeasurementDoc::readMeasurements()
        │  UnitConvertor(file unit → pattern unit)
        ▼
 VContainer.variables["bust"] = MeasurementVariable
```

**Individual (`.smis`):** one person. Each `<m>` has a numeric value **or a formula** over other measurements. Optional personal block (name, birth date, gender, email). Custom names use `@` prefix.

**Multisize (`.smms`):** a size chart. File stores `baseSize`, `baseHeight`, and per-measurement `base`, `size_increase`, `height_increase`. Runtime value:

```
k_size   = (currentSize   - baseSize)   / sizeIncrement    // sizeIncrement  = 2 cm
k_height = (currentHeight - baseHeight) / heightIncrement  // heightIncrement = 6 cm
value    = base + k_size * size_increase + k_height * height_increase
```

Inch **multisize** is rejected at runtime. Gradation grids: sizes 22–72 step 2; heights 50–200 step 6 (cm).

The pattern binds via `VAbstractPattern::MPath()`. Seamly2D checks that every measurement token used in formulas exists in the loaded file (`checkRequiredMeasurements`). Changing size/height in the UI updates `VContainer` static size/height; `MeasurementVariable::GetValue()` recomputes; then `LiteParse` rebuilds geometry.

Catalog: **246** known anthropometric names in groups **A–Q**, plus **56 patternmaking systems** (`p0`–`p54`, `p998`) as metadata (Aldrich, GOST, Bunka, …). Systems do not switch calculation engines.

Deep dive: [Module_Measurements.md](./Module_Measurements.md)

### 4.2 Pillar 2 — Instruction: formulas, relations, history DAG

There is **no explicit graph library**. The DAG is:

1. **XML order** of children under `<calculation>` (per draft block).
2. **`m_history`**: `QVector<VToolRecord>` — the same order, with a cursor for “insert earlier”.
3. **Parent IDs** on each tool (`basePoint`, curve id, …).
4. **Formula tokens** that name measurements or derived variables (`Line_A_B`, `AngleLine_A_B`, curve lengths). Those variables exist only after earlier tools ran.
5. **`referens` counts** so a tool cannot be deleted while dependents exist.

Recalculation:

| Parse mode | Effect |
|------------|--------|
| `FullParse` | Clear scenes, tools, history, data; rebuild everything |
| `LiteParse` | Keep scene items; clear derived variables; replay `Create(..., LiteParse)` and `UpdateGObject` |
| `LiteBlockParse` | Same as lite, current draft block only |

Formula pipeline:

```
Localized user text
    → VTranslateVars::FormulaFromUser
Internal English tokens
    → QmuTokenParser + Calculator (qmuparser)
    → bind VContainer::variables
    → number (pattern units)
```

Relations are **constructive**, not declarative. “Point C is 5 cm from B at 90° from AB” is an `EndLine` / `AlongLine` / `Normal` tool, not a free constraint. To change the relation, the user edits that tool; undo writes XML; parse replays.

Deep dives:

- [Module_DependencyTree.md](./Module_DependencyTree.md)
- [Module_GeometryEngine.md](./Module_GeometryEngine.md)
- [Module_BezierMath.md](./Module_BezierMath.md)
- [Module_FormulaEngine.md](./Module_FormulaEngine.md)
- [Module_DrawingTools.md](./Module_DrawingTools.md)
- [Module_PatternPieces.md](./Module_PatternPieces.md)

### 4.3 Pillar 3 — Output: canvas, nesting, export

```
Draft tools ──► draftScene (VMainGraphicsScene)
                     │
PatternPieceTool ──► VPiece ──► pieceScene
                     │
              preparePiecesForLayout()
                     │
              VLayoutPiece::Create
                     │
              VLayoutGenerator::Generate
                     │  VBank → VLayoutPaper → VPosition (thread pool)
                     │  edge pairing + rotation → VBestSquare
                     ▼
              layout page scenes
                     │
        ┌────────────┼────────────────────────────┐
        ▼            ▼                            ▼
     Print       ExportScene                  CLI (--basename)
  (QPrinter,   SVG / PDF / raster /         same path, headless
   VPoster)    OBJ / DXF Flat|AAMA /
               PS·EPS (pdftops)
```

Nesting is **edge-combination packing**, not a commercial marker optimizer: candidate transforms are scored by bounding square (optional save-length), with layout-allowance offset to keep a gap. ASTM DXF enum values exist but are **not offered in the UI**.

Deep dives:

- [Module_LayoutNesting.md](./Module_LayoutNesting.md)
- [Module_ExportRender.md](./Module_ExportRender.md)

### 4.4 File open lifecycle (pattern)

```
User opens .sm2d / .val
    → VPatternConverter (0.1.0 … 0.7.4) + Xerces (v0.7.4.xsd)
    → VPattern::setXMLContent
    → loadMeasurements() if <measurements> path is set
    → VPattern::Parse(FullParse)
         Parse custom variables
         For each draft block: ParseDraftStage
              point | line | spline | arc | elArc | tools | operation | path
              → VTool*::Create → AddGObject + derived variables + scene items
         Parse pieces / modeling nodes
    → draftScene displays Construction stage
```

Edit a formula → `SaveToolOptions` undo command → `NeedLiteParsing` → canvas updates without destroying items.

---

## 5. Feature Completeness Matrix

Coverage rule: every user-visible or architecturally distinct capability appears once. **Status** is code-backed (present in this tree). Deep-dive column is `—` when this file already contains the integration view.

### 5.1 Applications and shell

| ID | Feature | Location | Deep dive |
|----|---------|----------|-----------|
| A1 | Seamly2D pattern editor | `src/app/seamly2d` | [Module_Applications.md](./Module_Applications.md) |
| A2 | SeamlyMe measurement editor | `src/app/seamlyme` | [Module_Applications.md](./Module_Applications.md) |
| A3 | Draft / Pieces / Layout modes | `Draw` enum, `MainWindow::show*Mode` | [Module_Applications.md](./Module_Applications.md) |
| A4 | Toolbars (file, mode, draft, points, lines, curves, arcs, operations, pieces, layout, zoom, view, pen) | `mainwindow.ui` + `PenToolBar` | [Module_Applications.md](./Module_Applications.md) |
| A5 | Docks: tool properties, toolbox, groups, pieces, layout pages | `MainWindow` | [Module_Applications.md](./Module_Applications.md) |
| A6 | Property browser for tool options | `vpropertyexplorer` + `VToolOptionsPropertyBrowser` | [Module_Applications.md](./Module_Applications.md) |
| A7 | Preferences: configuration, pattern, paths, graphics view | `DialogPreferences` + pages | [Module_Applications.md](./Module_Applications.md) |
| A8 | SeamlyMe preferences: configuration, paths | `DialogSeamlyMePreferences` | [Module_Applications.md](./Module_Applications.md) |
| A9 | Welcome dialogs (locale/units first run) | both apps | [Module_Applications.md](./Module_Applications.md) |
| A10 | About dialogs | both apps | [Module_Applications.md](./Module_Applications.md) |
| A11 | Keyboard shortcuts reference | `ShortcutsDialog`, `MeShortcutsDialog` | [Module_Applications.md](./Module_Applications.md) |
| A12 | Themes (Light/Dark/Twilight Fusion, System, classic, Windows11) | `AppTheme`, `Application2D::setTheme` | [Module_Applications.md](./Module_Applications.md) |
| A13 | Auto-update from GitHub releases | `fervor` / `FvUpdater` | [Module_Applications.md](./Module_Applications.md) |
| A14 | Multi-window SeamlyMe + local socket | `ApplicationME` | [Module_Applications.md](./Module_Applications.md) |
| A15 | Launch SeamlyMe from Seamly2D | `MainWindow` (Ctrl+M) | [Module_Applications.md](./Module_Applications.md) |
| A16 | Calculator dialog | `CalculatorDialog` + `vwidgets/calculator` | [Module_Applications.md](./Module_Applications.md) |
| A17 | Decimal chart dialog | `DecimalChartDialog` | [Module_Applications.md](./Module_Applications.md) |
| A18 | Show-info dialog | `ShowInfoDialog` | [Module_Applications.md](./Module_Applications.md) |
| A19 | Date/time format dialog (labels) | `DialogDateTimeFormats` | [Module_Applications.md](./Module_Applications.md) |
| A20 | New pattern dialog | `DialogNewPattern` | [Module_Applications.md](./Module_Applications.md) |
| A21 | Pattern properties (metadata) | `DialogPatternProperties` | [Module_Applications.md](./Module_Applications.md) |
| A22 | Headless / no-GUI export path | `MainWindowsNoGUI` | [Module_ExportRender.md](./Module_ExportRender.md) |

### 5.2 Input — measurements (Pillar 1)

| ID | Feature | Location | Deep dive |
|----|---------|----------|-----------|
| I1 | Individual measurements `.smis` | `MeasurementDoc`, schema v0.3.4 | [Module_Measurements.md](./Module_Measurements.md) |
| I2 | Multisize / size tables `.smms` | `MeasurementDoc`, schema v0.4.5 | [Module_Measurements.md](./Module_Measurements.md) |
| I3 | Legacy `.vit` / `.vst` load + migrate | converters rename roots | [Module_Measurements.md](./Module_Measurements.md) |
| I4 | 246 known measurements, groups A–Q | `measurements_def.*` | [Module_Measurements.md](./Module_Measurements.md) |
| I5 | Custom `@` measurements + formulas | `MeasurementVariable::isCustom` | [Module_Measurements.md](./Module_Measurements.md) |
| I6 | Personal data (name, birth, gender, email, notes) | `MeasurementDoc` | [Module_Measurements.md](./Module_Measurements.md) |
| I7 | Patternmaking system codes p0–p54, p998 | `pmsystems.*` | [Module_Measurements.md](./Module_Measurements.md) |
| I8 | Units mm / cm / inch (+ px for graphics) | `Unit`, `UnitConvertor` | [Module_Measurements.md](./Module_Measurements.md) |
| I9 | Size/height gradation UI | Seamly2D + SeamlyMe | [Module_Measurements.md](./Module_Measurements.md) |
| I10 | Known-measurement database dialog + SVG diagrams | `MeasurementDatabaseDialog` | [Module_Measurements.md](./Module_Measurements.md) |
| I11 | Import measurement names from a pattern | `VLitePattern` | [Module_Measurements.md](./Module_Measurements.md) |
| I12 | Templates and sample files | `src/app/share/samples/measurements` | [Module_Measurements.md](./Module_Measurements.md) |
| I13 | CSV export of measurements | `DialogExportToCSV` | [Module_Measurements.md](./Module_Measurements.md) |
| I14 | Print measurement table | `TMainWindow` | [Module_Measurements.md](./Module_Measurements.md) |
| I15 | Read-only measurement files | `<read-only>` | [Module_Measurements.md](./Module_Measurements.md) |
| I16 | Bind / unload / change measurement file on pattern | `MainWindow::loadMeasurements` | [Module_Measurements.md](./Module_Measurements.md) |
| I17 | Required-measurement check | `checkRequiredMeasurements` | [Module_Measurements.md](./Module_Measurements.md) |
| I18 | Relative vs absolute measurement paths | `RelativeMPath` / `AbsoluteMPath` | [Module_Measurements.md](./Module_Measurements.md) |
| I19 | Individual XML converter 0.2.0 → 0.3.4 | `IndividualSizeConverter` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| I20 | Multisize XML converter 0.3.0 → 0.4.5 | `MultiSizeConverter` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| I21 | 3DLook / body-scan email conversion guidance | SeamlyMe (not a native format) | [Module_Measurements.md](./Module_Measurements.md) |
| I22 | Measurement name translation | `VTranslateMeasurements` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |

### 5.3 Instruction — geometry, DAG, tools, pieces (Pillar 2)

| ID | Feature | Location | Deep dive |
|----|---------|----------|-----------|
| G1 | Point primitive + label offsets | `VPointF` | [Module_GeometryEngine.md](./Module_GeometryEngine.md) |
| G2 | Circular arc (angles or length) | `VArc`, `VToolArc`, `VToolArcWithLength` | [Module_GeometryEngine.md](./Module_GeometryEngine.md) |
| G3 | Elliptical arc | `VEllipticalArc` | [Module_GeometryEngine.md](./Module_GeometryEngine.md) |
| G4 | Simple cubic spline (angles / kAsm / C lengths) | `VSpline` | [Module_BezierMath.md](./Module_BezierMath.md) |
| G5 | Four-point cubic Bézier | `VCubicBezier` | [Module_BezierMath.md](./Module_BezierMath.md) |
| G6 | Spline path / cubic Bézier path | `VSplinePath`, `VCubicBezierPath` | [Module_BezierMath.md](./Module_BezierMath.md) |
| G7 | Intersection and polar construction helpers | `VGObject` statics | [Module_GeometryEngine.md](./Module_GeometryEngine.md) |
| G8 | Cubic subdivision, arc length, Hobby handles | `VAbstractCubicBezier` | [Module_BezierMath.md](./Module_BezierMath.md) |
| G9 | Cut curve at length | `CutSpline`, `CutArc`, `CutSplinePath` | [Module_GeometryEngine.md](./Module_GeometryEngine.md) |
| D1 | History-ordered implicit DAG | `m_history`, XML order | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D2 | Full / lite / lite-block parse | `Document` enum, `VPattern::Parse` | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D3 | Per-tool `VContainer` snapshots (COW) | `VDataTool` | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D4 | Reference counting / delete safety | `IncrementReferens` | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D5 | Monotonic IDs + unique names | `VContainer::_id`, `uniqueNames` | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D6 | Draft blocks (named construction groups) | `VAbstractPattern` draft-block APIs | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D7 | History cursor (insert earlier) | `getCursorId` / `setCursorId` | [Module_DependencyTree.md](./Module_DependencyTree.md) |
| D8 | History dialog | `HistoryDialog` | [Module_Applications.md](./Module_Applications.md) |
| F1 | Formula evaluation (qmuparser) | `Calculator`, `qmuparser` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F2 | Localized formula translation | `VTranslateVars` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F3 | Custom pattern variables | `CustomVariable`, `DialogVariables` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F4 | Derived line length / line angle variables | `VLengthLine`, `VLineAngle` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F5 | Derived curve length, C-length, curve angles, arc radii | `VCurve*`, `VArcRadius` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F6 | Edit-formula dialog (Fx) | `EditFormulaDialog` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| F7 | Unit postfix operators in formulas (`cm`, `mm`, `in`) | `VTranslateVars` | [Module_FormulaEngine.md](./Module_FormulaEngine.md) |
| T1 | Base point | `VToolBasePoint` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T2 | End line (length + angle) | `VToolEndLine` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T3 | Along line / midpoint | `VToolAlongLine` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T4 | Shoulder point | `VToolShoulderPoint` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T5 | Normal (perpendicular) | `VToolNormal` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T6 | Bisector | `VToolBisector` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T7 | Height (foot of perpendicular) | `VToolHeight` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T8 | Line between two points | `VToolLine` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T9 | Line ∩ line | `VToolLineIntersect` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T10 | Line ∩ axis | `VToolLineIntersectAxis` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T11 | Curve ∩ axis | `VToolCurveIntersectAxis` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T12 | Point from X of P1 and Y of P2 | `PointIntersectXYTool` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T13 | Point of contact (circle–line) | `VToolPointOfContact` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T14 | Arc ∩ arc | `VToolPointOfIntersectionArcs` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T15 | Circle ∩ circle | `IntersectCirclesTool` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T16 | Curve ∩ curve | `VToolPointOfIntersectionCurves` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T17 | Point from circle and tangent | `IntersectCircleTangentTool` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T18 | Point from arc and tangent | `VToolPointFromArcAndTangent` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T19 | Triangle (right-triangle third vertex) | `VToolTriangle` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T20 | True darts (double point) | `VToolTrueDarts` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T21 | Spline / cubic Bézier / paths | `VToolSpline*` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T22 | Arc / arc-with-length / elliptical arc | `VToolArc*` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T23 | Move operation | `VToolMove` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T24 | Rotation operation | `VToolRotation` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T25 | Mirror by line | `VToolMirrorByLine` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T26 | Mirror by axis | `VToolMirrorByAxis` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| T27 | Interactive placement visualization | `visualization/` | [Module_DrawingTools.md](./Module_DrawingTools.md) |
| P1 | Pattern piece | `PatternPieceTool`, `VPiece` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P2 | Piece nodes (point/arc/elarc/spline/path) | `VNode*` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P3 | Internal paths | `InternalPathTool` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P4 | Anchor points | `AnchorPointTool` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P5 | Insert nodes into piece path | `InsertNodes` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P6 | Union of two pieces | `UnionTool` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P7 | Seam allowance (global + per-node + custom SA paths) | `VPiece` / `VPieceNode` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P8 | Notches (types, subtypes, cutline vs seamline) | `VPieceNode` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P9 | Grainline | `VGrainlineData` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P10 | Piece label + pattern-info label | `VPieceLabelData`, `VPatternLabelData` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P11 | Label templates | `VLabelTemplate`, schema 1.0.0 | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| P12 | Piece color, lock, include-in-layout | undo commands + `PiecesWidget` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| P13 | Node join angles (length, symmetry, right angle, …) | `PieceNodeAngle` | [Module_PatternPieces.md](./Module_PatternPieces.md) |
| U1 | Undo / redo stack | `QUndoStack` + `VUndoCommand` | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U2 | Add/delete/save tool options | `AddToCalc`, `DelTool`, `SaveToolOptions` | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U3 | Drag points / splines | `MoveSPoint`, `MoveSpline*` | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U4 | Draft-block add/rename/delete | undo commands | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U5 | Piece add/delete/move/options | undo commands | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U6 | Group add/delete/move items | undo commands | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| U7 | Broken-parse auto-undo | `UndoEvent` | [Module_UndoCommands.md](./Module_UndoCommands.md) |
| X1 | Named groups (visibility, lock, pen) | `VAbstractPattern` + `GroupsWidget` | [Module_Applications.md](./Module_Applications.md) |
| X2 | Background / reference images | `src/libs/tools` (`ImageTool`) | [Module_Applications.md](./Module_Applications.md) |
| X3 | Line color / type / weight defaults | `PenToolBar`, pattern defaults | [Module_Applications.md](./Module_Applications.md) |
| X4 | Point-name visibility and move labels | undo label commands | [Module_UndoCommands.md](./Module_UndoCommands.md) |

`Tool` enum sentinel: `LAST_ONE_DO_NOT_USE == 54` (`def.h`). Abstract holders (`Arrow`, `SinglePoint`, …) are not user tools.

### 5.4 Output — render, layout, export (Pillar 3)

| ID | Feature | Location | Deep dive |
|----|---------|----------|-----------|
| R1 | Draft scene + view (zoom, pan, rubber-band) | `VMainGraphicsScene`, `VMainGraphicsView` | [Module_ExportRender.md](./Module_ExportRender.md) |
| R2 | Piece scene | `pieceScene` | [Module_ExportRender.md](./Module_ExportRender.md) |
| R3 | Layout page scenes | `MainWindowsNoGUI` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| R4 | Scene items: points, curves, handles, text, grainline | `vwidgets` | [Module_ExportRender.md](./Module_ExportRender.md) |
| L1 | Nesting generator | `VLayoutGenerator` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L2 | Piece bank grouping (3-group / 2-group / descending) | `VBank` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L3 | Edge-combine + rotation search (thread pool) | `VPosition`, `VBestSquare` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L4 | Layout gap (allowance offset) | `SetLayoutAllowancePoints` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L5 | Forbid flipping / mirror compensation | `VLayoutPaper` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L6 | Strip optimization / unite pages | `gatherPages` / `unitePages` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L7 | Layout settings dialog | `LayoutSettingsDialog` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L8 | Paper templates (A0–A4, US, ANSI, rolls, custom) | `PaperSizeFormat`, `PageFormatCombobox` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| L9 | Layout progress / abort | `DialogLayoutProgress` | [Module_LayoutNesting.md](./Module_LayoutNesting.md) |
| E1 | SVG export | `SvgGenerator` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E2 | PDF export | `QPrinter` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E3 | Tiled PDF / poster print | `VPoster`, `PDFTiled` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E4 | Raster PNG / JPG / BMP / PPM / TIF | `QImage` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E5 | Wavefront OBJ | `VObjEngine` (Delaunay) | [Module_ExportRender.md](./Module_ExportRender.md) |
| E6 | PS / EPS via external `pdftops` | `convertPdfToPs` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E7 | DXF Flat R10–R2013 (optional binary) | `VDxfEngine` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E8 | DXF AAMA R10–R2013 (apparel layers) | `ExportToAAMA` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E9 | DXF ASTM (enum only; not in UI) | `LayoutExportFormat` 28–36 | [Module_ExportRender.md](./Module_ExportRender.md) |
| E10 | Export layout dialog + format combobox | `ExportLayoutDialog` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E11 | Text-as-paths option | export flags | [Module_ExportRender.md](./Module_ExportRender.md) |
| E12 | Print / print preview (original + tiled) | `MainWindowsNoGUI` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E13 | CLI export (`--basename` and related) | `VCommandLine` | [Module_ExportRender.md](./Module_ExportRender.md) |
| E14 | Export-only-details (skip nesting) | CLI / export path | [Module_ExportRender.md](./Module_ExportRender.md) |
| E15 | Export progress dialog | `ExportProgressDialog` | [Module_ExportRender.md](./Module_ExportRender.md) |

### 5.5 Persistence, i18n, quality, packaging

| ID | Feature | Location | Deep dive |
|----|---------|----------|-----------|
| IO1 | Pattern `.sm2d` / legacy `.val` | schema **v0.7.4**, min 0.1.0 | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| IO2 | Pattern converter 0.1.0 → 0.7.4 | `VPatternConverter` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| IO3 | Xerces-C schema validation | `VDomDocument::ValidateXML` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| IO4 | Label-template XML | `VLabelTemplateConverter` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| IO5 | Images stored as bytearray in pattern | developer README + `DraftImage` | [Module_FileIOParsing.md](./Module_FileIOParsing.md) |
| N1 | UI translations (Weblate) | `share/translations` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| N2 | Measurement-name translations | `measurements_*.ts` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| N3 | Locales (en, ru, uk, de, fr, es, …) | `SupportedLocales` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| Q1 | Unit tests `Seamly2DTest` | `src/test/Seamly2DTest` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| Q2 | Collection / regression CLI tests | `CollectionTest` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| Q3 | Parser harness | `ParserTest` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| Q4 | Translation integrity tests | `TranslationsTest` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B1 | qmake SUBDIRS build | `Seamly2D.pro`, `src.pro`, `libs.pro` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B2 | GitHub Actions CI | `.github/workflows/ci.yml` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B3 | Windows NSIS + zip | `dist/` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B4 | macOS signed/notarized DMG | `out/`, `CONFIG+=macSign` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B5 | Linux AppImage | CI linuxdeploy | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B6 | Linux Flatpak (Flathub, external recipe) | `io.seamly.seamly2d` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |
| B7 | Rolling version `YYYY.M.D.Hmm` | CI / `scripts/version.sh` | [Module_BuildTestCI.md](./Module_BuildTestCI.md) |

**Matrix counts:** 22 application, 22 input, 70 instruction (geometry/DAG/formula/tools/pieces/undo/groups), 24 output, 16 persistence/i18n/quality/build — **154 tracked features**, all mapped to code.

### 5.6 Known gaps (documented so the matrix is honest)

| Item | Reality in tree |
|------|-----------------|
| ASTM DXF | Enum present; UI export list commented / not offered |
| Native 3D body-scan import | Messaging only (email conversion), no parser |
| Geometric constraint solver | Intentionally absent; history replay instead |
| Debian MIME `*.smim` | Stale vs application `*.smis` |
| Embedded measurements in `.sm2d` | Path only; body data is a separate file |

---

## 6. File Format Map

| Extension | Kind | Current schema | Notes |
|-----------|------|----------------|-------|
| `.sm2d` | Pattern | pattern **v0.7.4** | Current |
| `.val` | Pattern | same family | Valentina legacy |
| `.smis` | Individual measurements | individual **v0.3.4** | Current |
| `.vit` | Individual measurements | migrates to `.smis` | Legacy |
| `.smms` | Multisize measurements | multi **v0.4.5** | Current |
| `.vst` | Multisize measurements | migrates to `.smms` | Legacy |
| Label template XML | Labels | **v1.0.0** | Not a clothing pattern |
| Export: svg, pdf, png, jpg, bmp, ppm, tif, obj, ps, eps, dxf | Output | — | See §5.4 |

---

## 7. Design Rationale (why it is built this way)

| Choice | Rationale |
|--------|-----------|
| History as the DAG | Patternmakers already think in construction order; XML serializes that order for free |
| Formulas on almost every numeric field | One block grades across a size table without redrawing |
| Separate measurement files | Same draft, many bodies; SeamlyMe can edit sizes without opening the CAD |
| LiteParse vs FullParse | Dragging a point must not rebuild the entire `QGraphicsScene` |
| Implicit sharing (`QSharedData`) | Cheap snapshots per tool; geometry reused in draft, piece, and layout |
| XSD + stepwise converters | Decade of files remain open; each version bump is an explicit migration |
| Qt Graphics View | Interactive 2D CAD with zoom/selection/undo is the toolkit’s native strength |
| qmake SUBDIRS, not CMake | Historical; CI and developer docs still assume `Seamly2D.pro` |
| No constraint solver | Clothing constructions are acyclic if users do not create cycles; a solver would be large and poorly matched to formula text |

---

## 8. Deep-dive documents

These files hold algorithms, class structures, and “why” for modules too large for this overview. All of them live beside this document in `docs/`.

| File | Scope |
|------|--------|
| [Module_Measurements.md](./Module_Measurements.md) | Anthropometry catalog, individual vs multisize math, SeamlyMe, units, PM systems |
| [Module_FileIOParsing.md](./Module_FileIOParsing.md) | XML tags, Xerces, every converter step, schemas |
| [Module_DependencyTree.md](./Module_DependencyTree.md) | History, parse modes, referens, IDs, draft blocks |
| [Module_GeometryEngine.md](./Module_GeometryEngine.md) | `VGObject` hierarchy, intersections, polar construction |
| [Module_BezierMath.md](./Module_BezierMath.md) | Cubics, paths, Hobby handles, cut-at-length |
| [Module_FormulaEngine.md](./Module_FormulaEngine.md) | qmuparser, `VFormula`, variable types, i18n tokens |
| [Module_DrawingTools.md](./Module_DrawingTools.md) | Complete `Tool` catalog, visualization, operations |
| [Module_PatternPieces.md](./Module_PatternPieces.md) | `VPiece`, SA, notches, grainline, labels, union |
| [Module_UndoCommands.md](./Module_UndoCommands.md) | Command list and XML-delta protocol |
| [Module_LayoutNesting.md](./Module_LayoutNesting.md) | Bank, paper, `VPosition`, scoring |
| [Module_ExportRender.md](./Module_ExportRender.md) | Scenes, paint devices, format matrix, CLI |
| [Module_Applications.md](./Module_Applications.md) | Both apps’ UI, dialogs, settings, groups, images |
| [Module_BuildTestCI.md](./Module_BuildTestCI.md) | qmake, tests, i18n, packaging |

---

## 9. Reading order for a new engineer

1. This file — system shape and data flow.
2. [Module_DependencyTree.md](./Module_DependencyTree.md) — how a change becomes a new canvas.
3. [Module_Measurements.md](./Module_Measurements.md) — what formulas are allowed to name.
4. [Module_DrawingTools.md](./Module_DrawingTools.md) + [Module_GeometryEngine.md](./Module_GeometryEngine.md) — how a point is born.
5. [Module_PatternPieces.md](./Module_PatternPieces.md) + [Module_LayoutNesting.md](./Module_LayoutNesting.md) — how a garment leaves the screen.

Primary code landmarks:

- Pattern parse factory: `src/app/seamly2d/xml/vpattern.cpp`
- Object store: `src/libs/vpatterndb/vcontainer.*`
- Tool base: `src/libs/vtools/tools/vabstracttool.*`
- Measurement I/O: `src/libs/vformat/measurements.*`
- Nesting: `src/libs/vlayout/vlayoutgenerator.*`
- Shared enums: `src/libs/vmisc/def.h`
