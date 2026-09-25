# mapper - Software Specification (reconstructed)

## Provenance

This document was reconstructed on 2026-08-14 from Jira project KAN ("Mapper Dev", 46 issues, KAN-4 through KAN-49) plus the working copy at `C:\Work\Training\mapper`. The original `SPECIFICATION.md` is not present in the repository and not present in Confluence. Every requirement, phase item and section number below carries the originating Jira key. Items marked **UNVERIFIED** are inferred from surrounding evidence rather than quoted from a citation, and require Sean's confirmation before they are treated as authoritative.

Section numbering is constrained by existing citations: source comments and Jira descriptions reference section 5, 5.1, 5.2, 5.3, section 6, section 7 and section 8. Numbering below preserves those references.

**Numbering conflict requiring confirmation (UNVERIFIED resolution):** the reconstruction request specified section 7 as "Divergences from the current tree", but KAN-48 cites "SPECIFICATION.md section 7 (risk register)". Section 7 is therefore retained as the risk register and divergences are placed at section 9. Confirm the intended assignment.

Identifier coverage recovered from Jira: FR-1 through FR-9 and FR-11; NFR-1, NFR-3, NFR-4; S1 through S5. FR-10 and NFR-2 are referenced by no issue and are reconstructed as gaps, not invented.

---

## 1. Scope

### 1.1 Goals (UNVERIFIED - section number and content inferred)

Desktop viewer for ESRI Shapefiles. C++20, Qt 6 Widgets, CMake. Hand-written shapefile parser with no GDAL and no shapelib dependency (KAN-5). Optional coordinate reference system (CRS) transformation via the vendored USGS General Cartographic Transformation Package (GCTP) (KAN-8).

Product description recorded in the root `CMakeLists.txt`: "Desktop viewer for ESRI Shapefiles". Version 0.1.0.

### 1.2 Non-goals

- No attribute table user interface in v1. The `.dbf` attribute table is parsed and held in memory only (KAN-20, which cites "that is explicitly a non-goal (SPECIFICATION.md section 1.2)").
- Shape type 31 (MultiPatch) is out of scope and must be rejected with a clear message rather than silently mis-parsed (KAN-31). Corroborated by `model/Geometry.h`: `MultiPatch = 31, ///< Out of scope; rejected by the reader.`
- Z and M values are read for record-loop synchronisation but discarded; X/Y only is rendered (KAN-27).

---

## 2. Functional requirements (UNVERIFIED - section number inferred; requirement content cited)

| ID | Requirement | Source |
|----|-------------|--------|
| FR-1 | Open a shapefile through File > Open. | KAN-49 |
| FR-2 | Missing sidecar files degrade gracefully. A missing `.prj` is non-fatal. A missing `.shx` is recoverable by walking the `.shp` sequentially. | KAN-26, KAN-29 |
| FR-3 | Additional entry paths to opening a shapefile: drag-and-drop, command-line argument, and a recent-files list of the last 10. | KAN-49 |
| FR-4 | Error type carrying file, byte offset, and reason (`ParseError`), surfaced to the operator. Never crash and never render a half-parsed layer silently. | KAN-22, KAN-31 |
| FR-5 | Zoom to extent on load. | KAN-43 |
| FR-6 | Mouse wheel zooms about the cursor position, not the view center. Click-drag pans. `+`/`-`/arrow keys mirror both. | KAN-45 |
| FR-7 | Preserve aspect ratio on resize. No anisotropic stretch when the window is resized. | KAN-43 |
| FR-8 | Status bar reporting cursor position in layer units, current scale, and total feature count. | KAN-47 |
| FR-9 | Default symbology: light fill, 1 px darker outline. | KAN-41 |
| FR-10 | **UNVERIFIED - no Jira issue cites FR-10. Content unrecoverable. Requires Sean's confirmation.** | none |
| FR-11 | Persist window geometry and last-used directory via `QSettings`. | KAN-24 |

Supporting functional detail cited by issues but not bound to an FR identifier:

