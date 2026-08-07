# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

`mapper` is a C++20 / Qt 6 desktop viewer for ESRI Shapefiles. It parses `.shp`
geometry with an in-house reader (no GDAL/shapelib) and renders it with pan/zoom
via `QPainter`. Current state: Phase 0 (scaffold) — the app builds and shows an
empty window; the shapefile reader does not exist yet.

Full scope, requirements, and the phase plan live in `SPECIFICATION.md`. Jira
issue mapping (project `KAN`) is in `JIRA-BACKLOG.md`. Windows dev-environment
setup is in `DEVELOPMENT.md`. **Treat these three files as living documents: when
a change alters scope, architecture, or phase status, update the relevant file
in the same change.**

**Repository root is `C:\Projects\mapper\source\mapper`** — not `C:\Projects\mapper`.
The parent directory also holds `gctp/` (original vendored `.tar.Z` archives, not
part of the build; the extracted, buildable copy lives in `third_party/gctp/`
inside the repo).

## Build, test, run

Requires a Developer Command Prompt for VS 2022 (plain `cmd`/PowerShell won't
find `cl.exe`), CMake 3.21+, and Qt 6 with `CMAKE_PREFIX_PATH` pointed at the
`msvc2022_64` install. Full setup: `DEVELOPMENT.md`.

```bat
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

Other presets: `windows-release` (optimized, for performance work),
`windows-relwithdebinfo` (optimized with symbols, for profiling). Never link a
Debug build against Release Qt libraries or vice versa.

Executable: `build\windows-debug\app\mapper.exe`. Run against a shapefile with
`mapper.exe <path.shp>`.

Run a single test binary directly for a tighter loop than `ctest`:

```bat
build\windows-debug\tests\mapper_tests.exe
build\windows-debug\tests\mapper_tests.exe "[envelope]"
```

Tests are Catch2 v3 (fetched via CMake `FetchContent`, not vendored). Tags are
whatever each `TEST_CASE` declares.

Warnings-as-errors (`/W4 /WX` MSVC, `-Wall -Wextra -Wpedantic -Werror` else) is
on for all `mapper_*` targets via the `mapper_warnings` interface target in the
root `CMakeLists.txt`. It is not applied to `third_party/`. A clean build has
zero warnings — treat any new warning as a build failure to fix, not suppress.

`clang-format` (LLVM-based, 100 col, 4-space indent) governs formatting; see
`.clang-format`. Include order is grouped and regrouped: local (`"..."`) →
current-dir angle-bracket → Qt (`<Q...>`) → other angle-bracket.

## Architecture

Layered, strictly downward-dependent, enforced by which CMake targets are
allowed to link Qt:

```
app/     main.cpp, MainWindow — Qt application shell, menus, actions
view/    MapCanvas (QWidget), Viewport, IRenderer → PainterRenderer
model/   Layer, Feature, Geometry, Envelope — plain C++, NO Qt
io/      ShapefileReader, ShxReader, DbfReader, PrjReader, ByteReader — plain C++, NO Qt
proj/    (Phase 4) CrsTransform → gctp shim
third_party/  Vendored USGS GCTP projection library (Phase 4), unmodified
```

**`model/` and `io/` must never depend on Qt.** This is what keeps the shapefile
parser headless-testable. It has already been violated and corrected once (an
earlier draft typed `Layer::vertices` as `QPointF`; see `SPECIFICATION.md` §5.1
"Correction") — watch for the same mistake creeping back in via a convenient Qt
type. Qt appears only in `view/` and `app/`; those link `Qt6::Widgets`, `model/`
and `io/` do not.

Each directory is one static-library CMake target (`mapper_model`, `mapper_io`,
`mapper_view`) linked into the `mapper` executable (`app/`). Namespaces mirror
directories: `mapper::model`, `mapper::io`, etc.

**Geometry storage:** one flat, contiguous `std::vector<Vertex>` per `Layer`,
with `Feature`/`Part` as index ranges into it — not a tree of nested vectors.
This is the load-bearing performance decision (cache-friendly, tight-loop
renderer); do not restructure it toward a per-feature/per-part container
without re-checking `SPECIFICATION.md` §5.1 and the NFR-1 performance target.

**Shapefile byte order:** file/record headers are big-endian, geometry payloads
are little-endian (ESRI spec). `io/Endian.h` exists to make every conversion
explicit at the call site rather than assumed.

**Render pipeline** (`SPECIFICATION.md` §5.3): cull features against
`Viewport::visibleEnvelope()`, transform vertices to screen space, decimate
points closer than 0.75px, emit one `QPainterPath`/`drawPath` call per feature.

## Constraints that shape implementation choices

- No GDAL, shapelib, or other third-party geospatial dependency — the shapefile
  reader is intentionally hand-written against the ESRI spec.
- Every `io/` read must be bounds-checked against the declared file length
  (NFR-3); malformed input must produce a `ParseError` (file, byte offset,
  reason), never a crash or a silently half-rendered layer.
- Cross-platform target (NFR-4): MSVC 2022, GCC 12+, Clang 15+. No
  compiler-specific extensions outside the GCTP shim in `third_party/`.
- `third_party/gctp/` is 1996-era K&R C, out of scope until Phase 4. It builds
  as its own target, exempt from `mapper_warnings`; any patch to the vendored
  source must land in `third_party/gctp/patches/` with a note, not edited in
  place.
