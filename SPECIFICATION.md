# mapper — Specification & Project Plan

**Project:** `mapper` — a C++ desktop application that displays a map from an ESRI Shapefile
**Owner:** Sean Baker (Logikos)
**Status:** Draft v0.1
**Last updated:** 2026-08-07

---

## 1. Purpose

Build a native desktop application that opens an ESRI Shapefile and renders its geometry
as an interactive map, with pan and zoom. The first release is a **viewer**, not an editor
and not a full GIS.

### 1.1 Goals

- Open a `.shp` file (with its sidecar `.shx`, `.dbf`, `.prj`) and draw its features.
- Smooth pan/zoom on files up to ~500k vertices without stutter.
- Cross-platform: Windows, Linux, macOS from one codebase.
- No third-party geospatial dependency — the shapefile reader is written in-house.

### 1.2 Non-goals (this release)

- Editing, digitizing, or writing shapefiles.
- Raster/basemap tiles, WMS, or any network data source.
- Attribute table UI, labelling, thematic styling, print/layout composition.
- Formats other than shapefile (GeoJSON, GeoPackage, KML).

### 1.3 Success criteria

| # | Criterion |
|---|-----------|
| S1 | A user opens `states.shp` via File → Open and sees the polygons drawn, zoomed to extent, within 2 s. |
| S2 | Mouse-wheel zoom and click-drag pan feel continuous (≥ 30 fps) on a 100k-vertex layer. |
| S3 | All three shapefile geometry families (point, polyline, polygon) render correctly, including multipart and polygon holes. |
| S4 | The app reports a clear, non-crashing error for a malformed or unsupported shapefile. |
| S5 | Builds clean from a fresh clone with `cmake -B build && cmake --build build` on Windows and Linux. |

---

## 2. Technology decisions

| Area | Decision | Rationale |
|------|----------|-----------|
| Language | C++20 | Ranges, `std::span`, designated initializers, `<bit>` for byte swapping. |
| GUI toolkit | **Qt 6** (Widgets) | Mature cross-platform desktop UI; `QPainter` gets us to a picture fast, with a documented upgrade path to `QOpenGLWidget` if profiling demands it. |
| Rendering | `QPainter` on a `QWidget` subclass for v1 | Simplest thing that meets S2 at MVP scale. Renderer is behind an interface so an OpenGL backend can be swapped in later. |
| Shapefile I/O | **Hand-written parser** | Explicit decision: no GDAL, no shapelib. The ESRI spec is small and stable; owning the reader means zero external geospatial dependencies and full control over error handling. |
| Projections | **GCTPc** (already vendored at `gctp/gctpc`) | USGS General Cartographic Transformation Package, C source, in-repo. Used only if/when a `.prj` requires reprojection — see §6, Phase 4. Not on the MVP path. |
| Build | CMake ≥ 3.21 | Qt 6 first-class support, presets, cross-platform. |
| Test | Catch2 (or GoogleTest) via CMake `FetchContent` | Unit tests for the parser and geometry/transform math. |
| VCS | Git | Repo root `C:\Projects\mapper`. |

Toolchain installation and per-developer setup are documented separately in
**`DEVELOPMENT.md`** (Windows). That document owns the pinned Qt version and the
build/test command reference; this one owns scope and design.

### 2.1 On the vendored GCTP

`C:\Projects\mapper\gctp` currently holds two USGS distributions — `gctpc` (C version, v c.1.3,
Feb 1996) and `gctp20` — plus their compressed tarballs. This is legacy C from 1996: it will
need a compatibility shim (`extern "C"`, warning suppression, possibly small portability
patches for MSVC) before it links into a modern C++ build. That work is scoped in Phase 4 and
is deliberately kept off the critical path so it cannot block a working viewer.

---

## 3. Functional requirements

### 3.1 File handling

- **FR-1** Open a shapefile via File → Open, drag-and-drop onto the window, and a command-line argument.
- **FR-2** Locate and read sidecars from the same basename: `.shx` (index), `.dbf` (attributes), `.prj` (CRS WKT). Missing `.shx` is recoverable (walk the `.shp` sequentially); missing `.dbf`/`.prj` is non-fatal.
- **FR-3** Show recent files (last 10) in the File menu.
- **FR-4** Report parse errors in a dialog naming the file, byte offset, and reason. Never crash, never render half a layer silently.

