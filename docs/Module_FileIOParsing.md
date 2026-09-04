# Module: File I/O, XML Parsing, and Schema Conversion

Parent: [Architecture.md](./Architecture.md)

All native documents are **XML**. Qt `QDomDocument` is the working tree. **Xerces-C** validates against versioned XSD resources. A **converter pipeline** walks files from their stored version to the current max, validating after each hop.

---

## 1. Why XML + XSD + converters

A pattern is a construction history, not a mesh. XML gives:

- Human-inspectable files (forum support)
- Ordered children = topological order ([Module_DependencyTree.md](./Module_DependencyTree.md))
- Attribute-level undo deltas ([Module_UndoCommands.md](./Module_UndoCommands.md))

XSD is the contract. Converters exist because the product has shipped files since Valentina 0.1.0; dropping old files would strand user archives.

Pattern: **Template Method** on `VAbstractConverter` (`Convert` → `applyPatches` / `downgradeToCurrentMaxVersion`). Each document type fills in `minVer`/`maxVer`/`getSchema`/`applyPatches`.

---

## 2. Class structure

```
VDomDocument                    // ifc/xml/vdomdocument.*
 ├── VAbstractConverter         // Convert(), backup, temp file
 │    ├── VPatternConverter           0.1.0 → 0.7.4
 │    ├── IndividualSizeConverter     0.2.0 → 0.3.4
 │    ├── MultiSizeConverter          0.3.0 → 0.4.5
 │    └── VLabelTemplateConverter     1.0.0 → 1.0.0
 └── VAbstractPattern : QObject + VDomDocument
      ├── VPattern          // Seamly2D
      └── VLitePattern      // SeamlyMe (read tokens only)

MeasurementDoc : VDomDocument   // vformat — not a converter; converters run first
VLabelTemplate                  // vformat
```

Exceptions (`ifc/exception/`): `VException`, `VExceptionBadId`, `VExceptionWrongId`, `VExceptionConversionError`, `VExceptionEmptyParameter`, `VExceptionObjectError`, `VExceptionUndo`.

Tag/attribute string constants: `ifc/ifcdef.h` (single source so converters and tools do not drift).

---

## 3. Open pipeline

```
User picks file
    │
    ▼
V*Converter(fileName)          // loads DOM, reads <version>
    │
    ├─ ver == max  → return original path
    ├─ ver <  max  → save .bak (if not read-only), applyPatches()
    └─ ver >  max  → downgradeToCurrentMaxVersion()  (rare; writes current max)
    │
    ▼
Each hop: toVersionX_Y_Z() then VDomDocument::ValidateXML(schema, tmpFile)
    │
    ▼
App loads converted temp (or original) into VPattern / MeasurementDoc
    │
    ▼
VPattern::Parse(FullParse) or MeasurementDoc::readMeasurements()
```

`VAbstractConverter::Convert()` uses `QTemporaryFile` so the original is untouched until the app saves. `CONVERTER_VERSION_CHECK(major, minor, patch)` packs versions as `(major<<16)|(minor<<8)|patch` (e.g. 0.7.4 = `0x000704`).

---

## 4. Xerces validation

`VDomDocument::ValidateXML`:

1. `XMLPlatformUtils::Initialize`
2. Load XSD from Qt resource (`://schema/...`) into `MemBufInputSource`
3. `XercesDOMParser` with full schema checking
4. Parse the instance (Windows: via memory buffer to avoid temp-path issues)

Schemas are compiled into `src/libs/ifc/schema.qrc`.

If validation fails, conversion throws; the UI shows the exception text. CLI export exits with `V_EX_NOINPUT`.

---

## 5. Pattern document (`.sm2d` / `.val`)

| | |
|--|--|
| Min | 0.1.0 |
| Max / current | **0.7.4** |
| Schema | `://schema/pattern/v0.7.4.xsd` |
| Class | `VPatternConverter` + `VPattern` |

### 5.1 Converter hop table

`applyPatches()` is a fall-through `switch (m_ver)` from `0x000100` through `0x000703`. Each case calls `toVersionX_Y_Z()` then re-validates.

| To version | Typical change (from `Q_STATIC_ASSERT` / `toVersion*` names) |
|------------|--------------------------------------------------------------|
| 0.1.1–0.1.4 | Early tag/id cleanup |
| 0.2.0 | Major structural rewrite of tools |
| 0.2.1–0.2.7 | Spline/path representation (old spline parsers still gated by min-ver asserts) |
| 0.3.x | Piece/modeling evolution |
| 0.4.x | Groups, operations, further tool attrs |
| 0.5.x | Intermediate schema |
| 0.6.0–0.6.9 | Images, line weights, piece lock/color, more piece data |
| 0.7.0–0.7.4 | Current family; 0.7.4 is a version bump to the latest XSD |

