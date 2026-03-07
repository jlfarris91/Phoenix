
"""
Convert tower.svg to line_list_2d model format (like ArrowMissile.json)
"""


import re
import json
import sys
from svgpathtools import svg2paths2

def main():
    # ...existing code...
    # Parse command-line arguments
    import argparse
    parser = argparse.ArgumentParser(description="Convert SVG to line_list_2d model format.")
    parser.add_argument('svg_path', nargs='?', default='tools/tower.svg', help='Input SVG file')
    parser.add_argument('out_path', nargs='?', default=r'tests\\TestRTS\\Data\\Catalogs\\Core\\Assets\\Units\\Human\\Tower\\Tower.json', help='Output model JSON file')
    args = parser.parse_args()

    svg_path = args.svg_path
    out_path = args.out_path

    paths, attributes, svg_attr = svg2paths2(svg_path)
    color_map = {}
    color_list = []
    lines = []

    # --- Find all path indices that are in the Sockets layer ---
    from xml.etree import ElementTree as ET
    tree = ET.parse(svg_path)
    root = tree.getroot()
    svg_ns = '{http://www.w3.org/2000/svg}'
    sockets_layer_ids = set()
    for g in root.findall(f'.//{svg_ns}g'):
        label = g.attrib.get(f'{{http://www.inkscape.org/namespaces/inkscape}}label') or g.attrib.get('label') or g.attrib.get('id','')
        if label.lower() == 'sockets':
            for elem in g:
                if 'id' in elem.attrib:
                    sockets_layer_ids.add(elem.attrib['id'])

    # --- Extract sockets from SVG XML ---
    # (reuse tree, root, svg_ns from above)
    width = float(root.attrib.get('width', svg_attr.get('width', '1000')).replace('px',''))
    height = float(root.attrib.get('height', svg_attr.get('height', '1000')).replace('px',''))
    cx_svg = width / 2
    cy_svg = height / 2
    sockets = []
    for g in root.findall(f'.//{svg_ns}g'):
        label = g.attrib.get(f'{{http://www.inkscape.org/namespaces/inkscape}}label') or g.attrib.get('label') or g.attrib.get('id','')
        if label.lower() == 'sockets':
            for elem in g:
                tag = elem.tag
                circ_id = elem.attrib.get('id','')
                cx = cy = None
                rotation = 0.0
                transform = elem.attrib.get('transform','')
                import re
                rot_match = re.search(r'rotate\(([-\d.]+)', transform)
                if rot_match:
                    try:
                        rotation = float(rot_match.group(1))
                    except ValueError:
                        rotation = 0.0
                if tag.endswith('circle'):
                    cx = elem.attrib.get('cx')
                    cy = elem.attrib.get('cy')
                elif tag.endswith('ellipse'):
                    cx = elem.attrib.get('cx')
                    cy = elem.attrib.get('cy')
                elif tag.endswith('rect'):
                    x = elem.attrib.get('x')
                    y = elem.attrib.get('y')
                    width_e = elem.attrib.get('width')
                    height_e = elem.attrib.get('height')
                    if x is not None and y is not None and width_e is not None and height_e is not None:
                        try:
                            cx = float(x) + float(width_e)/2
                            cy = float(y) + float(height_e)/2
                        except ValueError:
                            continue
                    else:
                        continue
                else:
                    continue
                if cx is not None and cy is not None:
                    try:
                        cx = float(cx)
                        cy = float(cy)
                        sockets.append({'id': circ_id, 'cx': cx, 'cy': cy, 'rotation': rotation})
                    except ValueError:
                        continue
    # ...existing code for lines...
    for idx, (path, attr) in enumerate(zip(paths, attributes)):
        # Exclude geometry from the Sockets layer
        if attr.get('id', '') in sockets_layer_ids:
            continue
        style = attr.get('style', '')
        color = None
        m = re.search(r'stroke:#([0-9a-fA-F]{6})', style)
        if m:
            color = '#' + m.group(1).upper() + 'FF'
        else:
            color = '#000000FF'
        if color not in color_map:
            color_map[color] = len(color_list)
            color_list.append(color)
        color_idx = color_map[color]
        # Discretize path into line segments
        for seg in path:
            # Only handle straight lines and polylines
            if seg.length() == 0:
                continue
            n = max(2, int(seg.length() // 1.0))
            pts = [seg.point(t) for t in [i/(n-1) for i in range(n)]]
            for i in range(len(pts)-1):
                x1, y1 = pts[i].real, pts[i].imag
                x2, y2 = pts[i+1].real, pts[i+1].imag
                lines.append([color_idx, x1, y1, x2, y2])
    if not lines:
        print("No lines found in SVG!")
        return
    # Normalize coordinates to [-1, 1] range using SVG page (viewBox or width/height)
    # Prefer viewBox if present, else use width/height attributes
    viewBox = root.attrib.get('viewBox') or svg_attr.get('viewBox')
    if viewBox:
        vb_vals = [float(v) for v in viewBox.replace(',', ' ').split()]
        min_x, min_y, width_box, height_box = vb_vals
        max_x = min_x + width_box
        max_y = min_y + height_box
    else:
        min_x = 0.0
        min_y = 0.0
        width_box = float(root.attrib.get('width', svg_attr.get('width', '1000')).replace('px',''))
        height_box = float(root.attrib.get('height', svg_attr.get('height', '1000')).replace('px',''))
        max_x = min_x + width_box
        max_y = min_y + height_box
    # Debug: print SVG page bounds
    print(f"SVG page bounds for normalization:")
    print(f"  min_x: {min_x}, max_x: {max_x}, width: {width_box}")
    print(f"  min_y: {min_y}, max_y: {max_y}, height: {height_box}")
    # Center coordinates so (0,0) is the SVG page center, no scaling
    cx = (min_x + max_x) / 2
    cy = (min_y + max_y) / 2
    # Transform geometry
    for l in lines:
        l[1] = l[1] - cx
        l[3] = l[3] - cx
        l[2] = -(l[2] - cy)  # Flip Y axis
        l[4] = -(l[4] - cy)
    # Transform sockets using SVG page center (even if outside)
    sockets_out = []
    for s in sockets:
        x = s['cx'] - cx
        y = -(s['cy'] - cy)
        sockets_out.append({
            'id': s['id'],
            'x': x,
            'y': y,
            'rotation': s.get('rotation', 0.0)
        })
    model = {
        "colors": color_list,
        "data": [float(v) for line in lines for v in line],
        "format": "line_list_2d",
        "sockets": sockets_out
    }
    with open(out_path, 'w') as f:
        json.dump(model, f, indent=2)
    print(f"Saved model with {len(lines)} lines to {out_path} and {len(sockets_out)} sockets")

if __name__ == '__main__':
    main()
