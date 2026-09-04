# Module: Measurements and Size Tables (Input)

Parent: [Architecture.md](./Architecture.md)

This module is **Pillar 1**. Body dimensions never live inside a `.sm2d` pattern. The pattern stores a **path** to a measurement file. SeamlyMe edits that file; Seamly2D binds it, converts units, and injects named variables into `VContainer` so formulas can see them.

---

## 1. Why this design

Clothing CAD has two independent axes of change:

| Axis | Changes | Must not force |
|------|---------|----------------|
| Construction | tools, formulas, pieces | re-measuring a person |
| Body | one person vs a size chart | redrawing the block |

Separating files (`MeasurementDoc` vs `VPattern`) makes one draft reusable across customers and graded sizes. Patternmaking-system codes (`p0`…`p54`, `p998`) are **metadata** (which textbook the names align with). They do not switch a calculation engine.

---

## 2. File formats

| Ext | Root tag | Kind | Current XSD |
|-----|----------|------|-------------|
| `.smis` | `<smis>` | Individual | `schema/individual_size_measurements/v0.3.4.xsd` |
| `.smms` | `<smms>` | Multisize (size table) | `schema/multi_size_measurements/v0.4.5.xsd` |
| `.vit` | `<vit>` | Legacy individual | Converter → `.smis` at v0.3.4 |
| `.vst` | `<vst>` | Legacy multisize | Converter → `.smms` at v0.4.5 |

Constants: `smisExt`, `smmsExt`, `vitExt`, `vstExt` in `src/libs/vmisc/def.cpp`.

Type detection (`MeasurementDoc::ReadType`): root `vst`/`smms` → Multisize; `vit`/`smis` → Individual; else Unknown.

There is **no** native body-scan format. SeamlyMe’s “body scanner” path is an info dialog pointing at 3DLook email conversion (`convert@seamly.io`).

---

## 3. Class structure

```
VDomDocument
 └── MeasurementDoc          // src/libs/vformat/measurements.*
        │  binds VContainer*
        ▼
 VInternalVariable
  └── VVariable
       └── MeasurementVariable   // src/libs/vpatterndb/variables/measurement_variable.*
              └── MeasurementVariableData  (QSharedData)
```

| Class | Role |
|-------|------|
| `MeasurementDoc` | Parse/write XML; CRUD on `<m>` nodes; personal/PM-system tags |
| `MeasurementVariable` | Runtime value. Individual: stored number or formula. Multisize: `CalcValue()` grading |
| `IndividualSizeConverter` | 0.2.0 → 0.3.4 |
| `MultiSizeConverter` | 0.3.0 → 0.4.5 |
| `AbstractMConverter` | Shared measurement-name rename maps |
| `VLitePattern` | SeamlyMe: open a pattern only to list measurement tokens it requires |
| `VTranslateMeasurements` | Localized names, numbers (A01…), descriptions, diagrams |

Design patterns: **implicit sharing** on the variable; **document object** for XML; **strategy** via two constructors (individual vs multisize) on the same class.

---

## 4. Data model

### 4.1 File-level

| Field | XML | Both types |
|-------|-----|------------|
| Format version | `<version>` | yes |
| Read-only | `<read-only>` | yes |
| Notes | `<notes>` | yes |
| Unit | `<unit>` = `mm` \| `cm` \| `inch` | yes |
| Patternmaking system | `<pm_system>` numeric / UI `p0`…`p998` | yes |

Individual also has `<personal>`: family-name, given-name, birth-date, gender (`male`/`female`/`unknown`), email.

### 4.2 Per-row `<m>`

| Attribute | Individual | Multisize |
|-----------|------------|-----------|
| `name` | known name or `@custom` | known name or `@custom` |
| `value` | number **or formula string** | — |
| `base` | — | value at base size/height |
| `size_increase` | — | increment per size step |
| `height_increase` | — | increment per height step |
| `full_name` | display | display |
| `description` | notes | notes |

Custom names start with `@` (`CustomMSign` in `ifc/ifcdef.cpp`).

---

## 5. Core algorithm — multisize grading

Implemented in `MeasurementVariable::CalcValue()`:

```
sizeIncrement   = UnitConvertor(2.0, Cm, patternUnit)
heightIncrement = UnitConvertor(6.0, Cm, patternUnit)

k_size   = (currentSize   - baseSize)   / sizeIncrement
k_height = (currentHeight - baseHeight) / heightIncrement

value = base + k_size * size_increase + k_height * height_increase
```

`GetValue()` always calls `CalcValue()`. If unit/size/height pointers are unset, it returns the stored base. **Inches are forbidden** for gradation (`qWarning` + return 0).

Discrete grids (`def.h`):

- Sizes `GSizes::S22` … `S72`, step `sizeStep = 2` (cm)
- Heights `GHeights::H50` … `H200`, step `heightStep = 6` (cm)

This is linear interpolation on a regular lattice, not a full multivariate grade rule table. That is intentional: simple, formula-stable, and matches how the sample GOST/Aldrich tables are stored.

---

## 6. Individual formulas

`MeasurementDoc::EvalFormula` uses `Calculator` against a **temporary** `VContainer` filled in XML order. A later custom measurement may reference earlier ones. Order in the table therefore matters; SeamlyMe exposes Top/Up/Down/Bottom.

