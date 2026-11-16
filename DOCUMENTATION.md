# Starry Library — Documentation

## Overview

Starry Library is currently a desktop application for exploring a virtual library and reading illustrated stories. Two connected rooms serve different audiences. One is for children, and the other is for teens and adults. Each room has its own catalog and reading layout. Select a book from a shelf or the current room's catalog to open it in a reader with animated 3D pop-ups.

The stories cover space missions, planets, satellites, rockets, and exploration of the Moon and Mars.

## Upcoming mobile versions

Android and iOS editions of **Starry Library** are planned to be added to this GitHub repository **in the next few days**. These mobile editions are not published yet. Release timing may change while platform-specific builds and device testing are completed.

The current build and installation instructions below apply to Linux and Windows desktop versions only.

## Technologies

| Component | Role |
| --- | --- |
| C++20 | Application logic, geometry, reader states, and animation. |
| SDL2 | Window, keyboard and mouse input, and graphics context. |
| OpenGL 3.3 | Shader-based rendering for rooms, furniture, books, and pop-ups. |
| Cairo | Text and interface drawing. |
| Fontconfig | Registration of bundled Barlow fonts on Linux. |
| FreeType | Font loading on Windows. |
| GLEW | OpenGL function loading on Windows. |
| Make | Desktop builds and tests. |

Keep the `assets/` directory beside the executable. It contains the shaders, textures, fonts, and models loaded at runtime.

## Source Layout

| File or directory | Responsibility |
| --- | --- |
| `src/main.cpp` | Entry point and selection of interactive, test, and frame capture modes. |
| `src/desktop.cpp` | Desktop application loop and SDL events. |
| `src/ui.cpp` | Controls, catalog, and interface elements. |
| `src/library.cpp` | Book selection, reader placement, camera movement, and page transitions. |
| `src/room.cpp`, `src/room_layout.cpp` | Room geometry, layout, boundaries, and navigation. |
| `src/render.cpp`, `assets/shaders/` | OpenGL scene setup and drawing. |
| `src/paint.cpp`, `src/printing.cpp` | Drawing of interface elements, pages, covers, and spines. |
| `src/popup.cpp`, `assets/popups/` | Loading and animation of OBJ pop-up scenes. |
| `src/catalog.inc`, `src/stories.cpp`, `src/child_text.cpp` | Catalog, stories, and children's reading material. |
| `src/imported_books.cpp`, `src/adult_books.cpp`, `src/technical_notes.cpp` | Adapted readings and longer texts. |
| `src/checks.cpp` | Geometry, navigation, and scene checks. |
| `tools/` | Pop-up model and material generation. |

The rooms and reader use 3D meshes. Each pop-up scene is an OBJ file linked to a page. Covers, fonts, and material textures are stored in `assets/covers/`, `assets/fonts/`, and `assets/materials/`.

## Installation and Launch

### Linux

The application requires x86-64 Linux, a driver that supports OpenGL 3.3, and SDL2, Cairo, OpenGL, and Fontconfig. For Arch Linux and CachyOS, the project README lists `sdl2-compat`, `cairo`, `fontconfig`, `libglvnd`, and a graphics driver such as `mesa`.

Run the launcher from the extracted project directory.

```bash
./ejecutable.sh
```

The launcher runs `biblioteca` from its own directory. To build and run from source, install the development headers for the libraries above and run the following commands.

```bash
make -j2
./biblioteca
```

The Makefile uses C++20. The executable resolves `assets/` relative to its own location.

### Windows x64

The GitHub Actions workflow produces `Biblioteca-3D-Windows-x64.zip`, a portable 64-bit build. Download it from a successful workflow run. The ZIP's size depends on the build, and an OpenGL 3.3-compatible graphics driver is required.

1. Extract the entire ZIP.
2. Open the extracted directory.
3. Run `biblioteca.exe`.

To move the application, copy the entire extracted directory; keep the required DLLs, `assets/`, and `etc/` beside `biblioteca.exe`.

