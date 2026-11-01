# Seamly3D sample projects

Twelve synthetic projects, ready to open. These are not validated sewing patterns
or calibrated fabric presets. They require Seamly3D project format **v4**.
The `.sm3d` extension follows the `.sm2d` naming convention.

## Getting started

1. Launch Seamly3D, choose **File → Open project…** and open `05_dress_form_draping.sm3d`.
2. Press **Assemble**. Each file stores initial placements; deformation is computed
   after opening rather than loaded as a precomputed result.
3. In **Cloth**, inspect simulated time, speed, mesh counts and solver status. The
   status bar reports mean seam gap and mean mesh-edge strain.
4. In **Seams**, select a relationship: side A is solid and side B is dashed. In
   **Pieces**, try isolation, transparency and the top-centre pin.
5. **Reset assembly** restores the initial placements. **Fit view** adjusts the
   camera. Use **Save as…** to keep a variant without overwriting the reference.

For projects without a dress form (02, 03 and 10), press **Pause**, then **Fit view**
/ **F** after the panel falls. The camera does not automatically follow falling cloth.

Simulated time may advance more slowly than wall time. Rest means low velocity for
one second; it does not establish garment correctness or constraint convergence.
The 20-second simulated-time limit is a separate stopping condition.

## Projects and expected behaviour

| File | Test | What to observe |
| --- | --- | --- |
| [01_dress_form.sm3d](01_dress_form.sm3d) | Dress form only | Measurements, stand, navigation and camera views. There is no cloth to assemble. |
| [02_free_fall.sm3d](02_free_fall.sm3d) | Free horizontal square, no air damping | Gravity moves the panel down to the floor at approximately Y = −499 mm. After 0.1 simulated seconds, the expected drop is about 51.1 mm. Free fall should not noticeably deform the panel. |
| [03_suspended_panel.sm3d](03_suspended_panel.sm3d) | Horizontal panel pinned at the centre of its top edge | The pinned centre stays fixed while the rest drops and oscillates before settling. Disable the pin in Pieces to compare with a free panel. |
| [04_geometric_pouch.sm3d](04_geometric_pouch.sm3d) | Two panels and a seam arc that wraps through zero | Sides and bottom move together; the top edge stays outside the seam. This geometric test can flatten the panels onto each other: there is no pressure or stuffing. |
| [05_dress_form_draping.sm3d](05_dress_form_draping.sm3d) | Two sewn panels, central pins and dress-form contact | Side edges move together, cloth deforms under gravity and stays suspended. Uses a 35 mm mesh. Recommended starting point. |
| [06_mesh_comparison_50mm.sm3d](06_mesh_comparison_50mm.sm3d) | Same project, coarser mesh | Compare with 07 at the same simulated time. It should use fewer vertices and triangles while retaining seam endpoints and undeformed lengths. |
| [07_mesh_comparison_25mm.sm3d](07_mesh_comparison_25mm.sm3d) | Same project, finer mesh | More detail and computation. Final shape and gap may differ: resolution does not calibrate material behaviour. Only meshSize differs from 06. |
| [08_paused_seam.sm3d](08_paused_seam.sm3d) | Right seam disabled but retained in the file | The open side remains free. Enable Include in assembly and apply to compare results. Gap measures active seams only: a low value does not establish a closed garment. |
| [09_four_flared_panels.sm3d](09_four_flared_panels.sm3d) | Four trapezoidal panels around the form | Isolate a panel, check the four joins, then inspect the combined deformation. This is a synthetic flared assembly, not a production-ready skirt pattern. |
| [10_concave_outline.sm3d](10_concave_outline.sm3d) | Suspended L-shaped panel | The notch should remain open; meshing must not fill the missing area. Enable Mesh and change resolution. |
| [11_STRESS_wrong_direction.sm3d](11_STRESS_wrong_direction.sm3d) | Deliberately incorrect correspondence on one seam | Look for twisting, folding or an inconsistent assembly. Compare the two sides in Seams and correct Reverse correspondence on side B. Even low gap and strain can conceal an incorrect pairing. Do not use this as a correct assembly example. |
| [12_endpoints_near_corners.sm3d](12_endpoints_near_corners.sm3d) | Rounded percentages and a 1 mm offset of the back panel | Simulation should advance without microscopic triangles or an immediate instability stop. This covers the previous corner regression; the small placement offset makes it different from 07. |

## Evaluating results

- **Seams:** compare contours, endpoints and direction before reading gap. A small
  gap can accompany an incorrect relationship, as demonstrated by project 11.
- **Mesh:** compare 06 and 07 at equal simulated time and check that corners and
  references remain. More triangles alone do not guarantee better accuracy.
- **Physics:** start with 02 and 03, where behaviour is easy to interpret, then
  inspect contact and sewing in 05 and 09.
- **Editing and recovery:** change a measurement or seam, try Undo/Redo, save a
  copy and reopen it. Files store authored inputs rather than simulation frames.
- **Limits:** self-collision, layers, continuous collision detection and calibrated
  textiles are not implemented. Intersecting cloth is not a valid physical result.
  Pins can also create incompatible constraints.

## Verification

Every file was loaded and meshed with the Seamly3D model. Physical cases were
advanced until rest or a requested limit of 1,200 frames (20 seconds); geometric
cases were advanced through 2,400 iterations. [VERIFICATION.json](VERIFICATION.json)
records intermediate checkpoints, mesh counts and file hashes. None produced
non-finite values or a numerical-instability stop in this local check. These checks
do not replace validation against real garments.

Opening and native Metal frame generation were checked with the companion inside
the macOS bundle. The three included previews show one simulated second for
projects 05 and 09, and 300 geometric iterations for project 11:

- [Dress-form draping](previews/05_dress_form_draping.png)
- [Four flared panels](previews/09_four_flared_panels.png)
- [Stress: wrong direction](previews/11_STRESS_wrong_direction.png)

Local measurements for the dress-form projects at rest:

| Project | Vertices | Mean gap | Mean mesh-edge strain |
| --- | ---: | ---: | ---: |
| 05 · 35 mm mesh | 2018 | 0.462 mm | 5.1% |
| 06 · 50 mm mesh | 706 | 0.954 mm | 4.3% |
| 07 · 25 mm mesh | 2690 | 0.360 mm | 4.3% |
| 09 · four panels | 1204 | 0.042 mm | 2.4% |

These values depend on the project and current solver. They are not acceptance
tolerances or garment-fit measurements. Verification covers the local macOS setup.
Filenames and descriptive labels have been translated to English; all simulation
inputs were compared against the previously measured projects and are unchanged.

Regenerate the `.sm3d` files from the repository root:

```sh
python3 scripts/seamly3d/generate_demos.py
```

The generator overwrites the twelve projects. It does not recompute the report or
previews.
