# mapper

A desktop viewer for ESRI Shapefiles, written in C++20 with Qt 6.

Open a `.shp` file, see its geometry drawn, pan and zoom. No GDAL — the
shapefile reader is written in-house.

## Status

Phase 0 (scaffold) is in place. The application builds and opens an empty
window; it does not read shapefiles yet. See `SPECIFICATION.md` §6 for the
phase plan and the [KAN board](https://logikos-sean-mapper.atlassian.net/browse/KAN)
for tracked work.

## Build

Requires Visual Studio 2022, CMake 3.21+, and Qt 6.11 (MSVC 2022 64-bit).
Full setup instructions are in `DEVELOPMENT.md`.

From a Developer Command Prompt for VS 2022, at the repository root:

```bat
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

The executable lands in `build\windows-debug\app\mapper.exe`.

## Layout

```
app/           main.cpp, MainWindow — Qt application shell
view/          MapCanvas — rendering (Qt)
model/         Envelope, Geometry, Layer — plain C++, no Qt
io/            Endian, shapefile parsing — plain C++, no Qt
tests/         Catch2 unit tests
third_party/   Vendored USGS GCTP projection library (Phase 4)
```

`model/` and `io/` must not depend on Qt. That rule is what keeps the parser
headless-testable; see `SPECIFICATION.md` §5.

## Documents

| File | Contents |
|------|----------|
| `SPECIFICATION.md` | Scope, requirements, architecture, phase plan |
| `DEVELOPMENT.md` | Development environment setup (Windows) |
| `JIRA-BACKLOG.md` | Jira issue mapping and dependency links |

## Licence

Not yet determined — gated on the Qt licensing decision
([KAN-40](https://logikos-sean-mapper.atlassian.net/browse/KAN-40)).