- Validate `.shp` magic number 9994; read file length, version, shape type, bounding box; reject files whose declared length disagrees with the actual size (KAN-19).
- Walk record headers (record number, content length, big-endian) and dispatch on the per-record shape type. Shape type 0 (Null) is skipped silently (KAN-21).
- Supported shape types: 1 (Point), 3 (PolyLine), 5 (Polygon), 8 (MultiPoint), and the Z and M variants 11/13/15/18 and 21/23/25/28 (KAN-23, KAN-25, KAN-27).
- Polygon ring orientation per the ESRI specification: clockwise rings are outer, counter-clockwise are holes. Unexpected orientation is warn-and-continue, not fail, because non-conformant producers are common (KAN-25).
- `.dbf` parsing assumes Latin-1 encoding, honouring the language-driver byte when present (KAN-20).
- `.prj` parsing extracts projection, datum, and parameters from Well-Known Text (WKT) (KAN-26).

---

## 3. Non-functional requirements (UNVERIFIED - section number inferred; requirement content cited)

| ID | Requirement | Source |
|----|-------------|--------|
| NFR-1 | Load a 50 MB shapefile in <= 3 s. | KAN-7 |
| NFR-2 | **UNVERIFIED - no Jira issue cites NFR-2. Content unrecoverable. Requires Sean's confirmation.** | none |
| NFR-3 | Every read bounds-checked against the declared file length. | KAN-17 |
| NFR-4 | Portability. macOS is named as a portability target. | KAN-44 |

No issue references an NFR identifier above NFR-4. **UNVERIFIED:** whether NFR-5 or higher existed in the original document.

---

## 4. Success criteria (UNVERIFIED - section number inferred; criterion text quoted)

| ID | Criterion | Source |
|----|-----------|--------|
| S1 | Open a shapefile via File > Open and see polygons drawn, zoomed to extent, within 2 s. | KAN-6, KAN-49 |
| S2 | >= 30 fps pan/zoom on a 100k-vertex layer. | KAN-7, KAN-24 |
| S3 | Point, polyline, and polygon all render correctly, including multipart geometry and polygon holes. | KAN-6, KAN-49 |
| S4 | Clear, non-crashing error for malformed or unsupported shapefiles. | KAN-7, KAN-22, KAN-24 |
| S5 | Builds clean from a fresh clone on Windows and Linux, and a downloadable build exists. | KAN-9, KAN-38 |

---

## 5. Architecture: modules and dependency rules

Module decomposition: `app/`, `view/`, `model/`, `io/`, `tests/` (KAN-11).

Dependency rule: arrows point downward only. `model/` and `io/` are plain C++ and MUST NOT link Qt. Only `view/` and `app/` may (KAN-11). This rule is quoted in the root `CMakeLists.txt` and re-cited in `model/Geometry.h`:

> Deliberately NOT QPointF: model/ and io/ must not depend on Qt (SPECIFICATION.md section 5). Conversion happens in the renderer.

Consequences stated in Jira:

- The shapefile parser is plain C++ with no Qt dependency and is unit-testable headless (KAN-5, KAN-17).
- Legacy C from GCTP is isolated behind a C++ interface so no legacy C leaks into the rest of the codebase (KAN-30).

**UNVERIFIED:** the module layering diagram and any further ordering constraints among `app/`, `view/`, `model/`, `io/` beyond "downward only".

### 5.1 Data model: flat vertex buffer

Vertices live in ONE flat contiguous buffer per layer. Features and parts are index ranges into it. Geometry decodes directly into this buffer, not into a nested vector-of-vectors (KAN-5, KAN-23).

`model/Layer.h` records this as authoritative:

> Vertices live in ONE flat contiguous buffer; features and parts are index ranges into it. This is the central performance decision in the design -- see SPECIFICATION.md section 5.1. Do not replace with nested vectors.

Types as built (`model/Geometry.h`, `model/Layer.h`):

- `Vertex { double x, y }` - world-coordinate vertex, deliberately not `QPointF`.
- `Part { uint32 firstVertex, vertexCount }` - one contiguous run of vertices: one ring or one line part.
- `Feature { ShapeType type, Envelope bbox, uint32 firstPart, partCount, recordNumber }`.
- `Layer { string name, ShapeType type, Envelope extent, vector<Feature>, vector<Part>, vector<Vertex> }`.
- `Envelope { double xmin, ymin, xmax, ymax }` with `empty()`, `isEmpty()`, `width()`, `height()`, `expand()`, `intersects()`, `contains()`.

### 5.2 Viewport transform

`Viewport` owns exactly three values: center point (world), scale (world units per pixel), and widget size. It provides `toScreen`, `toWorld`, and `visibleEnvelope`. All pan and zoom operations mutate only those three values (KAN-35).

