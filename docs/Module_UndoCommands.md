# Module: Undo / Redo Commands

Parent: [Architecture.md](./Architecture.md)

Every mutating edit is a Qt `QUndoCommand`. Seamly2D stores **XML deltas**, not in-memory object clones. Undo/redo patches `VAbstractPattern`’s DOM, then requests `FullParse` or `LiteParse`.

---

## 1. Why XML as the undo payload

The DOM **is** the document. If undo replayed C++ objects only, save would drift from undo state. Writing `QDomElement` snapshots means:

- Save, undo, and converter all speak the same language
- Lite parse can rebuild derived geometry from attributes
- Commands stay small (one element, not the whole container)

Pattern: **Command** + **Memento** (the memento is a `QDomElement`).

---

## 2. Base class

```
QObject + QUndoCommand
 └── VUndoCommand
      xml, doc, nodeId, redoFlag
      signals: ClearScene, NeedFullParsing, NeedLiteParsing
```

`src/libs/vtools/undocommands/vundocommand.*`

Helpers: `UndoDeleteAfterSibling`, `IncrementReferences` / `DecrementReferences` overloads for id lists, `CustomSARecord`s, and `VPieceNode`s.

`UndoCommand` enum (partial; not every command has an enum value):

`AddDraftBlock`, `AddToCalc`, `MoveSpline`, `MoveSplinePath`, `MoveSPoint`, `SaveToolOptions`, `SavePieceOptions`, `SavePiecePathOptions`, `MovePiece`, `deleteTool`, `DeleteDraftBlock`, `RenameDraftBlock`, `MoveLabel`, `MoveDoubleLabel`, `RotationMoveLabel`, `TogglePieceInLayout`, `TogglePieceLock`, `SetPieceColor`.

Group/image/piece-add commands exist as classes without extra enum entries.

---

## 3. Command catalog

### Draft / tools

| Class | Does |
|-------|------|
| `AddToCalc` | Insert tool XML into `<calculation>` (after cursor if set) |
| `DelTool` | Remove tool element; blocked conceptually by referens |
| `SaveToolOptions` | Replace tool attributes (formula edits, options) |
| `MoveSPoint` | Drag a single point |
| `MoveSpline` / `MoveSplinePath` | Drag handles / path points |

### Draft blocks

| Class | Does |
|-------|------|
| `AddDraftBlock` | New named block |
| `DeleteDraftBlock` | Remove block |
| `RenameDraftBlock` | Rename + history records |

### Pieces

| Class | Does |
|-------|------|
| `AddPiece` | Insert piece XML |
| `DeletePiece` | Remove piece |
| `SavePieceOptions` | Piece dialog apply |
| `SavePiecePathOptions` | Internal/custom path options |
| `MovePiece` | Translate piece on piece scene |
| `TogglePieceInLayout` | Nesting inclusion |
| `TogglePieceLock` | Lock flag |
| `SetPieceColor` | Display color |
| `AddDetNode` | Add modeling node |

### Groups / images / labels

| Class | Does |
|-------|------|
| `AddGroup` / `DelGroup` | Group CRUD |
| `AddGroupItem` / `RemoveGroupItem` / `MoveGroupItem` | Membership |
| `AddImage` | Background image |
| `MoveLabel` / `MoveDoubleLabel` / `MoveAbstractLabel` / `MoveOperationLabel` | Point-name labels |
| `ShowPointName` / `ShowDoublePointName` / `ShowOperationPointName` | Visibility |

---

## 4. Redo / parse protocol

Typical `redo()`:

1. Insert or patch DOM.
2. `emit NeedLiteParsing(Document::LiteParse)` or `NeedFullParsing`.
3. `VPattern::LiteParseTree` / `Parse(FullParse)` rebuilds `VContainer` + scene.

Typical `undo()`: reverse the DOM patch, emit the same parse signal.

`RedoFullParsing()` on the base class is for operations that invalidate the tool map (add/delete draft block, delete tool).

`redoFlag` distinguishes first apply (from the dialog) vs stack redo.

---

## 5. Broken redo → automatic undo

If lite/full parse throws `VExceptionUndo` (formula/id disaster mid-redo):

```
VPattern::LiteParseTree catch
    → QApplication::postEvent(this, new UndoEvent())
    → customEvent → undo the last command
```

Goal: the stack never sits on a DOM the parser cannot load. Other parse exceptions disable the GUI (`setGuiEnabled(false)`) for file-level corruption.

---

## 6. What is not on the undo stack

- Measurement file edits (those happen in SeamlyMe or as “load another file”)
- Layout generation (ephemeral; regenerate)
- Zoom/pan/selection
- Preferences

Changing size/height for multisize triggers lite parse without a dedicated undo command (it is a view on the same file).

---

## 7. Property browser integration

Live option edits go through `SaveToolOptions` so the property grid, tool dialog, and XML stay aligned. `VFormula` metatype editors in `app/seamly2d/core/` commit formulas the same way.

Related: [Module_DependencyTree.md](./Module_DependencyTree.md), [Module_FileIOParsing.md](./Module_FileIOParsing.md).