On load into Seamly2D, values (and multisize bases/increments) are converted **file unit → pattern unit** via `UnitConvertor`.

---

## 7. Anthropometric catalog (246 names, groups A–Q)

Defined in `src/libs/vpatterndb/measurements_def.h/.cpp`. `AllGroupNames()` concatenates `ListGroupA()` … `ListGroupQ()`.

| Group | Theme | Count |
|-------|-------|------:|
| A | Direct height | 23 |
| B | Direct width | 5 |
| C | Indentation | 3 |
| D | Hand | 5 |
| E | Foot | 4 |
| F | Head | 6 |
| G | Circumference and arc | 47 |
| H | Vertical | 42 |
| I | Horizontal | 14 |
| J | Bust | 11 |
| K | Balance | 13 |
| L | Arm | 22 |
| M | Leg | 14 |
| N | Crotch and rise | 8 |
| O | Men & tailoring | 14 |
| P | Historical & specialty | 12 |
| Q | Patternmaking (dart widths) | 3 |
| | **Total** | **246** |

Example A-group tokens: `height_M` (A01), `heightNeckBack_M` (A02), … UI diagrams are SVG resources under `src/libs/vmisc/share/resources/diagrams/`.

Validation: `eachKnownNameIsValid()` — every non-custom name must exist in the catalog. Pattern load also runs `checkRequiredMeasurements()` so every token used in the draft exists in the bound file.

---

## 8. Patternmaking systems (56 codes)

`src/libs/vpatterndb/pmsystems.h/.cpp` plus display strings in `VTranslateVars`. Codes `p0`–`p54` and `p998`.

Notable:

| Code | System |
|------|--------|
| p0 | Bunka |
| p41 | Aldrich / Men |
| p42 | Aldrich / Women |
| p51 | GOST 17917-86 |
| p998 | None (Seamly internal default) |

Samples: `src/app/share/samples/measurements/templates/aldrich_women_template.smis`, `multisize/gost_man_ru.smms`.

---

## 9. Binding into the pattern (data flow)

```
.sm2d  <measurements>relative-or-absolute-path</measurements>
        │
        ▼
MainWindow::openMeasurementFile / loadMeasurements
        │
        ▼
IndividualSizeConverter or MultiSizeConverter
        │
        ▼
MeasurementDoc::setXMLContent + readMeasurements
        │  for each <m>: MeasurementVariable → data->AddVariable(name, meash)
        ▼
VContainer::variables   (VarType::Measurement)
        │
        ▼
Calculator::EvalFormula(DataVariables(), formula)
```

Helpers: `VAbstractPattern::MPath()` / `SetMPath()`, `RelativeMPath` / `AbsoluteMPath` (`vmisc/def.cpp`). Unload clears measurement variables without wiping geometry until the next parse.

Multisize UI: user picks size/height → `VContainer::setSize/setHeight` → every `MeasurementVariable::GetValue()` changes → `LiteParse` replays tools.

Label placeholders can include measurement file name, extension, size, and height.

---

## 10. SeamlyMe feature surface

Application: `src/app/seamlyme/` — `ApplicationME` (multi-window, local socket), `TMainWindow`.

| Feature | Implementation |
|---------|----------------|
| New individual / multisize | `NewMeasurementsDialog` (type, unit, base size/height) |
| Open template / create from existing | file menu |
| Add known from database | `MeasurementDatabaseDialog` (groups A–Q, diagrams, filter) |
| Add custom `@…` | table insert |
| Reorder rows | Top / Up / Down / Bottom |
| Import names from pattern | `VLitePattern::ListMeasurements` |
| Formula editor | shared `EditFormulaDialog` |
| Gradation preview | size/height combos; live graded values |
| Display unit independent of file unit | pattern-unit combo |
| Print table | Qt print |
| CSV export | `DialogExportToCSV` |
| Preferences | configuration + path pages |
| CLI | open file; set base size/height; test mode |

---

## 11. Units

`enum class Unit { Mm, Cm, Inch, Px }`. Px is graphics-only.

- Measurement file has its own `<unit>`; pattern has its own unit.
- Load converts into pattern units.
- Formula postfix operators `cm`, `mm`, `in` (`VTranslateVars::InitPostfixOperators`).
- Multisize increments are defined in **cm** (2 / 6) then converted.

---

## 12. Converters (summary)

Full step list: [Module_FileIOParsing.md](./Module_FileIOParsing.md).

Individual: 0.2.0 → 0.3.0 (renames) → 0.3.1 (gender) → 0.3.2 (`pm_system`) → 0.3.3 (more renames) → **0.3.4** (`vit` → `smis`).

Multisize: 0.3.0 → 0.4.0 (base/increments) → 0.4.1 (`pm_system`) → 0.4.2 (renames) → 0.4.3/0.4.4 (schema) → **0.4.5** (`vst` → `smms`).

---

## 13. Why not embed measurements in the pattern

The pattern schema (`v0.7.4`) has `<measurements>` as a **string path**, not a body-measurement subtree. Embedding would freeze a customer into a draft file and make SeamlyMe a second editor of the same DOM. The split matches how a workroom keeps a size chart and a block as two documents.

Related: [Module_FormulaEngine.md](./Module_FormulaEngine.md) (how names become tokens), [Module_FileIOParsing.md](./Module_FileIOParsing.md) (XML/XSD).