Old spline XML is still parsed by `ParseOldToolSpline` / `ParseOldToolSplinePath` until `PatternMinVer` reaches 0.2.7 (static asserts in `vpattern.h`).

### 5.2 Root tags `VPattern::Parse` walks

`TagDraftBlock`, `TagVariables`, `TagDescription`, `TagNotes`, `TagMeasurements` (path only), `TagVersion`, `TagGradation`, `TagImage`, `TagUnit`, `TagPatternName`, `TagPatternNum`, `TagCompanyName`, `TagCustomerName`, `TagPatternLabel`.

Draft-block children (calculation stage): `point`, `line`, `spline`, `arc`, `tools`, `operation`, `elArc`, `path`. Dispatch is `attribute type=` → `ParseTool*`.

`<measurements>` is a **string path**, not nested body data. See [Module_Measurements.md](./Module_Measurements.md).

### 5.3 Images

`DraftImage` (`def.h`) stores id, filename, lock, origin, position, size, aspect lock, units, rotation, visibility, opacity, z-order. Pixel data is persisted as a **bytearray** in the pattern (png/jpg/bmp). SVG icons are preferred in the app chrome; user images stay raster to avoid missing fonts.

---

## 6. Measurement documents

See [Module_Measurements.md](./Module_Measurements.md) for the data model. Converter classes:

### Individual (`IndividualSizeConverter`)

| Step | Effect |
|------|--------|
| → 0.3.0 | New tags; `OldNamesToNewNames_InV0_3_0` |
| → 0.3.1 | Gender string conversion |
| → 0.3.2 | `pm_system` tag |
| → 0.3.3 | `OldNamesToNewNames_InV0_3_3` |
| → 0.3.4 | Root `vit` → `smis` |

### Multisize (`MultiSizeConverter`)

| Step | Effect |
|------|--------|
| → 0.4.0 | Convert attrs to `base` / `size_increase` / `height_increase` |
| → 0.4.1 | `pm_system` |
| → 0.4.2 | Name renames |
| → 0.4.3, 0.4.4 | Schema-only bumps |
| → 0.4.5 | Root `vst` → `smms` |

Rename tables live on `AbstractMConverter` so both converters share anthropometric identity across versions.

Formula token rename uses `Replace` + `CorrectionsPositions` / `BiasTokens` so substituting a longer name does not invalidate later token offsets.

---

## 7. Label templates

`VLabelTemplate` + `VLabelTemplateConverter`. Schema `label_template/v1.0.0.xsd` (min = max = 1.0.0). Used by piece/pattern info labels; edited via `EditLabelTemplateDialog`.

---

## 8. Schema inventory (63 XSD files)

Under `src/libs/ifc/schema/`:

| Family | Versions present |
|--------|------------------|
| `pattern/` | 0.1.0–0.1.4, 0.2.0–0.2.7, 0.3.0–0.3.9, 0.4.0–0.4.8, 0.5.0–0.5.1, 0.6.0–0.6.9, 0.7.0–**0.7.4** |
| `individual_size_measurements/` | 0.2.0, 0.3.0–**0.3.4** |
| `multi_size_measurements/` | 0.3.0, 0.4.0–**0.4.5** |
| `label_template/` | **1.0.0** |

Every hop in `applyPatches` must have a matching `getSchema(ver)` entry; static asserts in the converter `.cpp` fail the build if min/max and hop functions disagree.

---

## 9. Save path

`VPattern::SaveDocument` / `MeasurementDoc::SaveDocument` write the in-memory DOM (already at current max version). Read-only files refuse mutation. Converters write `.bak` next to the original before patching a writable file.

---

## 10. Why not a single “current-only” format

1. Forum users still attach 0.4.x / Valentina `.val` files.
2. Each hop is small and testable (`CollectionTest` includes legacy samples).
3. XSD per version documents what a given release could write.

Cost: `vpatternconverter.cpp` is large. The `Q_STATIC_ASSERT_X(PatternMinVer < …)` comments mark dead conversion steps that can be deleted only after the project **raises the minimum** supported version — a conscious compatibility policy, not forgotten code.

Related: [Module_DependencyTree.md](./Module_DependencyTree.md) (what the XML order means), [Module_UndoCommands.md](./Module_UndoCommands.md) (DOM as undo store).
