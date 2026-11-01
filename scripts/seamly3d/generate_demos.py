#!/usr/bin/env python3
"""Regenerate synthetic Seamly3D v4 fixtures; no third-party patterns or assets."""
from copy import deepcopy
import json
import math
from pathlib import Path

DEST = Path(__file__).resolve().parents[2] / 'src/app/share/samples/seamly3d'


def project(title, physical=False, mesh=25, form=False):
    return dict(format='seamly3d', version=4, units='mm', source='Synthetic demo: ' + title,
                meshSize=mesh, simulation=dict(physical=physical, density=200,
                stretchCompliance=0.000001, bendCompliance=0.01, damping=2, friction=0.4),
                dressForm=dict(enabled=form, contact=True, stand=True, bust=920,
                waist=720, hips=960, neck=360, shoulders=380, height=760, clearance=3),
                pieces=[], seams=[])


def piece(ident, name, outline, translation=(0, 0, 0), rotation=(0, 0, 0), pin=False):
    return dict(id=ident, name=name, outline=outline, translation=list(translation),
                rotation=list(rotation), pinTop=pin)


def seam(ident, name, a, b, start_a, end_a, start_b, end_b, reverse=True, enabled=True):
    return dict(id=ident, name=name, a=a, b=b, startA=start_a, endA=end_a,
                startB=start_b, endB=end_b, reverse=reverse, enabled=enabled)


def corner(outline, index):
    lengths = [math.dist(a, b) for a, b in zip(outline, outline[1:] + outline[:1])]
    return sum(lengths[:index]) / sum(lengths)


def write(name, data):
    (DEST / name).write_text(json.dumps(data, indent=2, ensure_ascii=False) + '\n')


def main():
    DEST.mkdir(parents=True, exist_ok=True)
    write('01_dress_form.sm3d', project('dress form without cloth', form=True))

    square = [[-100, 100], [100, 100], [100, -100], [-100, -100]]
    falling = project('free fall and floor contact', physical=True)
    falling['simulation']['damping'] = 0
    falling['pieces'] = [piece('sheet', 'Free panel 200 × 200 mm', square,
                               translation=(0, 200, 0), rotation=(90, 0, 0))]
    write('02_free_fall.sm3d', falling)

    suspended = project('suspended panel', physical=True)
    suspended['simulation']['damping'] = 3
    outline = [[-150, 200], [150, 200], [150, -200], [-150, -200]]
    suspended['pieces'] = [piece('sheet', 'Panel pinned at the top centre', outline,
                                 translation=(0, 100, 0), rotation=(90, 0, 0), pin=True)]
    write('03_suspended_panel.sm3d', suspended)

    pouch = project('open pouch, geometric assembly', mesh=15)
    outline = [[-45, 50], [45, 50], [55, 30], [55, -35], [35, -55],
               [-35, -55], [-55, -35], [-55, 30]]
    pouch['pieces'] = [piece('front', 'Pouch front', outline, (0, 0, 35)),
                       piece('back', 'Pouch back', outline, (0, 0, -35))]
    pouch['seams'] = [seam('sides', 'Sides and bottom, open top', 'front', 'back',
                           corner(outline, 1), 0, corner(outline, 1), 0, reverse=False)]
    write('04_geometric_pouch.sm3d', pouch)

    body = project('two panels on the dress form', physical=True, mesh=35, form=True)
    outline = [[-225, 300], [225, 300], [225, -300], [-225, -300]]
    body['pieces'] = [piece('front', 'Front panel', outline, (0, 34.4, 182.2), pin=True),
                      piece('back', 'Back panel', outline, (0, 34.4, -182.2), (0, 180, 0), pin=True)]
    body['seams'] = [seam('right', 'Right side', 'front', 'back', corner(outline, 1),
                          corner(outline, 2), corner(outline, 3), 0),
                     seam('left', 'Left side', 'front', 'back', corner(outline, 3),
                          0, corner(outline, 1), corner(outline, 2))]
    write('05_dress_form_draping.sm3d', body)
    for size, name in [(50, '06_mesh_comparison_50mm.sm3d'), (25, '07_mesh_comparison_25mm.sm3d')]:
        variant = deepcopy(body)
        variant['meshSize'] = size
        write(name, variant)
    opened = deepcopy(body)
    opened['source'] = 'Synthetic demo: one open side, seam paused'
    opened['seams'][0]['enabled'] = False
    write('08_paused_seam.sm3d', opened)

    skirt = project('four flared panels, not a validated sewing pattern', physical=True, mesh=35, form=True)
    outline = [[-100, 200], [100, 200], [180, -200], [-180, -200]]
    for i, name in enumerate(['Front', 'Right', 'Back', 'Left']):
        angle = i * math.pi / 2
        skirt['pieces'].append(piece(f'panel-{i}', name + ' · flared panel', outline,
                                     (165 * math.sin(angle), -200, 165 * math.cos(angle)),
                                     (0, i * 90, 0), pin=True))
        skirt['seams'].append(seam(f'side-{i}', f'Join {i+1}', f'panel-{i}',
                                   f'panel-{(i+1)%4}', corner(outline, 1),
                                   corner(outline, 2), corner(outline, 3), 0))
    write('09_four_flared_panels.sm3d', skirt)

    concave = project('concave L-shaped outline', physical=True)
    outline = [[-150, 150], [150, 150], [150, 50], [0, 50], [0, -150], [-150, -150]]
    concave['pieces'] = [piece('concave', 'Concave L-shaped panel', outline,
                               (0, 120, 0), (90, 0, 0), pin=True)]
    write('10_concave_outline.sm3d', concave)

    wrong = deepcopy(body)
    wrong['source'] = 'Deliberate STRESS: wrong sewing direction, not a correct assembly'
    wrong['simulation']['physical'] = False
    for panel in wrong['pieces']:
        panel['pinTop'] = False
    wrong['seams'][0]['name'] = 'STRESS: deliberately wrong direction'
    wrong['seams'][0]['reverse'] = False
    write('11_STRESS_wrong_direction.sm3d', wrong)

    rounded = deepcopy(body)
    rounded['source'] = 'Synthetic demo: regression: rounded percentages near corners'
    rounded['meshSize'] = 25
    rounded['pieces'][1]['translation'][0] = 1
    for relationship in rounded['seams']:
        for key in ('startA', 'endA', 'startB', 'endB'):
            relationship[key] = round(relationship[key], 8)
    write('12_endpoints_near_corners.sm3d', rounded)


if __name__ == '__main__':
    main()