Correctness obligation: `toWorld(toScreen(p)) == p` within tolerance, including edge cases at extreme zoom levels (KAN-37).

### 5.3 Render pipeline and IRenderer

Rendering is placed behind an `IRenderer` interface so an OpenGL backend can be swapped in later without redesign. `MapCanvas` is a `QWidget` subclass whose `paintEvent` drives the pipeline; the concrete v1 backend is `PainterRenderer` (KAN-39).

Pipeline steps, as recoverable:

1. **UNVERIFIED** - query visible features. Inferred from KAN-18, which describes replacing "the Phase 2 linear scan in `visibleEnvelope` queries" with a spatial index.
2. **UNVERIFIED** - transform world coordinates to screen coordinates. Inferred; KAN-16 refers to "the transform pass".
3. Screen-space vertex decimation: skip vertices closer than 0.75 px to the previously emitted one during the transform pass. Cited as "SPECIFICATION.md section 5.3 step 3" (KAN-16).
4. **UNVERIFIED** - emit primitives through `IRenderer`. Inferred from KAN-39.

Polygon fill uses `QPainterPath` with the even-odd fill rule so inner rings render as holes (KAN-41).

---

## 6. Phase plan

Total estimate across phases as stated by the epics: 11.5 d (0.5 + 3 + 3 + 2 + 2 + 1).

### Phase 0: Scaffold (KAN-4)

**Estimate:** 0.5 d. **Exit criteria:** `cmake --build build && ctest` is green on Windows (KAN-4, KAN-15).

Scope statement (KAN-4): establishes the repository, CMake structure, Qt 6 discovery, and the test harness. Produces no user-visible behaviour.

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-10 | 0.1 d | Initialize Git repository at `C:\Projects\mapper` with `.gitignore` (build output, `data/samples/`, IDE dirs), `.clang-format`, `LICENSE`, and a starter `README.md`. |
| KAN-11 | 0.2 d | Top-level `CMakeLists.txt` plus subdirectory targets for `app/`, `view/`, `model/`, `io/`, `tests/`. Enforce the section 5 dependency rule. |
| KAN-12 | 0.1 d | `CMakePresets.json` with three Windows Ninja presets: `windows-debug`, `windows-release`, `windows-relwithdebinfo`. Pin the Qt path so every developer and CI agent builds the same version. Cites DEVELOPMENT.md section 4.2. |
| KAN-13 | 0.1 d | `find_package(Qt6 REQUIRED COMPONENTS Widgets)` plus `qt_standard_project_setup()`. Produce a `mapper.exe` that opens an empty main window and exits cleanly. |
| KAN-15 | 0.1 d | Catch2 via CMake `FetchContent`, registered with `ctest`. One trivial passing test proves the harness. |

**UNVERIFIED:** item estimates sum to 0.6 d against the stated phase estimate of 0.5 d.

### Phase 1: Shapefile reader (KAN-5)

**Estimate:** 3 d. **Blocked by:** Phase 0. **Exit criteria:** a headless CLI harness prints feature count and extent for any sample shapefile (KAN-5, KAN-33).

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-17 | 0.3 d | `ByteReader`: primitive read helpers for int32/double in both byte orders, every read bounds-checked (NFR-3). No Qt dependency. |
| KAN-19 | 0.3 d | Parse the `.shp` 100-byte main header; validate magic 9994; reject declared-length/actual-size mismatch. |
| KAN-21 | 0.3 d | Record loop with per-record shape type dispatch. Null (type 0) skipped silently. |
| KAN-23 | 0.3 d | Decode Point (1) and MultiPoint (8) into the section 5.1 flat vertex buffer. |
| KAN-25 | 0.6 d | Decode PolyLine (3) and Polygon (5): parts array, multipart geometry, ring orientation. Largest single story in Phase 1. Unexpected orientation is warn-and-continue. |
| KAN-27 | 0.3 d | Parse Z and M variants (11/13/15/18, 21/23/25/28); read full records to stay in sync, render X/Y only. |
| KAN-29 | 0.3 d | `.shx` reader for direct record offsets, with sequential-walk fallback (FR-2). |
| KAN-31 | 0.2 d | `ParseError` carrying file, byte offset, reason (FR-4). Reject MultiPatch (31) cleanly. |
| KAN-33 | 0.4 d | Unit-test with hand-built byte fixtures per supported shape type, plus truncated and corrupted inputs, plus a real-world sample (Natural Earth admin-0). |

