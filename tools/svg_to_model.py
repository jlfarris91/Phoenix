
"""
Convert tower.svg to line_list_2d model format (like ArrowMissile.json)
"""


import re
import json
import sys
from svgpathtools import svg2paths2

def main():
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
    for path, attr in zip(paths, attributes):
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
    # Normalize coordinates to [-1, 1] range, preserving aspect ratio
    all_x = [l[1] for l in lines] + [l[3] for l in lines]
    all_y = [l[2] for l in lines] + [l[4] for l in lines]
    min_x, max_x = min(all_x), max(all_x)
    min_y, max_y = min(all_y), max(all_y)
    width = max_x - min_x
    height = max_y - min_y
    # Pad the shorter axis to make the bounds square
    if width > height:
        pad = (width - height) / 2
        min_y -= pad
        max_y += pad
    elif height > width:
        pad = (height - width) / 2
        min_x -= pad
        max_x += pad
    def norm(val, minv, maxv):
        return 2 * (val - minv) / (maxv - minv) - 1
    def norm_y(val, minv, maxv):
        # Flip Y axis so model is upright
        return -1 * (2 * (val - minv) / (maxv - minv) - 1)
    for l in lines:
        l[1] = norm(l[1], min_x, max_x)
        l[3] = norm(l[3], min_x, max_x)
        l[2] = norm_y(l[2], min_y, max_y)
        l[4] = norm_y(l[4], min_y, max_y)
    model = {
        "colors": color_list,
        "data": [float(v) for line in lines for v in line],
        "format": "line_list_2d"
    }
    with open(out_path, 'w') as f:
        json.dump(model, f, indent=2)
    print(f"Saved model with {len(lines)} lines to {out_path}")

if __name__ == '__main__':
    main()
