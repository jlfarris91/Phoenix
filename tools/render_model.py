import matplotlib.pyplot as plt
import matplotlib.collections as mc
import numpy as np
import json
import sys
import os

# Convert hex colors to RGB
def hex_to_rgb(hex_color):
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) / 255 for i in (0, 2, 4))

# Load model from JSON file
if len(sys.argv) < 2:
    print("Error: Please specify a model file")
    print("Usage: python render_model.py <model_file.json>")
    sys.exit(1)

model_file = sys.argv[1]

try:
    with open(model_file, 'r') as f:
        model_data = json.load(f)
except FileNotFoundError:
    print(f"Error: Model file '{model_file}' not found")
    sys.exit(1)

# Parse colors
colors_map = {}
if isinstance(model_data["colors"], list):
    # Array format: colors are indexed by position
    for idx, hex_color in enumerate(model_data["colors"]):
        colors_map[idx] = hex_to_rgb(hex_color)
else:
    # Object format: colors are keyed by string index
    for idx_str, hex_color in model_data["colors"].items():
        colors_map[int(idx_str)] = hex_to_rgb(hex_color)

# Parse lines from JSON
line_segments = []
line_colors = []
data = model_data["data"]

# Check format
model_format = model_data.get("format", "line_list_2d")

if model_format == "line_list_2d":
    # Format: [color, x1, y1, x2, y2] - can be flat list or list of lists
    if data and isinstance(data[0], (list, tuple)):
        # List of lists format
        for line in data:
            color_idx = int(line[0])
            x1, y1 = line[1], line[2]
            x2, y2 = line[3], line[4]
            
            line_segments.append([(x1, y1), (x2, y2)])
            line_colors.append(colors_map[color_idx])
    else:
        # Flat list format
        i = 0
        while i < len(data):
            color_idx = int(data[i])
            x1, y1 = data[i+1], data[i+2]
            x2, y2 = data[i+3], data[i+4]
            
            line_segments.append([(x1, y1), (x2, y2)])
            line_colors.append(colors_map[color_idx])
            i += 5
elif model_format == "line_list_3d":
    # Legacy 3D format: [color, x1, y1, z1, x2, y2, z2] - not supported in simplified renderer
    print("Error: This script only supports line_list_2d format. Please use pre-projected 2D coordinates.")
    sys.exit(1)

# Create figure
fig, ax = plt.subplots(figsize=(12, 12))

# Create line collection
lc = mc.LineCollection(line_segments, colors=line_colors, linewidths=2.5)
ax.add_collection(lc)


# Set up the plot
ax.set_aspect('equal')
ax.set_facecolor('#1a1a1a')
fig.patch.set_facecolor('#1a1a1a')
ax.set_xticks([])
ax.set_yticks([])
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.spines['bottom'].set_visible(False)
ax.spines['left'].set_visible(False)
ax.set_xlim([-1, 1])
ax.set_ylim([-1, 1])

# Set up output directory relative to the script location (root-level .build folder)
script_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(script_dir)  # Go up one level from tools/ to root
build_dir = os.path.join(root_dir, ".build")
os.makedirs(build_dir, exist_ok=True)

# Get output filename from model base name (without path)
base_filename = os.path.basename(model_file)
output_file = os.path.join(build_dir, base_filename.replace('.json', '.png'))

# Save and show
plt.savefig(output_file, dpi=150, bbox_inches='tight', facecolor='#1a1a1a')
print(f"Visualization saved as '{output_file}'")
plt.show()
