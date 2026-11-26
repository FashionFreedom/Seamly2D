# SPDX-License-Identifier: GPL-3.0-or-later
"""Independent, small ASCII DXF audit for our own Seamly-exported samples.

This reports what was actually exported. It is not an AAMA/ASTM certification
or a replacement importer. It does not copy or call BETA or Seamly code.
"""
from pathlib import Path
from collections import Counter
import argparse
import json


def field(entity, code, default=None):
    return next((v for c, v in entity if c == code), default)


def bbox(points):
    if not points:
        return None
    return [min(p[0] for p in points), min(p[1] for p in points),
            max(p[0] for p in points), max(p[1] for p in points)]


def area(points):
    if len(points) < 3:
        return 0
    return abs(sum(a[0]*b[1]-a[1]*b[0] for a, b in zip(points, points[1:]+points[:1]))) / 2


def audit(path):
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    pairs = [(int(lines[i].strip()), lines[i+1].strip()) for i in range(0, len(lines)-1, 2)]
    entities = []
    current = []
    for code, value in pairs:
        if code == 0:
            if current:
                entities.append(current)
            current = []
        current.append((code, value))
    if current:
        entities.append(current)
    blocks = []
    block = None
    poly = None
    for entity in entities:
        kind = field(entity, 0)
        layer = field(entity, 8, "0")
        if kind == "BLOCK":
            block = {"name": field(entity, 2), "entities": Counter(), "text": [],
                     "cut": [], "stitch": [], "other_polylines": [], "grain": [], "notches": []}
            poly = None
        elif kind == "ENDBLK" and block is not None:
            for contour in block["cut"] + block["stitch"]:
                contour["bbox_mm"] = bbox(contour["points"])
                contour["area_mm2"] = area(contour["points"])
            block["entities"] = dict(block["entities"])
            blocks.append(block)
            block = None
        elif block is not None:
            block["entities"][kind + "@" + layer] += 1
            if kind in ("TEXT", "MTEXT"):
                block["text"].append(field(entity, 1, ""))
            elif kind in ("POLYLINE", "LWPOLYLINE"):
                poly = {"layer": layer, "closed": bool(int(field(entity, 70, "0")) & 1), "points": []}
                block["cut" if layer == "1" else "stitch" if layer == "8" else "other_polylines"].append(poly)
                if kind == "LWPOLYLINE":
                    x = None
                    for c, v in entity:
                        if c == 10:
                            x = float(v)
                        elif c == 20 and x is not None:
                            poly["points"].append([x, float(v)])
            elif kind == "VERTEX" and poly is not None:
                poly["points"].append([float(field(entity, 10, "0")), float(field(entity, 20, "0"))])
            elif kind == "SEQEND":
                poly = None
            elif kind == "LINE" and layer in ("4", "7"):
                segment = [[float(field(entity, 10, "0")), float(field(entity, 20, "0"))],
                           [float(field(entity, 11, "0")), float(field(entity, 21, "0"))]]
                block["grain" if layer == "7" else "notches"].append(segment)
    return {"file": str(path), "blocks": blocks}


if __name__ == '__main__':
    import sys
    before = {b['name']: b for b in audit(Path(sys.argv[1]))['blocks']}
    after = {b['name']: b for b in audit(Path(sys.argv[2]))['blocks']}
    same = before == after
    print(json.dumps({'equal_parsed_block_geometry_and_text': same, 'blocks': sorted(before)}, indent=2))
    raise SystemExit(0 if same else 1)