| Path | Contents |
| --- | --- |
| `biblioteca.exe` | Windows x86-64 graphical executable. |
| `*.dll` | Required Windows libraries, including SDL2, Cairo, GLEW, Fontconfig, and the C++ runtime. |
| `assets/covers/` | 22 PNG covers. |
| `assets/popups/` and `assets/open_models/` | 67 OBJ pop-up models and 3 adapted models (AcrimSAT, Viking Lander, and Astronaut). |
| `assets/fonts/` | Five Barlow and Liberation Serif font files and their licenses. |
| `assets/shaders/` and `assets/materials/` | Two GLSL shaders and the `grain.ppm` texture. |
| `etc/fonts/` | Bundled Fontconfig configuration. |
| `LICENSE` and `ASSETS_OPEN.md` | Project license and asset credits. |

The Windows executable loads fonts through FreeType and accesses OpenGL through GLEW. The application DLLs are bundled with the ZIP.

## Controls

English is the initial interface language. Use F9 or the language control to switch to Spanish.

| Control | Action |
| --- | --- |
| `W`, `A`, `S`, `D` | Move around the room. |
| Shift + movement | Move faster. |
| Hold the right mouse button and move the mouse | Look around. |
| `Tab` | Capture or release the mouse for camera control. |
| Click a spine or look at it and press `E` | Approach the shelf and open the book. |
| `/` or `K` | Open the current room's catalog. |
| `G` | Walk to the other room. |
| `F` | Approach the reference lectern. |
| Left and right arrow keys | Turn pages while a book is open. |
| Mouse wheel | Scroll or adjust the view while reading. |
| Left-drag an open book | Rotate the book. |
| `M` | Pause or resume model animation. |
| `Esc` | Release the mouse, cancel a walk, or close the reader. |
| `F2` | Toggle the controls guide. |
| `F9` | Switch between English and Spanish. |
| `F11` | Toggle full screen. |

Selecting a book from a distance moves the camera to the reading position before opening it. Room layout and movement boundaries are defined in `src/room_layout.cpp`.

## Testing and Asset Generation

### Linux

```bash
make test
./biblioteca --render-check ./cuadros
```

`make test` runs `./biblioteca --self-test`. The `--render-check` mode exercises reading scenes and saves reference frames to the specified directory. It requires the graphics driver and the project assets.

Run these scripts from the project root to regenerate the pop-up meshes and the material texture.

```bash
python3 tools/generate_popups.py
python3 tools/generate_materials.py
```

### Windows x64

The previous Windows package had structural checks, but native Windows execution is not yet reliably confirmed. Wine or successful compilation alone is insufficient evidence. Treat Windows builds as experimental until real-machine validation.

## Assets and Credits

Mission imagery and adapted models are credited in `ASSETS_OPEN.md`, which also records their sources and usage terms. Barlow and Liberation Serif licenses are in `assets/fonts/`. Design notes are in `DESIGN.md` in the source tree.

The build commands apply to Linux. The Windows instructions and asset counts describe the prior portable ZIP and must be rechecked for each release.
## Windows Native Verification

The Linux and Windows builds share the source code in the main branch. Do not label Windows packages stable based solely on CI or Wine. A portable ZIP and a self-extracting EXE require independent testing.

1. Download the exact candidate artifact on a real Windows 11 x64 machine; record its SHA-256 checksum.
2. Extract the portable ZIP completely, preserving DLLs, `assets/` and `etc/` relative to `biblioteca.exe`. For a standalone self-extracting EXE, launch that file directly.
3. Double-click without MSYS2, Wine, or development libraries installed. Confirm that a window stays open rather than immediately closing.
4. Test camera movement, both rooms, catalog, book opening, turning pages, pop-up animations, fonts and accents, covers, and 3D models against Linux screenshots.
5. Restart; test fullscreen and windowed operation and asset loading again.
6. For failures, check Windows Event Viewer > Windows Logs > Application and record faulting module and exception code. If possible, launch from `cmd.exe` to capture diagnostic output. Inspect dependencies and resource paths.
7. Record OS build, GPU, graphics driver, package type, checksum, screenshots and test results. Repeat on a second real Windows computer before release.

Investigate native runtime DLLs, OpenGL initialization, font configuration, packaging paths, and self-extraction when a build works in Wine but fails on Windows. Do not assume the root cause without logs.
