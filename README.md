# Biblioteca 3D

An interactive desktop library built with C++20, SDL2, OpenGL 3.3 and Cairo.

**Platform status:** Linux x86-64 is the primary supported target. The repository also builds a native Windows x64 executable using GitHub Actions. Windows builds are experimental until tested with a real graphics driver.

## Experience

The library has two interconnected rooms. The children's room contains 40 interactive books arranged along the front, side and back shelves. The teen and adult room has 19 longer readings, each with eight pages, arranged around a reading space with four lanterns. Both rooms have their own searchable catalogs and are connected by a door.

Each story combines illustrated reading with animated, foldable 3D pop-up scenes. The subject matter includes Mars and Europa, the Moon, Saturn, Earth observation, orbital spacecraft, solar activity and NASA missions. A rocket launches over 2.10 seconds when its page opens, followed by a fading trail. Rovers, antennas, spacecraft and beacons move independently. Pop-up geometry folds onto the paper during page turns.

Selecting a book from a shelf or a catalog makes the camera approach the reading position before opening the book. An open reference book on a lectern, next to the door in the adult room, contains mission references and clickable links.

### Reading collections

The original collection includes themed stories with unique scenes for every double-page spread. Additional titles are adapted from the Apollo 11 and 12, lunar exploration, Mars, Kepler and Pioneer 10 source documents. The content includes both shorter children's stories and longer technical readings.

**Content language:** The legacy reading collection and several embedded titles are still in Spanish. Localization of all story prose is separate from the Windows build and is not yet complete.

## Visual direction

The children's room resembles a celestial cartography workshop, with turquoise ceramics, brass markings, carved wooden shelves and orbital diagrams around a central hearth. The adult room uses walnut, olive green, opal-glass lighting and restrained typography. Covers use credited NASA mission images. Pop-ups are modeled as physical paper structures with editable OBJ meshes, per-vertex colors and materials.

Visual rules are documented in [DESIGN.md](DESIGN.md). Model and image sources are credited in [ASSETS_OPEN.md](ASSETS_OPEN.md).

## Linux installation

Keep the `assets/` directory next to the executable. On Arch Linux or CachyOS, install the runtime equivalents of SDL2, Cairo, Fontconfig and OpenGL (for example `sdl2-compat`, `cairo`, `fontconfig`, `libglvnd` and `mesa`).

From the extracted project directory:

```bash
./ejecutable.sh
```

To compile from source, install GCC with C++20 support and the development dependencies, then run:

```bash
make -j2
make test
./biblioteca
```

For render regression frames:

```bash
./biblioteca --render-check ./frames
```

## Windows x64

The repository contains an MSYS2 UCRT64 build of the same desktop application, plus a GitHub Actions workflow that packages and tests a portable ZIP on a Windows runner. This is a native Win32 program, **not** a UWP application and not a Wine executable.

When a build artifact is available, extract the entire portable ZIP and run `biblioteca.exe` from the extracted directory. Keep the DLL files and `assets/` directory beside the executable. An OpenGL 3.3-capable graphics driver is required.

GitHub Actions compile and smoke tests do not prove that the full 3D interface renders correctly on arbitrary GPUs. Perform a manual native Windows acceptance test before publishing a stable release. See [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md) for the validation procedure.

## Controls

| Input | Action |
| --- | --- |
| W, A, S, D | Walk |
| Shift + movement | Move faster |
| Hold right mouse button + move | Look around |
| Tab | Capture or release mouse control |
| Click a spine, or point at it and press E | Approach and open a book |
| / or K | Open the current room catalog |
| G | Walk toward the other room |
| F | Approach the reference lectern |
| Left / Right arrows | Turn pages when reading |
| Mouse wheel | Navigate or zoom, depending on context |
| Left-drag an open book | Rotate the book |
| M | Pause or resume 3D model animations |
| Esc | Release camera control, cancel an action or close the reader |
| F2 | Show or hide the controls guide |
| F11 | Toggle full screen |

## Source layout

| Directory or file | Purpose |
| --- | --- |
| `src/main.cpp`, `src/desktop.cpp` | Entry point, event loop and desktop window |
| `src/ui.cpp`, `src/paint.cpp` | Interaction and interface drawing |
| `src/library.cpp`, `src/printing.cpp` | Reader, book selection, page geometry and cover rendering |
| `src/room.cpp`, `src/room_layout.cpp` | Room meshes, navigation and collision boundaries |
| `src/render.cpp`, `assets/shaders/` | OpenGL renderer and shaders |
| `src/catalog.inc`, `src/stories.cpp`, `src/child_text.cpp` | Core catalog and stories |
| `src/imported_books.cpp`, `src/adult_books.cpp` | Adapted and adult readings |
| `src/popup.cpp`, `assets/popups/` | Mesh loading, folded paper and animation |
| `src/checks.cpp` | Geometry and interaction regression tests |
| `tools/` | Editable pop-up and material generation scripts |

Regenerate assets as needed:

```bash
python3 tools/generate_popups.py
python3 tools/generate_materials.py
```

## Documentation and licenses

Read [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md) for detailed Linux and Windows instructions, test procedures and known limits. Credits and third-party licenses are kept in [ASSETS_OPEN.md](ASSETS_OPEN.md) and `assets/fonts/`. The project license is in [LICENSE](LICENSE).