### 3.2 Shapefile support

Shape types to support in v1:

| Code | Type | v1 |
|------|------|----|
| 0 | Null | skip silently |
| 1 | Point | yes |
| 3 | PolyLine | yes |
| 5 | Polygon | yes (incl. multipart + holes) |
| 8 | MultiPoint | yes |
| 11/13/15/18 | PointZ / PolyLineZ / PolygonZ / MultiPointZ | read, render X/Y only, ignore Z/M |
| 21/23/25/28 | …M variants | read, render X/Y only |
| 31 | MultiPatch | out of scope — reject with a clear message |

Parser must handle: big-endian file/record headers vs. little-endian geometry, the 100-byte
header, per-record headers, the bounding box, and polygon ring orientation (clockwise = outer,
counter-clockwise = hole, per the ESRI spec).

### 3.3 Map view

- **FR-5** Zoom to full extent on load; View → Zoom to Extent restores it.
- **FR-6** Mouse wheel zooms about the cursor position; click-drag pans; `+`/`-`/arrow keys mirror this.
- **FR-7** Maintain aspect ratio — no anisotropic stretch — on window resize.
- **FR-8** Status bar shows cursor coordinates in layer units, the current scale, and the feature count.
- **FR-9** Default symbology: polygons a light fill with a darker 1 px outline, lines 1.5 px, points 4 px circles. Fixed palette in v1; no per-layer styling UI.

### 3.4 Application shell

- **FR-10** Single main window: menu bar, map canvas (central widget), status bar.
- **FR-11** Remember window geometry and last-opened directory across sessions (`QSettings`).
- **FR-12** Single layer at a time in v1. The data model is a `std::vector<Layer>` from day one so multi-layer is additive, not a rewrite.

---

## 4. Non-functional requirements

- **NFR-1 Performance** — Load a 50 MB shapefile in ≤ 3 s; sustain ≥ 30 fps pan/zoom at 100k vertices. Achieved via a pre-built vertex cache and a zoom-dependent decimation pass, not by re-parsing on every paint.
- **NFR-2 Memory** — Peak RSS ≤ 4× the input `.shp` size.
- **NFR-3 Robustness** — Every read is bounds-checked against the declared file length. Fuzz-tested against truncated and corrupted inputs.
- **NFR-4 Portability** — MSVC 2022, GCC 12+, Clang 15+. No compiler-specific extensions outside the GCTP shim.
- **NFR-5 Code quality** — Warnings-as-errors on the project's own targets (vendored GCTP exempt). `clang-format` enforced.

---

## 5. Architecture

Repository root is `C:\Projects\mapper\source\mapper` (`git@github.com:Sean-Baker-Logikos/mapper.git`).
The two original GCTP `.tar.Z` archives remain outside the repo at `C:\Projects\mapper\gctp\`;
the extracted sources are what we build.

```
┌──────────────────────────────────────────────────────┐
│  app/            main.cpp, MainWindow, menus, actions │
├──────────────────────────────────────────────────────┤
│  view/           MapCanvas (QWidget)                  │
│                  Viewport  (world ⇄ screen transform) │
│                  IRenderer → PainterRenderer          │
├──────────────────────────────────────────────────────┤
│  model/          Layer, Feature, Geometry, Envelope   │
├──────────────────────────────────────────────────────┤
│  io/             ShapefileReader, ShxReader,          │
│                  DbfReader, PrjReader, ByteReader     │
├──────────────────────────────────────────────────────┤
│  proj/  (Ph. 4)  CrsTransform → gctp shim             │
├──────────────────────────────────────────────────────┤
│  third_party/    gctp/  (vendored, unmodified)        │
└──────────────────────────────────────────────────────┘
```

**Dependency rule:** arrows point downward only. `io/` and `model/` know nothing about Qt —
they are plain C++ and unit-testable headless. Qt appears only in `app/` and `view/`.

### 5.1 Key types

```cpp
struct Envelope { double xmin, ymin, xmax, ymax; };

struct Vertex { double x, y; };   // NOT QPointF — see note below

struct Part  { std::uint32_t firstVertex, vertexCount; };   // index into Layer::vertices

struct Feature {
    ShapeType   type;
    Envelope    bbox;
    std::uint32_t firstPart, partCount;                     // index into Layer::parts
    std::uint32_t recordNumber;
};