### Phase 2: Render and interact (KAN-6)

**Estimate:** 3 d. **Blocked by:** Phase 1. **Exit criteria:** success criteria S1 and S3 met; the application is a usable viewer (KAN-6, KAN-49).

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-35 | 0.4 d | `Viewport` world/screen transform per section 5.2. |
| KAN-37 | 0.2 d | Unit-test `Viewport`, including round-trip `toWorld(toScreen(p))` and extreme-zoom edge cases. |
| KAN-39 | 0.6 d | `MapCanvas::paintEvent` driving the section 5.3 pipeline through `IRenderer` to `PainterRenderer`. |
| KAN-41 | 0.4 d | Polygon fills with holes via `QPainterPath` even-odd fill. Default symbology per FR-9. |
| KAN-43 | 0.3 d | Zoom to extent on load; preserve aspect ratio on resize (FR-5, FR-7). |
| KAN-45 | 0.5 d | Wheel zoom about the cursor, drag pan, keyboard navigation (FR-6). |
| KAN-47 | 0.2 d | Status bar: coordinates, scale, feature count (FR-8). |
| KAN-49 | 0.4 d | File Open, drag-and-drop, CLI argument, recent files (FR-1, FR-3). |

### Phase 3: Performance and polish (KAN-7)

**Estimate:** 2 d. **Blocked by:** Phase 2. **Exit criteria:** success criteria S2 and S4 and NFR-1 met (KAN-7, KAN-24).

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-14 | 0.3 d | Profile against a 50 MB / 500k-vertex layer and record a baseline. Release configuration only; Debug numbers are meaningless for this work. |
| KAN-16 | 0.4 d | Screen-space vertex decimation at 0.75 px (section 5.3 step 3). |
| KAN-18 | 0.5 d | Spatial index over feature bounding boxes for culling: uniform grid or packed R-tree, replacing the Phase 2 linear scan in `visibleEnvelope` queries. |
| KAN-20 | 0.4 d | `.dbf` reader surfacing feature count only. dBASE III attribute table held in memory. Latin-1 assumed, language-driver byte honoured. No attribute table UI (section 1.2). |
| KAN-22 | 0.2 d | Error dialogs, busy cursor, progress for large loads. Surface `ParseError` (FR-4, S4). |
| KAN-24 | 0.2 d | Persist window geometry and last directory via `QSettings` (FR-11). |

### Phase 4: Projections (KAN-8) - optional for v1

**Estimate:** 2 d. **Blocked by:** Phase 2. **Gated on** the decision "Is Phase 4 in v1?" (KAN-42); deferring cuts approximately 2 days. **Exit criteria:** a UTM shapefile and a geographic shapefile display in a common frame (KAN-8, KAN-32).

Basis (KAN-8): uses the USGS GCTP package already vendored at `gctp/` (GCTPc v c.1.3, Feb 1996). K&R-era C that has never been compiled by MSVC; the highest-uncertainty estimate in the plan. See the spike KAN-48.

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-26 | 0.4 d | `.prj` WKT reader: projection, datum, parameters. Missing `.prj` non-fatal (FR-2). |
| KAN-28 | 0.6 d | Build vendored GCTPc as a static library with an MSVC shim. Expect `_CRT_SECURE_NO_WARNINGS`, `/W0` on this target only, and small patches for missing `<unistd.h>` and `M_PI`. Warnings-as-errors disabled for this target only. Keep vendored sources unmodified where possible; any required patch goes in `third_party/gctp/patches/` with a note explaining why. Cites DEVELOPMENT.md section 7. |
| KAN-30 | 0.5 d | `CrsTransform`: C++ interface wrapping the `gctp()` entry point via `extern "C"`, isolating all legacy C. |
| KAN-32 | 0.5 d | Reproject layer vertices to a display CRS at load time. Wire the USGS validation data in `gctp/gctpc/test/` into `ctest` as the acceptance criterion for the transform layer. |

### Phase 5: Release (KAN-9)

**Estimate:** 1 d. **Blocked by:** Phase 3. **Exit criteria:** success criterion S5 met (KAN-9, KAN-38).

