# Module: Dependency Tree (History DAG)

Parent: [Architecture.md](./Architecture.md)

Seamly2D does **not** use a constraint solver or a graph library. Dependencies are an **implicit DAG** encoded as (1) XML sibling order, (2) `m_history`, (3) parent IDs, (4) formula tokens, (5) reference counts. Recalculation is **replay**.

---

## 1. Why history instead of constraints

Garment blocks are built the way a textbook is written: “from A, square out 2 cm, intersect the bust line…”. That sequence is acyclic if the user does not create a cycle. A constraint engine (SolidWorks-style) would be:

- Large and poorly matched to **text formulas**
- Hard to serialize into the existing XML
- Weaker at explaining *which construction step* failed

Replay is simple: if tool *N*’s formula fails, the error is on tool *N*. Undo is a DOM edit plus replay ([Module_UndoCommands.md](./Module_UndoCommands.md)).

The cost: you cannot say “keep these two points 10 cm apart” independently of how they were constructed. You edit the tool that *created* the distance.

---

## 2. The five encodings of the same DAG

```
XML children under <calculation>     ≡  topological order
m_history : QVector<VToolRecord>     ≡  {id, Tool, draftBlockName}
parent IDs on each tool              ≡  explicit edges
formula tokens (Line_A_B, bust, …)   ≡  data edges (must already exist)
VDataTool::_referens                 ≡  delete-safety reverse edges
```

`VToolRecord` lives in `ifc/xml/vtoolrecord.*`. `VAbstractPattern` owns `m_history` and a **cursor** (`getCursorId` / `setCursorId`) so a new tool can be inserted *earlier* than “end of history” (patternmakers often need a missed construction step in the middle).

`VAbstractTool::AddRecord` appends or inserts after the cursor.

---

## 3. Class structure

```
VAbstractPattern                    // history, draft blocks, groups, MPath
 └── VPattern                       // parse factory, scenes, VContainer*

VDataTool : QObject                 // owns VContainer data snapshot + _referens
 └── VAbstractTool                  // doc, id, CheckFormula, AddRecord, vis
      └── VInteractiveTool          // DialogTool
           ├── VDrawTool            // QGraphicsItem on draft scene
           ├── PatternPieceTool
           └── VAbstractNode        // modeling copies
      UnionTool (also under VAbstractTool)
```

Each tool holds its own `VContainer data` (Qt implicit sharing / copy-on-write). On create:

```
doc->UpdateToolData(id, data)
```

Tool *N*’s snapshot is the world **after** tools 1…N. Later tools that copy the container see prior geometry and variables. This is why formulas cannot name a point that has not been created yet — it is not in that snapshot.

---

## 4. Identifiers and names

| Mechanism | Role |
|-----------|------|
| `VContainer::_id` | Monotonic `quint32` allocator (`getNextId`, `UpdateId`) |
| XML `id` attribute | Persistence |
| `NULL_ID` = 0 | Sentinel |
| `uniqueNames` | Point names + variable names must be unique |
| `VPattern::GenerateLabel` | Alphabet sequence for new points |
| `GenerateSuffix` | Operation copies (`_M`, rotated names, etc.) |

`PrepareForParse(FullParse)` calls `TestUniqueId()` before wiping state.

---

## 5. Parse modes

`enum class Document { LiteParse, LiteBlockParse, FullParse }` (`vabstractpattern.h`).

| Mode | `PrepareForParse` | Scene | Tools map | History |
|------|-------------------|-------|-----------|---------|
| **FullParse** | Clear scenes, origins, `ClearForFullParse`, tools, history, images | Rebuilt | Recreated | Recreated |
| **LiteParse** | `ClearUniqueNames`; clear derived vars (Variable, LineAngle, LineLength, CurveLength, CurveCLength, ArcRadius, CurveAngle) | **Kept** | Kept; `Create(..., LiteParse)` updates GObjects | Kept |
| **LiteBlockParse** | Same as lite, current draft block only (`parseCurrentDraftBlock`) | Kept | Kept | Kept |

`VPattern::Parse` walks root tags, then each draft block’s calculation/modeling stages. `LiteParseTree` is the slot undo commands emit (`NeedLiteParsing`).

**Why two speeds:** dragging a point or editing one formula must not destroy every `QGraphicsItem` (selection, zoom, undo visual continuity). Full parse is for open-file, draft-block switch, and broken-state recovery.