struct Layer {
    std::string             name;
    ShapeType               type;
    Envelope                extent;
    std::vector<Feature>    features;
    std::vector<Part>       parts;
    std::vector<Vertex>     vertices;   // one flat, contiguous buffer
    std::optional<Crs>      crs;
};
```

Geometry lives in **one flat vertex buffer per layer** with features/parts as index ranges —
not a tree of `vector<vector<Point>>`. This is the single most important performance decision
in the design: it keeps the whole layer cache-friendly and makes the renderer a tight loop.

> **Correction (2026-08-07).** An earlier draft of this section declared `vertices` as
> `std::vector<QPointF>`, which contradicts the dependency rule stated directly above it —
> `QPointF` is QtCore, and `model/` must not depend on Qt. The buffer holds a plain
> `Vertex { double x, y; }`; conversion to Qt types happens in the renderer, at the point
> where coordinates are already being transformed to screen space anyway.

### 5.2 Viewport

`Viewport` owns the world→screen affine transform: a center point in world coordinates, a
scale (world units per pixel), and the widget size. All pan/zoom operations mutate these three
and nothing else. Provides `toScreen(worldPt)`, `toWorld(screenPt)`, `visibleEnvelope()`.

### 5.3 Render pipeline

1. Query the layer's features against `viewport.visibleEnvelope()` (linear scan in v1; spatial index in Phase 3).
2. For each visible feature, walk its parts, transform vertices to screen space.
3. Decimate: skip vertices closer than 0.75 px to the previously emitted one.
4. Emit `QPainterPath` per part; batch into one `drawPath`/`drawPolyline` call per feature.

---

## 6. Phased plan

Each phase ends with something runnable. Estimates assume one developer.

### Phase 0 — Scaffold *(0.5 day)* — **implemented, build unverified**

- [x] Repo cloned; `.gitignore`, `.clang-format`, `README.md`. *(`LICENSE` still outstanding — gated on the Qt licensing decision.)*
- [x] CMake skeleton with `app/`, `view/`, `model/`, `io/`, `tests/` targets; `CMakePresets.json`.
- [x] Qt 6 discovered via `find_package(Qt6 COMPONENTS Widgets)`.
- [x] `MainWindow` with a `MapCanvas` central widget, File and View menus, status bar.
- [x] Catch2 v3.5.2 wired via `FetchContent`; 11 passing tests over `Envelope` and `Endian`.
- **Exit:** `cmake --build build && ctest` is green on Windows and Linux.

**Verification status.** `model/`, `io/` and `tests/` were configured, built and run on
GCC 13 / CMake 3.28: 11/11 tests pass with zero warnings under `-Wall -Wextra -Wpedantic
-Werror`. The Qt-dependent targets (`view/`, `app/`) have **not** been compiled — no Qt
toolchain was available to the agent. First MSVC build is the outstanding acceptance step.

### Phase 1 — Shapefile reader *(3 days)*

- [ ] `ByteReader` — bounds-checked, endian-aware primitive reads.
- [ ] `.shp` main header (100 bytes): magic `9994`, file length, version, shape type, bbox.
- [ ] Record loop: record number, content length, shape type per record.
- [ ] Point, MultiPoint, PolyLine, Polygon geometry decode into the flat buffer.
- [ ] Z/M variants: parse, discard Z/M.
- [ ] `.shx` reader for direct record offsets; fall back to sequential walk when absent.
- [ ] Error type carrying file, byte offset, and reason.
- [ ] Unit tests: hand-built byte fixtures for each shape type; truncated-file cases; a real-world sample (e.g. Natural Earth admin-0).
- **Exit:** a headless CLI test harness prints feature count and extent for any sample shapefile.

### Phase 2 — Render & interact *(3 days)*

- [ ] `Viewport` + transform math, with unit tests (round-trip `toWorld(toScreen(p)) == p`).
- [ ] `MapCanvas::paintEvent` → `PainterRenderer`.
- [ ] Polygon fill with holes via `QPainterPath` even-odd fill.
- [ ] Zoom-to-extent on load; aspect-ratio-preserving resize.
- [ ] Wheel zoom about cursor, drag pan, keyboard nav.
- [ ] Status bar: coordinates, scale, feature count.
- [ ] File → Open, drag-and-drop, CLI argument, recent files.
- **Exit:** S1, S3 met — the app is a usable viewer.

### Phase 3 — Performance & polish *(2 days)*

- [ ] Profile against a 50 MB / 500k-vertex layer.
- [ ] Vertex decimation by screen-space distance.
- [ ] Spatial index (uniform grid or packed R-tree) over feature bboxes for visibility culling.
- [ ] `.dbf` reader — parsed and held in memory, surfaced only as feature count for now.
- [ ] Error dialogs, busy cursor / progress for large loads.
- [ ] `QSettings` persistence of window geometry and last directory.
- **Exit:** S2, S4, NFR-1 met.

### Phase 4 — Projections *(2 days, optional for v1)*

- [ ] `.prj` WKT reader — enough to identify projection, datum, and parameters.
- [ ] Build the vendored GCTPc as a static library; MSVC portability shim; `extern "C"` wrapper.
- [ ] `CrsTransform` interface over `gctp()` forward/inverse calls.
- [ ] Reproject layer vertices to a display CRS on load; validate against the GCTP test vectors in `gctp/gctpc/test`.
- **Exit:** a UTM shapefile and a geographic shapefile display in a common frame.

### Phase 5 — Release *(1 day)*

- [ ] `windeployqt` / `linuxdeployqt` packaging.
- [ ] GitHub Actions CI: build + test on Windows and Linux.
- [ ] README with build instructions and screenshots.
- **Exit:** S5 met; a downloadable build exists.

**Total: ~11.5 working days** (~9.5 excluding Phase 4).

Tracked in Jira as project **KAN — Mapper Dev** on `logikos-sean-mapper.atlassian.net`:
one Epic per phase (KAN-4 … KAN-9), one Story per checklist item. Key mapping and field
notes are in `JIRA-BACKLOG.md`. When a phase changes here, update the corresponding Epic.

---

## 7. Risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Hand-written parser hits spec edge cases (ring orientation, degenerate parts, non-conformant writers) | Med | Test against shapefiles from several producers — ArcGIS, QGIS, Natural Earth, TIGER. Treat non-conformance as warn-and-continue, not fail. |
| `QPainter` too slow at target scale | Med | `IRenderer` interface is in place from Phase 2; OpenGL backend is a contained swap, not a redesign. |
| 1996-era GCTP won't build clean under MSVC/C++20 | Med | Isolated in Phase 4 and off the MVP path. Fallback: implement UTM/Albers/Lambert-Conformal-Conic directly — a few hundred lines covers most US data. |
| `.dbf` encoding ambiguity (no reliable codepage) | Low | Assume Latin-1, honour the language-driver byte when present, expose an override later. |
| Scope creep toward "a small QGIS" | High | §1.2 non-goals are binding for v1. New asks go to a v2 backlog. |

---

## 8. Open questions

1. **Qt licensing** — open-source LGPLv3 (requires dynamic linking / relink ability) or a commercial licence? Affects packaging in Phase 5.
2. **Minimum Qt version** — resolved to a working default of **Qt 6.11** (MSVC 2022, Windows 10 1809+ / 11); see `DEVELOPMENT.md` §3.1. Revisit if Q1 lands on a commercial licence, in which case 6.8 LTS is the better pin.
3. **Is Phase 4 in v1?** If Logikos data is all in one CRS, projections can defer to v2 and cut ~2 days.
4. **macOS support** — build and test there, or Windows + Linux only for v1?
5. **Sample data** — is there a representative shapefile from real work to test and benchmark against?

---

## 9. Maintenance

This document is the source of truth for scope. Update it in the same change that alters
behaviour — when a phase completes, when a decision in §2 changes, or when an open question
in §8 is answered. Amendments are logged below.

| Date | Change |
|------|--------|
| 2026-08-07 | Initial draft. |
| 2026-08-07 | Added `DEVELOPMENT.md` cross-reference (§2); resolved §8 Q2 to a Qt 6.11 working default. |
| 2026-08-07 | Plan exported to Jira project KAN; §6 cross-referenced to `JIRA-BACKLOG.md`. Open questions §8 Q1, Q3, Q4, Q5 filed as KAN-40, KAN-42, KAN-44, KAN-46. |
| 2026-08-07 | Repo root established at `source/mapper`; docs and GCTP moved under version control. Phase 0 implemented. §5.1 corrected: `QPointF` → `Vertex`, which violated the §5 no-Qt-in-model rule. |