Packaging approach depends on the Qt licensing decision (KAN-40): LGPLv3 requires dynamic linking and the ability for a user to relink against their own Qt (KAN-9, KAN-34).

| Item | Estimate | Detail |
|------|----------|--------|
| KAN-34 | 0.3 d | Package for Windows with `windeployqt`. Approach gated on the Qt licensing decision. |
| KAN-36 | 0.5 d | GitHub Actions CI building and testing on Windows and Linux. Headless Qt install via `jurplel/install-qt-action` or `aqtinstall`; the GUI online installer documented in DEVELOPMENT.md section 3.2 is not usable in CI. |
| KAN-38 | 0.2 d | README with build instructions and screenshots. |

---

## 7. Risk register

Cited by KAN-48 as "SPECIFICATION.md section 7 (risk register)".

| Risk | Mitigation | Source |
|------|------------|--------|
| GCTPc (1996-era K&R C, targeting DG/UX, SunOS, IRIX) may not compile under MSVC. Highest-uncertainty estimate in the plan; Phase 4's 2 d estimate assumes a compatibility shim suffices. | Spike KAN-48, timeboxed to 1 hour: attempt a minimal MSVC compile of `gctp/gctpc/source/` and report what breaks. Fallback: implement UTM, Albers, and Lambert Conformal Conic directly; a few hundred lines covers most US data. | KAN-48, KAN-8, KAN-28 |
| Non-conformant shapefile producers emit unexpected polygon ring orientation. | Treat unexpected orientation as warn-and-continue, not fail. | KAN-25 |
| Curated public samples do not surface the parser edge cases real production data does. | Source a representative production shapefile (KAN-46). Natural Earth and TIGER/Line cover public-data cases; a Logikos file covers producer quirks. | KAN-46, KAN-33 |
| Qt licensing choice constrains both packaging and the pinned Qt version. | Resolve KAN-40 before Phase 5. | KAN-40, KAN-9, KAN-34 |
| macOS is an NFR-4 portability target but no macOS build has been attempted, and DEVELOPMENT.md covers Windows only. | Resolve KAN-44 before fixing Phase 5 CI scope. | KAN-44 |

**UNVERIFIED:** whether section 7 in the original contained additional risks, or probability/impact scoring.

---

## 8. Open decisions and questions

Question numbering is fixed by Jira citations.

### Q1. Qt licensing: LGPLv3 or commercial (KAN-40)

Blocks Phase 5 packaging (KAN-9) and determines the pinned Qt version.

LGPLv3 requires dynamic linking and the ability for a user to relink against their own Qt. A commercial licence removes that constraint and makes Qt 6.8 LTS (supported to Oct 2029) the better pin. Open-source use makes Qt 6.11 the better pin, because Qt's LTS designation confers nothing on open-source users; OSS support for 6.8 ended April 2025.

### Q2. **UNVERIFIED - no Jira issue cites section 8 Q2. Content unrecoverable. Requires Sean's confirmation.**

### Q3. Is Phase 4 (projections) in v1? (KAN-42)

Blocks Phase 4 (KAN-8). Deferring to v2 cuts approximately 2 days from the plan. If all Logikos data is in a single CRS, reprojection can wait.

### Q4. macOS support for v1 (KAN-44)

Affects Phase 5 CI scope (KAN-9). NFR-4 names macOS as a portability target, but DEVELOPMENT.md currently covers Windows only and no macOS build has been attempted.

### Q5. Source a representative production shapefile for testing (KAN-46)

Feeds Phase 1 (KAN-5) test coverage and the Phase 3 (KAN-7) performance baseline.

---

## 8a. Companion document: DEVELOPMENT.md

Jira issues cite a second document, `DEVELOPMENT.md`. It does not exist in the staged tree (see section 9). Cited sections:

| DEVELOPMENT.md section | Subject | Citing issue |
|------------------------|---------|--------------|
| 3.2 | Qt installation via the GUI online installer; explicitly not usable in CI. | KAN-36 |
| 4.2 | CMake presets and Qt path pinning. | KAN-12 |
| 7 | Building the vendored GCTP tree. | KAN-28 |

KAN-44 additionally states that DEVELOPMENT.md currently covers Windows only.

**UNVERIFIED:** section 8a is a reconstruction artifact. The original document's location for this content is unknown.

---

## 9. Divergences from the current tree

