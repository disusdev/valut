# Vault

A minimal, high-performance 3D graphics engine and software rasterizer written in C.

## Overview

Xeno is a from-scratch 3D graphics engine that implements software rendering techniques. It includes a custom software rasterizer (`nude`), comprehensive math library, asset management system, and cross-platform windowing.

## Features

### Graphics (`nude`)
- Software rasterizer with depth buffering
- Triangle rendering with texturing and shading
- Wireframe and filled triangle modes
- Backface culling
- Perspective-correct texturing
- Multiple drawing flags (wireframe, dots, normals, etc.)

### Math Library (`mm`)
- Vector math (2D, 3D, 4D)
- Matrix operations (4x4 matrices)
- Rotors for 3D rotations
- Noise generation (Perlin-like)
- RNG with various distributions
- Fast math functions (sin, cos, sqrt using lookup tables)

### Asset System
- **Asset Packer**: Tool to pack textures and 3D models into binary blobs
- **OBJ Loader**: Supports .obj file format
- **Texture Loading**: PNG, JPEG, BMP, TGA, PSD, GIF, HDR formats via stb_image
- Runtime asset loading from memory

### Platform Layer
- Cross-platform window creation
- Input handling (keyboard, mouse)
- Audio output (Windows WASAPI implementation)
- Event system
- Screenshot capture

### Data Structures
- Dynamic arrays (`darray`)
- Arena-based memory allocation
- Mesh representation with vertices, indices, normals, colors, UVs
- Camera system with orbit and FPS-style controls

## Architecture

The codebase follows a clean, modular architecture:

```
src/
├── nude.[ch]      # Software rasterizer
├── mm.[ch]        # Math library
├── xeno.[ch]      # Platform abstraction
├── game.[ch]      # Demo application
├── asset_*.[ch]   # Asset loading system
├── mesh*.[ch]     # 3D mesh utilities
├── camera.[ch]    # Camera controls
├── input.[ch]     # Input handling
├── event.[ch]     # Event system
├── texture.[ch]   # Texture management
├── text.[ch]      # Bitmap font rendering
└── darray.[ch]    # Dynamic arrays
```

## Building

### Prerequisites
- Windows (currently the only supported platform)
- C compiler (MSVC, GCC, or Clang)

### Build Commands

**Using Clang:**
```bash
clang xeno_win.c nude.c game.c mm.c mm_tables.c mesh_primitive.c camera.c text.c event.c input.c darray.c obj_loader.c texture.c framegraph.c collision.c asset_loader.c -o demo.exe -luser32 -lgdi32 -lole32 -lavrt -std=c89 -O3 -ffast-math -march=native -fstrict-aliasing -DNDEBIG -D_CRT_SECURE_NO_WARNINGS
```

### Asset Preparation

# Pack some assets
asset_packer.exe game cube.obj sphere.obj texture.png

# This creates: game.assets and game_assets.c

Then pass the asset file as argument:
```bash
demo.exe game.assets
```

## Usage

### Running the Demo

```bash
# Or run with custom assets
demo.exe path/to/your/assets.bin
```

The demo runs at 800x800 resolution and includes:
- 3D camera controls (WASD + Look with arrows)
- Dynamic lighting
- Multiple rendering modes (number keys 1-6)
- Animated dot grid background
- 3D ship/planet models
- Screenshot capture (F12)

### Controls

| Key | Action |
|-----|--------|
| WASD | Move camera |
| Arrows | Look around |
| Space/C | Up/Down |
| 1 | Toggle shading |
| 2 | Toggle wireframe |
| 3 | Toggle dot mode |
| 4 | Toggle culling |
| 5 | Toggle backface |
| 6 | Toggle normals |
| F12 | Screenshot |
| ESC | Quit |

### Code Example

```c
#include "nude.h"
#include "mm.h"

// Initialize
uint32_t* color_buffer = malloc(width * height * sizeof(uint32_t));
float* depth_buffer = malloc(width * height * sizeof(float));

// Draw a triangle
n_triangle_fill_draw(color_buffer, depth_buffer,
    100, 100, 0.5f, 0xFF0000FF,  // vertex 1: x,y,z,color
    200, 150, 0.5f, 0x00FF00FF,  // vertex 2
    150, 250, 0.5f, 0x0000FFFF); // vertex 3

// Render mesh
mat4_t view = camera_view(&camera);
mat4_t proj = camera_projection(&camera, aspect_ratio);
n_mesh_draw(color_buffer, depth_buffer, width, height,
    mesh, view, proj);
```

## File Formats

### Asset Blob Format
Custom binary format containing:
- Textures (RGBA8888)
- Meshes (vertices, indices, normals, colors, UVs)

### Supported Model Formats
- Wavefront OBJ (.obj)

### Supported Texture Formats
- PNG, JPEG, BMP, TGA, PSD, GIF, HDR, PNM

## Performance

- Software rendering optimized for performance
- Fast math using precomputed lookup tables
- Minimal memory footprint
- No external dependencies except stb_image for asset packer tool

## Contributing

This is a clean, minimal codebase designed for learning and experimentation. The code style emphasizes:

- No comments (self-documenting code)
- Consistent naming conventions
- Modular architecture
- Minimal dependencies

## Credits

- stb_image: Public domain image loading library
- Mathematics based on geometric algebra and rotors principles

---

*Built from scratch with C and software rendering techniques.*
