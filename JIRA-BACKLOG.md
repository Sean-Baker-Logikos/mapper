# mapper — Jira Backlog

**Site:** `logikos-sean-mapper.atlassian.net`
**Project:** `KAN` — Mapper Dev (team-managed / next-gen, software)
**Board:** <https://logikos-sean-mapper.atlassian.net/browse/KAN>
**Created:** 2026-08-07
**Status:** Created. 46 issues — 6 Epics, 35 Stories, 5 Tasks.

Structure: Epic per phase, Story per checklist item, derived from `SPECIFICATION.md` §6.
No dates or sprints. Estimates live in each issue's description because the project is
team-managed and exposes neither Story Points nor Original Estimate (see §Field notes).

Issue keys are non-contiguous — the two creation passes ran concurrently and Jira
interleaved key allocation. There are no gaps in coverage.

---

## Epic KAN-4 — Phase 0: Scaffold · 0.5 d

Exit: `cmake --build build && ctest` green on Windows.

| Key | Story | Est |
|-----|-------|-----|
| KAN-10 | Initialize Git repository with baseline config files | 0.1 |
| KAN-11 | Create CMake skeleton with app/view/model/io/tests targets | 0.2 |
| KAN-12 | Add CMakePresets.json for debug, release, and relwithdebinfo | 0.1 |
| KAN-13 | Wire Qt 6 discovery; empty MainWindow launches | 0.1 |
| KAN-15 | Wire Catch2 via FetchContent with one passing test | 0.1 |

## Epic KAN-5 — Phase 1: Shapefile reader · 3 d

Exit: headless CLI harness prints feature count and extent for any sample shapefile.
Blocked by KAN-4.

| Key | Story | Est |
|-----|-------|-----|
| KAN-17 | Implement ByteReader with bounds-checked endian-aware reads | 0.3 |
| KAN-19 | Parse .shp 100-byte main header | 0.3 |
| KAN-21 | Implement record loop with per-record shape type dispatch | 0.3 |
| KAN-23 | Decode Point and MultiPoint geometry | 0.3 |
| KAN-25 | Decode PolyLine and Polygon including multipart and holes | 0.6 |
| KAN-27 | Parse Z and M shape variants, discarding Z/M values | 0.3 |
| KAN-29 | Implement .shx reader with sequential-walk fallback | 0.3 |
| KAN-31 | Add ParseError type and reject MultiPatch cleanly | 0.2 |
| KAN-33 | Unit-test parser with byte fixtures and corruption cases | 0.4 |

## Epic KAN-6 — Phase 2: Render and interact · 3 d

Exit: success criteria S1 and S3 — a usable viewer. Blocked by KAN-5.

| Key | Story | Est |
|-----|-------|-----|
| KAN-35 | Implement Viewport world/screen transform | 0.4 |
| KAN-37 | Unit-test Viewport including round-trip transform | 0.2 |
| KAN-39 | Implement MapCanvas paintEvent through IRenderer to PainterRenderer | 0.6 |
| KAN-41 | Render polygon fills with holes using even-odd fill | 0.4 |
| KAN-43 | Zoom to extent on load and preserve aspect ratio on resize | 0.3 |
| KAN-45 | Implement wheel zoom, drag pan, and keyboard navigation | 0.5 |
| KAN-47 | Add status bar with coordinates, scale, and feature count | 0.2 |
| KAN-49 | Implement File Open, drag-and-drop, CLI argument, and recent files | 0.4 |

## Epic KAN-7 — Phase 3: Performance and polish · 2 d

Exit: success criteria S2, S4, NFR-1. Blocked by KAN-6.

| Key | Story | Est |
|-----|-------|-----|
| KAN-14 | Profile against a 50 MB / 500k-vertex layer and record a baseline | 0.3 |
| KAN-16 | Implement screen-space vertex decimation | 0.4 |
| KAN-18 | Add spatial index over feature bounding boxes for culling | 0.5 |
| KAN-20 | Implement .dbf reader, surfacing feature count only | 0.4 |
| KAN-22 | Add error dialogs, busy cursor, and progress for large loads | 0.2 |
| KAN-24 | Persist window geometry and last directory via QSettings | 0.2 |

## Epic KAN-8 — Phase 4: Projections *(optional for v1)* · 2 d

Exit: a UTM and a geographic shapefile display in a common frame.
Blocked by KAN-6. Gated on decision KAN-42. De-risked by spike KAN-48.

| Key | Story | Est |
|-----|-------|-----|
| KAN-26 | Implement .prj WKT reader | 0.4 |
| KAN-28 | Build vendored GCTPc as a static library with MSVC shim | 0.6 |
| KAN-30 | Implement CrsTransform over GCTP forward/inverse via extern C | 0.5 |
| KAN-32 | Reproject layer vertices on load and validate against GCTP test vectors | 0.5 |

## Epic KAN-9 — Phase 5: Release · 1 d

Exit: success criterion S5. Blocked by KAN-7. Packaging gated on decision KAN-40.