Comparison of the citations above against the working copy at `C:\Work\Training\mapper`, as of 2026-08-14. Items marked RESOLVED were corrected in the build-fix pass of 2026-08-14; items marked OPEN remain outstanding.

### 9.1 Repository location and baseline files

1. **Repository path.** `out/build/x64-debug/CMakeCache.txt` records `mapper_SOURCE_DIR:STATIC=C:/Work/Training/mapper` and `CMAKE_HOME_DIRECTORY:INTERNAL=C:/Work/Training/mapper`. KAN-10 specifies `C:\Projects\mapper`.
2. **Baseline config files absent.** No `.gitignore`, `.clang-format`, `LICENSE`, or `README.md` in the tree. KAN-10 requires all four.
3. **No Git repository.** OPEN. No `.git` directory observed. KAN-10 requires an initialized repository. **UNVERIFIED:** confirm against the working copy.
4. **DEVELOPMENT.md absent.** Cited by KAN-12 (section 4.2), KAN-28 (section 7), KAN-36 (section 3.2) and KAN-44. Not present.
5. **SPECIFICATION.md absent.** Cited by 44 of 46 issues and by source comments in `CMakeLists.txt`, `model/Geometry.h`, `model/Layer.h`. Not present.

### 9.2 Build configuration

6. **Preset names.** OPEN. `CMakePresets.json` defines `windows-base` (hidden), `x64-debug`, `x64-relwithdebinfo`, `x64-release`. KAN-12 specifies `windows-debug`, `windows-release`, `windows-relwithdebinfo`. The RelWithDebInfo preset was added and the two x86 presets were removed (no Qt 6 x86 kit is published), but the naming scheme still diverges. Renaming invalidates existing build directories; deferred pending confirmation.
7. **Toolchain mismatch.** RESOLVED in CMake, blocked on a host prerequisite. Root `CMakeLists.txt` previously hardcoded `set(Qt6_DIR "C:/Qt/6.11.1/mingw_64/lib/cmake/Qt6")`, a MinGW kit, while every preset sets `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` to `cl.exe` (MSVC); `link.exe` cannot consume GCC import libraries, and the cache recorded `Qt6_DIR:PATH=Qt6_DIR-NOTFOUND`. The hardcoded pin was removed; `CMAKE_PREFIX_PATH` is now pinned in `CMakePresets.json` per KAN-12. **Host prerequisite:** `C:\Qt\6.11.1` contains only `mingw_64`; the `msvc2022_64` kit must be installed via the Qt Maintenance Tool.
8. **Undefined `mapper_warnings` target.** RESOLVED. Five subdirectories linked `mapper_warnings` with no target defined, deferring failure to link time. Now defined in the root as an INTERFACE library carrying `/W4 /permissive- /EHsc` (MSVC) and `-Wall -Wextra -Wpedantic` otherwise. Warnings-as-errors is not enabled; KAN-28 implies a per-target opt-out will be required for the GCTP sources.
9. **C++20 not applied project-wide.** RESOLVED. `CMAKE_CXX_STANDARD 20`, `CMAKE_CXX_STANDARD_REQUIRED ON` and `CMAKE_CXX_EXTENSIONS OFF` are now set at project scope. Previously only the `mapper` target set `CXX_STANDARD 20`; `mapper_model` and `mapper_io` link no Qt target, inherit no `cxx_std_17` INTERFACE requirement, and compiled at the MSVC default of C++14, which rejects the nested namespace definitions and `[[nodiscard]]` used throughout. `cmake_minimum_required` was raised from 3.8 to 3.21.
10. **Tests gated off by default.** RESOLVED. `MAPPER_BUILD_TESTS` is now declared via `option(... ON)` and set in `CMakePresets.json`. Previously undeclared, so `tests/` was never added and `LastTest.log` recorded a run with zero tests, leaving the Phase 0 exit criterion (KAN-15) unmet.
11. **Visual Studio template residue.** RESOLVED. Root `mapper.cpp` and `mapper.h` were unmodified CMake-template scaffolding, with `mapper.cpp` defining a second `main()`. Both moved to `_to_delete/` for the developer to remove.
11a. **AUTOMOC scope.** RESOLVED. `qt_standard_project_setup()` sets `CMAKE_AUTOMOC` for the whole tree, which generated `mapper_model_autogen` for a target that section 5 forbids from depending on Qt. `CMAKE_AUTOMOC` is now cleared after that call and enabled per target on `mapper_view` and `mapper`.

