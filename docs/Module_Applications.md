# Module: Applications and UI Shell

Parent: [Architecture.md](./Architecture.md)

Two Qt Widgets apps share libraries. Seamly2D is the CAD; SeamlyMe is the measurement editor. They meet at a measurement **file path**, not a shared process heap (Seamly2D can spawn SeamlyMe).

---

## 1. Seamly2D

| Item | Location |
|------|----------|
| Entry | `src/app/seamly2d/main.cpp` |
| App | `Application2D` : `VAbstractApplication` (`core/application_2d.*`) |
| Window | `MainWindow` + `mainwindow.ui`; base `MainWindowsNoGUI` |
| Document | `xml/vpattern.*` |

Boot: Qt resources → `Application2D` → optional `SeamlyWelcomeDialog` → `MainWindow` → `VCommandLine`.

`qApp` is `#define`d to `Application2D*` (`application_2d.h`).

### Modes

`showDraftMode` / `showPieceMode` / `showLayoutMode`. One `VMainGraphicsView` (`ui->view`) is re-targeted at `draftScene`, `pieceScene`, or a layout page.

### Toolbars (`mainwindow.ui`)

`file_`, `mode_`, `draft_`, `edit_`, `zoom_`, `tools_ToolBox_`, `points_`, `lines_`, `curves_`, `arcs_`, `operations_`, `pieces_`, `details_`, `layout_`, `pointName_`, `view_`, plus runtime **PenToolBar** (default line color/type/weight).

### Docks

| Dock | Content |
|------|---------|
| Tool properties | `VPropertyExplorer` / `VToolOptionsPropertyBrowser` |
| Toolbox | Tool buttons |
| Groups | `GroupsWidget` |
| Pieces | `PiecesWidget` |
| Layout pages | page list for nest |

### App dialogs (`app/seamly2d/dialogs/`)

1. `About2DDialog`
2. `AbstractLayoutDialog` (base)
3. `CalculatorDialog`
4. `DecimalChartDialog`
5. `DialogDateTimeFormats`
6. `DialogLayoutProgress`
7. `DialogNewPattern`
8. `DialogPatternProperties`
9. `DialogPreferences` + Configuration / Pattern / Paths / Graphics View pages
10. `DialogVariables`
11. `ExportLayoutDialog`
12. `ExportProgressDialog`
13. `HistoryDialog`
14. `LayoutSettingsDialog`
15. `ShortcutsDialog`
16. `ShowInfoDialog`
17. `SeamlyWelcomeDialog`

### Shared / lib dialogs used here

`ImageDialog` (`libs/tools`), `DialogExportToCSV` (`vmisc`), formula/label/undo dialogs (`vtools`), every tool dialog ([Module_DrawingTools.md](./Module_DrawingTools.md)).

---

## 2. SeamlyMe

| Item | Location |
|------|----------|
| Entry | `src/app/seamlyme/main.cpp` |
| App | `ApplicationME` — multi-window + local socket CLI |
| Window | `TMainWindow` |
| Lite pattern | `vlitepattern.*` |

Dialogs:

1. `NewMeasurementsDialog`
2. `MeasurementDatabaseDialog` (`database_dialog`)
3. `DialogSeamlyMePreferences` + configuration / path pages
4. `MeWelcomeDialog`
5. `MeShortcutsDialog`
6. `DialogAboutSeamlyMe`

Feature list: [Module_Measurements.md](./Module_Measurements.md) §10.

Seamly2D **Ctrl+M** launches SeamlyMe on the bound file so the user edits sizes without a second measurement parser in the CAD.

---

## 3. Groups

Named collections of draft objects: visibility, lock, color, linetype, lineweight (`GroupAttributes`). XML on `VAbstractPattern`. UI: `GroupsWidget` + `dialoggroup` / `editgroup_dialog` / `addtogroup_dialog`. Undo: [Module_UndoCommands.md](./Module_UndoCommands.md).

Groups are **display filters**, not calculation scopes.

---

## 4. Background images

`src/libs/tools` (not `vtools`):

- `ImageTool` — import into pattern XML + scene
- `ImageItem` — graphics item
- `ImageDialog` — position, scale, opacity, lock, z-order

`DraftImage` struct in `def.h`. Pixel bytes stored in the pattern ([Module_FileIOParsing.md](./Module_FileIOParsing.md)). Used as a tracing reference in Draft.

---

## 5. History and variables UI

- `HistoryDialog` — table of `VToolRecord`s; search; jump/highlight on canvas.
- `DialogVariables` — custom variables; Fx editor; rename rewrites formulas.

---

## 6. Preferences and appearance

`VCommonSettings` / `VSettings` (2D) / `VSeamlyMeSettings`.

`AppTheme`: LightFusion, DarkFusion, TwilightFusion, System, classic, Windows11 (`Application2D::setTheme`).

Graphics view page: zoom, origin, antialiasing-related options. Pattern page: default piece SA, grainline, etc. Path page: template / image / body-scan directories.

---

## 7. Property explorer

`src/libs/vpropertyexplorer` — Qt-style grid (`VProperty`, `VPropertyModel`, `VPropertyTreeView`, factories). Plugins: bool, color, enum, file, number, string, shortcut, point, vector3d, line type/color/weight, plaintext, direction, object, label, widget.

Seamly2D adds `VFormula` property editors in `app/seamly2d/core/`. This is how tool options edit without opening the full dialog.

---

## 8. Calculator and decimal chart

In-app calculator (`vwidgets/calculator` + `CalculatorDialog`) uses the same parser family. `DecimalChartDialog` is a reference table for patternmakers (not part of the DAG).

---

## 9. Auto-update

`src/libs/fervor` — `FvUpdater` polls `https://api.github.com/repos/FashionFreedom/Seamly2D/releases/latest`. MIT-licensed Fervor derivative. Optional silent check.

---

## 10. i18n in the UI

`VAbstractApplication` installs translators for `seamly2d_*.qm`, `measurements_*.qm`, `qtbase_*.qm`. Welcome dialog picks locale on first run. Measurement names in formulas follow `VTranslateVars` ([Module_FormulaEngine.md](./Module_FormulaEngine.md)).

---

## 11. Headless mode

`MainWindowsNoGUI` exists so CI and `--basename` export do not require a user at the canvas. CollectionTest drives this path.

Related: [Module_ExportRender.md](./Module_ExportRender.md), [Module_BuildTestCI.md](./Module_BuildTestCI.md).