Measurements are **not** cleared on lite parse — they are inputs. Custom variables *are* cleared and rebuilt from XML (`parseVariablesElement`) so formula order is respected.

---

## 6. Parse dispatch (factory)

`VPattern::ParseDraftStage` walks children in **document order**:

| Tag | Handler |
|-----|---------|
| `point` | `ParsePointElement` → `ParseToolBasePoint`, `EndLine`, `AlongLine`, intersections, cuts, nodes, … |
| `line` | `ParseLineElement` |
| `spline` | `ParseSplineElement` (incl. old spline if min-ver still < 0.2.7) |
| `arc` | `ParseArcElement` |
| `elArc` | `ParseEllipticalArcElement` |
| `tools` | `ParseToolsElement` (union, piece, …) |
| `operation` | `ParseOperationElement` (move/rotate/mirror) |
| `path` | `ParsePathElement` (internal paths) |

Each `ParseTool*` reads attributes, then `VToolX::Create(scene, …, parse)`. `Create` is the **template method**: FullParse constructs the graphics object and registers it; LiteParse updates geometry in place.

`Q_STATIC_ASSERT_X(Tool::LAST_ONE_DO_NOT_USE == 54)` in `ActiveDrawBoundingRect` fails the build if a new enum value is added without handling.

---

## 7. Draft blocks

A pattern may contain multiple named **draft blocks** (pattern pieces at the construction level — not the same as `VPiece`).

- `changeActiveDraftBlock` / `getActiveDraftBlockName`
- `appendDraftBlock` / `renameDraftBlock` / delete (undo commands)
- Each block has its own calculation history subset (`VToolRecord::draftBlockName`)
- `setCurrentData()` restores the container snapshot of the **last object in the active block** after a full parse (needed when more than one block exists)

`LiteBlockParse` exists so editing inside one block does not replay every other block.

---

## 8. Reference counting

`VDataTool::_referens`: dependents call `VPattern::IncrementReferens(id)` / `DecrementReferens`. Delete is refused while `referens() > 0`. Piece nodes, operation copies, and formula-using tools all increment parents.

This is the **reverse** of the DAG: history order is forward; referens is “who points at me”.

---

## 9. Recalculation lifecycle (typical edit)

```
User edits EndLine length formula in property browser
    → SaveToolOptions undo command writes XML attribute
    → emit NeedLiteParsing(LiteParse)
    → VPattern::LiteParseTree
         PrepareForParse: drop derived variables
         parseVariablesElement
         for each draft block: ParseDraftStage(LiteParse)
              each tool Create(..., LiteParse)
                   CheckFormula → Calculator
                   UpdateGObject
                   UpdateToolData
    → QGraphicsItems refresh from new VPointF / curves
    → emit patternParsed
```

Failure during redo: `VExceptionUndo` → `UndoEvent` posted to `VPattern::customEvent` → auto-undo. Object/id/conversion errors disable the GUI (`setGuiEnabled(false)`) so the user cannot compound a corrupt tree; CLI exits.

---

## 10. Formula edges vs ID edges

Two kinds of dependency:

1. **Structural:** `basePoint="12"` — the tool cannot exist without that point. Encoded in XML attributes; referens tracks them.
2. **Algebraic:** formula `bust/2 + Line_A_B` — tokens must already be in `VContainer::variables`. Encoded only in the formula string. History order is the only guarantee.

There is no static analysis that draws a graph of formula tokens. `ListExpressions()` / `ListMeasurements()` scan XML for support (required-measurement check, rename-variable).

---

## 11. Groups are not DAG nodes

`Tool::Group` is XML metadata (name, visible, locked, color, linetype, lineweight) plus item membership. Groups do not participate in calculation order. They only affect **display** and selection. See [Module_Applications.md](./Module_Applications.md).

`Tool::BackgroundImage` is similarly non-calculating (z-order decoration).

---

## 12. What “cycle” looks like

The engine does not detect cycles. If the user writes a custom variable `a = b + 1` and `b = a`, evaluation uses qmuparser’s unknown-var=0 factory then binds known tokens ([Module_FormulaEngine.md](./Module_FormulaEngine.md)) — results will be wrong or zero, not a graph error. Construction cycles via IDs are hard to create because a tool can only reference **already parsed** ids.

Related: [Module_DrawingTools.md](./Module_DrawingTools.md), [Module_FormulaEngine.md](./Module_FormulaEngine.md), [Module_UndoCommands.md](./Module_UndoCommands.md).
