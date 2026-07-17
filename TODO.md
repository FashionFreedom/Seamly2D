# TODO — Tagged SVG Handoff to SeamlyLayout

See `PROJECT_PLAN.md` for full details. Check off subtasks as they are accomplished; when every subtask of a task is complete, move the task to `COMPLETED.md`.

## Task 8 — Verification

- [ ] Build `vlayout`, `vformat`, `seamly2d` on branch `svg-update`
- [ ] Layout Mode click produces tagged SVG; SeamlyLayout (or stub) launches with it
- [ ] Manual SVG exports (pieces + layout, text-as-paths on/off) carry the attributes
- [ ] SVG inspection: all groups tagged, ids unique, no empty groups or `M0,0` paths
- [ ] Visual diff vs baseline SVG (geometry/colors/line weights/labels unchanged) — baseline: `status-docs/baseline/richmond-shirt-baseline_pieces.svg`
- [ ] DXF / PDF / PNG export regression check
- [ ] Document the `data-*` contract in `./status-docs/` and mirror into `./seamlyLayout/docs/`
