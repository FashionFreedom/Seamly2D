# Module: Formula Engine

Parent: [Architecture.md](./Architecture.md)

Almost every numeric field in Seamly2D is a **formula string**, not a literal. Formulas bind measurements, custom variables, and geometry-derived quantities. The evaluator is a vendored **muParser** fork (`src/libs/qmuparser`) wrapped by `Calculator` and `VFormula`.

---

## 1. Why formulas everywhere

A block must **grade**. If the bust dart is `bust/12` rather than `4.5`, changing the measurement file or the multisize size/height recomputes the whole DAG ([Module_DependencyTree.md](./Module_DependencyTree.md)) without rewriting tools.

The cost: parsing, localization (decimal separators, translated measurement names), and a strict evaluation order.

---

## 2. Pipeline

```
User types localized text in EditFormulaDialog / property editor
        │
        ▼
VTranslateVars::FormulaFromUser     // names → internal English tokens
        │                              // locale decimal → parser decimal
        ▼
Stored in XML attribute (internal form)
        │
        ▼
VAbstractTool::CheckFormula / VFormula::Eval / MeasurementDoc::EvalFormula
        │
        ▼
Calculator::EvalFormula(VContainer::variables, formula)
        │
        ├─ SetVarFactory(unknown → 0)   // first pass
        ├─ Eval()
        ├─ GetTokens(); strip unary "-" and built-in function names
        └─ InitVariables: bind only tokens present in the hash; Eval again
        │
        ▼
qreal in pattern units
```

`FormulaToUser` is the inverse for display.

**Why two-pass eval:** muParser must know variables before a full eval. Unknown names become 0 via the factory so tokenization succeeds; then only **declared** variables are bound. A typo that is not a known name evaluates as 0 — a known UX hazard, mitigated by the formula dialog’s variable list and `CheckFormula` error dialogs.

`setAllowSubexpressions(false)`: one expression per field (no `;` chains). Comment in `calculator.h`: do not keep a formula dialog’s parser alive while creating another `Calculator` — separator state can leak.

---

## 3. Class structure

```
qmu::QmuParser (qmuparser)
 └── qmu::QmuFormulaBase
      ├── Calculator              // vpatterndb/calculator.*
      └── QmuTokenParser          // tokenize for translation / rename

VFormula                          // UI wrapper: Eval, error flag, postfix unit, check-zero
VTranslateVars                    // measurement + function + PM-system translation
VTranslateMeasurements            // catalog strings for the 246 names
VContainer::variables             // QHash<QString, QSharedPointer<VInternalVariable>>
```

`VFormula` is a `Q_DECLARE_METATYPE` so the property browser can edit it (`vformulaproperty` in the app `core/`).

---

## 4. Variable type system

`VarType` on `VInternalVariable`:

| Type | Class | Typical name |
|------|-------|----------------|
| `Measurement` | `MeasurementVariable` | `height_M`, `bust_…`, `@MyEase` |
| `Variable` | `CustomVariable` | user increments in the Variables dialog |
| `LineLength` | `VLengthLine` | `Line_A_B` |
| `LineAngle` | `VLineAngle` | `AngleLine_A_B` |
| `CurveLength` | `VCurveLength` | curve name or `name_segN` |
| `CurveCLength` | `VCurveCLength` | `c1Length_*`, `c2Length_*` |
| `CurveAngle` | `VCurveAngle` | `angle1_*`, `angle2_*` |
| `ArcRadius` | `VArcRadius` | `RadiusArc_*`, `Radius1Arc_*` |

Prefixes: `ifc/ifcdef.h` (`Line_`, `AngleLine_`, `RadiusArc_`, …).

`VContainer::AddLine` / `AddArc` / `AddSpline` create derived variables as side effects of construction. Lite parse **clears** those derived types and rebuilds them so stale lengths cannot survive a move.

Measurements are **not** cleared on lite parse.

---

## 5. Custom variables

XML `<variables>` list, edited in `DialogVariables`. Each has name, formula, description. Evaluated in **list order** (`VPattern::parseVariablesElement` / `LiteParseVariables`). Later variables may reference earlier ones and measurements.

CRUD on the DOM: `addEmptyCustomVariable`, `setVariableFormula`, `replaceNameInFormula` (rewrites tokens in all `VFormulaField`s when renaming).

---

## 6. Localization

`VTranslateVars`:

- Measurement internal token ↔ translated UI name
- Function names per locale
- Postfix operators `cm`, `mm`, `in`
- Patternmaking system titles

`QmuTokenParser` extracts identifiers so rename/translate can rewrite formulas without regex accidents (same bias-token idea as converters).

TranslationsTest asserts builtin regexp and qmuParser error strings stay translatable.

---

## 7. `CheckFormula` and zero checks

`VAbstractTool::CheckFormula(toolId, formula, data)`:

- Runs `Calculator`
- On failure, can show an undo-oriented error (`DialogUndo`) so a bad redo does not stick
- `VFormula::setCheckZero` — some tools reject 0 (e.g. radius)

`EditFormulaDialog` (`vtools/dialogs/support`) is shared by tools, variables, and SeamlyMe (`DialogSource` bit flags hide tabs).

---

## 8. Units inside formulas

Pattern unit is implicit (all `qreal`s in the container are already converted). Postfix operators convert a literal: `2.54in` etc. Mixing without postfix uses pattern units.

Multisize: the measurement **value** already includes grading (`CalcValue`) before the formula sees it. Formulas do not contain `k_size`; they contain `bust`.

---

## 9. Why not Lua / Python / a full CAS

- Must run in a tight lite-parse loop over hundreds of tools
- Must be serializable as short XML attributes
- Must be translatable for non-programmer patternmakers
- muParser is small, BSD-licensed, and already battle-tested in Valentina

The fork lives in-tree (`qmuparser`) so decimal-separator and token-reader behavior stay under project control (`qmudef.h`, `qmutokenparser`).

Related: [Module_Measurements.md](./Module_Measurements.md), [Module_DependencyTree.md](./Module_DependencyTree.md).