| Key | Story | Est |
|-----|-------|-----|
| KAN-34 | Package for Windows with windeployqt | 0.3 |
| KAN-36 | Add GitHub Actions CI building and testing on Windows and Linux | 0.5 |
| KAN-38 | Write README with build instructions and screenshots | 0.2 |

---

## Decisions and spike (Tasks, no parent Epic)

| Key | Issue | Blocks |
|-----|-------|--------|
| KAN-40 | DECISION: Qt licensing — LGPLv3 or commercial | KAN-9 packaging; the pinned Qt version |
| KAN-42 | DECISION: Is Phase 4 (projections) in v1? | KAN-8 (~2 d) |
| KAN-44 | DECISION: macOS support for v1 | KAN-9 CI scope |
| KAN-46 | Source a representative production shapefile for testing | KAN-5 coverage, KAN-7 baseline |
| KAN-48 | SPIKE: Confirm GCTPc compiles under MSVC (1 h timebox) | De-risks KAN-8 |

---

## Field notes

`KAN` is a **team-managed** project. Consequences:

- **No Story Points or Original Estimate field** is available on create. Estimates are the
  first line of each issue's description. They do not roll up and will not appear in
  velocity or burndown reports. To fix: Project settings → Features → enable *Estimation*,
  then the Story point estimate field becomes settable at 3 points per day.
- **Epic parenting works** — all 35 Stories are correctly parented (verified by JQL).
- Labels applied: `mapper` on every issue, plus `phase-0`…`phase-5`, `optional`,
  `decision`, `spike`.
- Dependencies are real Jira issue links — see §Dependency links below.

## Dependency links

18 links created. Direction reads blocker → blocked.

### Phase chain (Epic level)

```
KAN-4 ──▶ KAN-5 ──▶ KAN-6 ──┬──▶ KAN-7 ──▶ KAN-9
 P0        P1        P2     │     P3        P5
                            └──▶ KAN-8
                                  P4
```

| Blocker | Blocks |
|---------|--------|
| KAN-4 Phase 0 | KAN-5 Phase 1 |
| KAN-5 Phase 1 | KAN-6 Phase 2 |
| KAN-6 Phase 2 | KAN-7 Phase 3 |
| KAN-6 Phase 2 | KAN-8 Phase 4 |
| KAN-7 Phase 3 | KAN-9 Phase 5 |

Phase 4 is the only branch — everything else is a single serial chain.

### Decisions and spike gating real work

| Blocker | Blocks | Why |
|---------|--------|-----|
| KAN-42 Phase 4 in v1? | KAN-8 Phase 4 | Whole epic is contingent |
| KAN-48 GCTPc/MSVC spike | KAN-28 Build GCTPc static lib | Spike determines whether the approach survives |
| KAN-40 Qt licensing | KAN-34 windeployqt packaging | LGPLv3 forces dynamic linking |
| KAN-44 macOS support | KAN-36 GitHub Actions CI | Determines runner matrix |

### Intra-phase ordering (Story level)

| Blocker | Blocks | Why |
|---------|--------|-----|
| KAN-17 ByteReader | KAN-19 Parse main header | Header parsing needs the primitive reads |
| KAN-19 Parse main header | KAN-21 Record loop | Loop starts at the header's end offset |
| KAN-21 Record loop | KAN-23 Point/MultiPoint | Dispatch must exist before decoders |
| KAN-21 Record loop | KAN-25 PolyLine/Polygon | Same |
| KAN-25 PolyLine/Polygon | KAN-41 Polygon fill with holes | Renderer needs ring orientation decoded |
| KAN-35 Viewport | KAN-39 MapCanvas paintEvent | Paint path needs the transform |

Only genuine technical dependencies are linked. The remaining stories within a phase can
be done in any order.

### Relates (context, not blocking)

| Issue | Relates to | Why |
|-------|-----------|-----|
| KAN-46 Production shapefile | KAN-33 Parser unit tests | Real data drives edge-case coverage |
| KAN-46 Production shapefile | KAN-14 Performance baseline | Preferable benchmark input |
| KAN-40 Qt licensing | KAN-12 CMakePresets | Presets pin the Qt version the decision selects |

## Pre-existing issues

`KAN-1` (Task 1), `KAN-2` (Task 2), and `KAN-3` (Subtask 2.1) are Jira's sample content and
were left untouched. **KAN-2 sits in the In Progress column** and will misrepresent board
state until removed. Deleting them is yours to do — I do not delete Jira data.

---

## Sequencing

Phase blockers are strictly serial: KAN-4 → KAN-5 → KAN-6 → KAN-7 → KAN-9, with KAN-8
branching off KAN-6. With one developer this is ~11.5 working days of wall clock, not a
parallelizable backlog. Only Phase 4 can overlap other work.

Recommended first actions, in order: KAN-48 (1 h spike, de-risks the largest unknown),
then KAN-40 and KAN-42 (unblock Phases 4 and 5), then KAN-10 to start Phase 0.
