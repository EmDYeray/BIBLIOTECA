# Windows x64 — native desktop build

Starry Library runs as a native C++20 Windows desktop application built with SDL2, OpenGL 3.3, Cairo, Fontconfig and GLEW. It is not a UWP or Wine application.

## Build using GitHub Actions

Open [Windows native build and verification](https://github.com/EmDYeray/BIBLIOTECA/actions/workflows/windows.yml). Choose a successful run and download its artifact **Biblioteca-Windows-test-build**.

The artifact contains:
- `Biblioteca-3D-Windows-x64.zip` — recommended portable build, containing `biblioteca.exe`, dependent DLLs, `assets/` and font configuration.
- `SHA256SUMS.txt` — ZIP integrity hash.
- `Biblioteca-Windows.exe` — optional NSIS self-extracting wrapper for comparison; not the recommended release format.

**Extract the portable ZIP completely** before running `biblioteca.exe`. Keep its support files in place.

## Local build

Use an MSYS2 **UCRT64** terminal with GCC, Make, pkg-config, SDL2, Cairo, Fontconfig and GLEW:

```sh
make -f Makefile.windows -j2
make -f Makefile.windows test
```

The executable requires its dependent DLLs and `assets/` at runtime. GitHub Actions assembles these into a portable distribution.

## Automated verification

The Windows CI workflow:
1. Compiles the actual C++20 source under native Windows, using MSYS2 UCRT64.
2. Executes geometry and navigation self-tests.
3. Bundles its dependent DLLs, assets, shaders and fonts.
4. Runs packaged CLI tests through native Windows CMD with the system-only PATH.
5. Attempts the OpenGL `--desktop-check` renderer, requiring PNG proof frames to count as successful. The graphics step is currently nonblocking on GitHub-hosted runners, since their GPU availability is limited.
6. Checks ZIP integrity and computes SHA-256 checksums.
7. Publishes a downloadable test artifact.

**Native CLI tests passing do not guarantee GPU rendering or interaction on an end-user Windows computer.**

## Physical Windows acceptance test

1. On a normal Windows 11 x64 computer with the graphics driver installed, extract the exact CI ZIP.
2. Launch `biblioteca.exe` via File Explorer. Confirm that the main window stays open.
3. Walk through both rooms, inspect the catalog, click a book, change pages and observe the animated 3D models.
4. Check typography, special characters, cover images, frame rendering and the reference book against Linux screenshots.
5. Close and reopen, test full-screen mode, then try on a second Windows machine.
6. If it fails, use Windows Event Viewer > Windows Logs > Application to collect faulting module and exception code. No PowerShell is required.

Do not label the Windows package stable until it passes this manual acceptance test. In particular, a program that runs under Wine but not native Windows must be diagnosed on Windows itself.

See [DOCUMENTATION.md](DOCUMENTATION.md) for the full cross-platform reference.