### 9.3 Vendored GCTP

12. **GCTP vendoring path.** OPEN. The package is vendored at `third_party/gctp/`, holding two distributions: `gctp20/` and `gctpc/`, each with `source/`, `doc/` and `test/proj/`. KAN-8 states it is vendored at `gctp/`; KAN-28 references `gctp/gctpc`, KAN-32 references `gctp/gctpc/test/`, KAN-48 references `gctp/gctpc/source/`. The Jira citations omit the `third_party/` prefix and none of them account for the second distribution, `gctp20/`. No `third_party/gctp/patches/` directory exists yet (KAN-28). No CMake target builds either distribution; GCTP is not yet in the build graph.

### 9.4 Implementation completeness against the phase plan

13. **Phase 0 partially complete.** CMake skeleton (KAN-11) and Qt discovery with an empty `MainWindow` (KAN-13) exist. Catch2 wiring (KAN-15) exists in `tests/CMakeLists.txt` at tag `v3.5.2` and is now reachable per item 10. The 11 existing cases were confirmed passing on 2026-08-14 by configuring `model/`, `io/` and `tests/` with GCC 13 at C++20 on Linux, which exercises the CMake logic but not the MSVC toolchain or the Qt targets.
14. **Phase 1 not started.** `io/` contains only `Endian.h`/`Endian.cpp` (byte-swap helpers plus `hostIsLittleEndian()`). No `ByteReader` (KAN-17), no `.shp` header parser (KAN-19), no record loop (KAN-21), no geometry decoders (KAN-23, KAN-25, KAN-27), no `.shx` reader (KAN-29), no `ParseError` (KAN-31). The `Endian.h` comment correctly documents the mixed byte order the format requires.
15. **Phase 2 not started.** No `Viewport` (KAN-35), no `IRenderer`, no `PainterRenderer` (KAN-39). `view/MapCanvas.h` states this explicitly: "Phase 0 scaffold: paints a placeholder when no layer is loaded. The Viewport transform, IRenderer indirection and the actual geometry drawing arrive in Phase 2 (KAN-35, KAN-39)." `app/MainWindow.cpp` disables the View menu with the comment "Populated in Phase 2 (KAN-43)" and notes "Phase 2 (KAN-49) fills in Open, drag-and-drop, CLI argument and recent files."
16. **Phase 5 CI absent.** No `.github/workflows/` directory (KAN-36).
17. **Test coverage present.** `tests/test_envelope.cpp` (7 cases) and `tests/test_endian.cpp` (4 cases). Neither corresponds to a Phase 0 issue beyond KAN-15's "one trivial passing test"; both exceed that scope. No `Viewport` tests (KAN-37) and no parser fixtures (KAN-33).

### 9.5 Internal inconsistency in the model

18. **Envelope default state contradicts its own documentation.** `model/Envelope.h` states "Default-constructed envelopes are empty (inverted bounds), so the first `expand()` call adopts the point exactly", but the default member initialisers are `xmin = ymin = xmax = ymax = 0.0`, which `isEmpty()` reports as non-empty. Only the explicit `Envelope::empty()` factory produces inverted bounds. `model/Layer.h` uses `Envelope::empty()` for both `Feature::bbox` and `Layer::extent`, so behaviour is currently correct, but a default-constructed `Envelope` does not behave as documented. Not covered by any issue.

### 9.6 Specification gaps

19. **FR-10 and NFR-2 unrecoverable.** No issue in KAN cites either identifier. The surrounding identifiers (FR-9, FR-11; NFR-1, NFR-3) are cited, confirming both identifiers existed.
20. **Section 8 Q2 unrecoverable.** Q1, Q3, Q4 and Q5 are each cited by exactly one issue. No issue cites Q2.
21. **Phase 0 estimate arithmetic.** Constituent story estimates sum to 0.6 d against the epic's stated 0.5 d. Phases 1 through 5 all reconcile exactly.
22. **KAN-20 phase placement.** The `.dbf` reader is parented to Phase 3 (KAN-7) and cites "SPECIFICATION.md section 6, Phase 3", though the remaining file-format parsing is Phase 1. Internally consistent, listed for confirmation that this was intentional.
