# mapper — Development Environment Setup

**Platform:** Windows 10 (1809+) / Windows 11, x64
**Companion to:** `SPECIFICATION.md`
**Last updated:** 2026-08-07
**Time to complete:** ~45–75 minutes, most of it unattended downloading

This document takes a machine with nothing installed to a state where
`cmake --build build` produces a running `mapper.exe` and `ctest` passes.

Linux and macOS are out of scope for this revision — see §10.

---

## 1. Prerequisites

| Component | Version | Why |
|-----------|---------|-----|
| Windows | 10 build 1809 or later, or Windows 11 (x64) | Qt 6.11 minimum. Note: Qt 6.12 is announced as the **last** release to support Windows 10. |
| Visual Studio 2022 | 17.8+, "Desktop development with C++" | MSVC v143 toolset. The only Windows compiler Qt ships prebuilt binaries for. |
| Windows 11 SDK | 10.0.22621 or later | Bundled with the VS workload. |
| CMake | 3.21+ required; 4.4.x current | Build system. Qt itself needs ≥ 3.16; the spec sets 3.21 for `CMakePresets` v3. |
| Ninja | 1.11+ | Fast parallel builds. Ships with VS 2022. |
| Git | 2.40+ | Version control. |
| Qt | 6.8 LTS or 6.11 — **decide first, see §3.1** | GUI toolkit. |
| Disk | ~35 GB free | VS ~20 GB, Qt ~5 GB, build tree ~3 GB, headroom. |
| RAM | 16 GB recommended | 8 GB works but link steps will swap. |

Administrator rights are needed for the Visual Studio and Git installers.
Qt installs per-user and does not require them.

---

## 2. Install the toolchain

### 2.1 Visual Studio 2022

Download the Community, Professional, or Enterprise installer from
<https://visualstudio.microsoft.com/downloads/>. Community is free for
individuals and small organizations — confirm Logikos' license terms before
using it commercially.

In the installer, select the **Desktop development with C++** workload. Under
*Installation details*, confirm these are checked:

- MSVC v143 — VS 2022 C++ x64/x86 build tools (latest)
- Windows 11 SDK (10.0.22621.0 or newer)
- C++ CMake tools for Windows *(this is where Ninja comes from)*
- C++ AddressSanitizer *(optional, useful for the parser fuzzing in Phase 1)*

If you only want the compiler without the IDE, install **Build Tools for
Visual Studio 2022** instead and use VS Code or CLion as the editor.

Verify from a *Developer Command Prompt for VS 2022*:

```bat
cl
:: Microsoft (R) C/C++ Optimizing Compiler Version 19.4x.xxxxx for x64
ninja --version
:: 1.11.x or later
```

### 2.2 CMake

The VS workload includes a CMake, but it lags behind and is not on the system
`PATH`. Install the standalone release so every shell and IDE sees the same one.

Download the Windows x64 installer (`cmake-4.4.x-windows-x86_64.msi`) from
<https://cmake.org/download/>. **Check "Add CMake to the system PATH for all
users"** during install.

```bat
cmake --version
:: cmake version 4.4.x
```

> CMake 4.x rejects projects declaring `cmake_minimum_required(VERSION <3.5)`.
> That affects old third-party CMake projects, not `mapper` — the vendored GCTP
> ships Unix Makefiles, not CMake, and we wrap it in our own target (§7).

### 2.3 Git

Install from <https://git-scm.com/download/win>. Defaults are fine, with two
choices worth deliberating:

- **Line endings:** select "Checkout as-is, commit Unix-style line endings".
  The repo's `.gitattributes` is the authority; this setting avoids fighting it.
- **Credential Manager:** leave enabled.

```bat
git --version
git config --global user.name  "Sean Baker"
git config --global user.email "sbaker@logikos.com"
git config --global core.longpaths true
```

`core.longpaths` matters: Qt-adjacent paths plus a deep build tree can exceed
the legacy 260-character limit. Also enable it system-wide once:

```powershell
# PowerShell as Administrator
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" `
  -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
```

---

## 3. Install Qt 6

### 3.1 Choose a version first

This is a real decision, not a formality, and it is coupled to
`SPECIFICATION.md` §8 Q1 (licensing):

| | Qt 6.8 LTS | Qt 6.11 |
|---|---|---|
| Released | Oct 2024 | Mar 2026 |
| LTS | Yes — but **commercial license holders only** | No |
| Open-source patches | Ended Apr 2025 | Through Sep 2026 |
| Best for | A commercial Qt license (support to Oct 2029) | Open-source / LGPL use |

Qt's LTS designation confers nothing on open-source users — for them an LTS
release behaves exactly like a regular one and stops receiving patches when the
next release lands. So:

- **Open-source / LGPLv3 build → install Qt 6.11.** Newest security fixes,
  and you will be moving forward with each release regardless.
- **Commercial license → install Qt 6.8 LTS.** Patch stream through 2029.

Pick one and pin it in `CMakePresets.json` so every developer and CI agent
builds against the same version. Until §8 Q1 is settled, **6.11 is the working
default.**

### 3.2 Run the Qt Online Installer

1. Create a free Qt Account at <https://login.qt.io/register> — the online
   installer requires sign-in even for open-source use.
2. Download the Qt Online Installer for Windows from
   <https://www.qt.io/download-qt-installer-oss>.
3. Run it. When asked, select **open-source** or enter commercial credentials,
   per §3.1.
4. At *Installation Folder*, keep `C:\Qt` and choose **Custom installation**.
   Do not accept the default selection — it installs far more than we need.

Select exactly these components:

```
Qt
└── Qt 6.11.x                 (or 6.8.x — see §3.1)
    ├── [x] MSVC 2022 64-bit
    ├── [x] Qt Debug Information Files      ← needed for usable debugging
    └── Additional Libraries
        └── (none required for v1)
Qt
└── Developer and Designer Tools
    ├── [x] Qt Creator <version>            ← optional, see §6.2
    ├── [ ] Qt Creator CDB Debugger Support ← check this if using Qt Creator
    ├── [ ] CMake                           ← skip, we installed it in §2.2
    └── [ ] Ninja                           ← skip, VS provides it
```

Explicitly **do not** install MinGW, WebAssembly, Android, or the Qt 5
compatibility module. They add tens of gigabytes and Qt binaries built by
different compilers cannot be mixed — a MinGW Qt will not link against MSVC
object files.

Expected result: `C:\Qt\6.11.x\msvc2022_64\`, roughly 5 GB.

### 3.3 Tell CMake where Qt is

Set a persistent user environment variable so no one has to pass `-DCMAKE_PREFIX_PATH`
by hand:

```powershell
[Environment]::SetEnvironmentVariable(
  "CMAKE_PREFIX_PATH", "C:\Qt\6.11.0\msvc2022_64", "User")
```

Adjust the patch version to match what you installed. Open a new terminal for
it to take effect.

---

## 4. Get the source and build

### 4.1 Clone

```bat
mkdir C:\Projects\mapper\source
cd C:\Projects\mapper\source
git clone git@github.com:Sean-Baker-Logikos/mapper.git
cd mapper
```

**Repository root is `C:\Projects\mapper\source\mapper`** — not `C:\Projects\mapper`.
Everything the build needs is inside it, including the vendored GCTP sources at
`third_party/gctp/` and the three project documents at the root.

The original GCTP `.tar.Z` archives (~21 MB) stay outside the repo at
`C:\Projects\mapper\gctp\`. They duplicate the extracted trees and are not
needed to build.

The clone URL is SSH. If you don't have a key registered with GitHub, use the
HTTPS remote instead: `https://github.com/Sean-Baker-Logikos/mapper.git`.

### 4.2 Configure and build

All commands run from the repository root in a **Developer Command Prompt for
VS 2022** (or any shell after running `vcvars64.bat`). A plain `cmd` or
PowerShell will not find `cl.exe`.

```bat
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug --output-on-failure
```

Presets defined in `CMakePresets.json`:

| Preset | Config | Notes |
|--------|--------|-------|
| `windows-debug` | Debug | Ninja, ASan available, assertions on. Day-to-day. |
| `windows-release` | Release | `/O2`, LTO. Use for the Phase 3 performance work — Debug numbers are meaningless. |
| `windows-relwithdebinfo` | RelWithDebInfo | Optimized with symbols. Use for profiling. |

Binaries land in `build\windows-debug\app\mapper.exe`.

### 4.3 Run

```bat
build\windows-debug\app\mapper.exe
build\windows-debug\app\mapper.exe data\samples\ne_110m_admin_0_countries.shp
```

Running from the command line inherits `PATH`; if Qt DLLs are not found, see §9.

---

## 5. Test data

The repository does not vendor sample shapefiles — they are large and licensed
separately. Download at least one set into `data\samples\` (git-ignored):

| Source | Use | URL |
|--------|-----|-----|
| Natural Earth 1:110m Admin 0 Countries | Small polygon layer, ~4k vertices. Primary smoke test. | <https://www.naturalearthdata.com/downloads/110m-cultural-vectors/> |
| Natural Earth 1:10m Admin 0 Countries | ~550k vertices. The NFR-1 / S2 performance benchmark. | <https://www.naturalearthdata.com/downloads/10m-cultural-vectors/> |
| US Census TIGER/Line roads (one county) | Polyline layer, real-world non-conformant producer. | <https://www.census.gov/geographies/mapping-files/time-series/geo/tiger-line-file.html> |

Natural Earth is public domain; TIGER/Line is a US Government work. Both are
free to redistribute, but keeping them out of the repo keeps clone times sane.

Per `SPECIFICATION.md` §8 Q5 — if Logikos has a representative production
shapefile, add it here. Real data surfaces parser edge cases that curated
samples do not.

---

## 6. IDE setup

Any of these work. Pick one; the build is IDE-agnostic because everything runs
through CMake presets.

### 6.1 Visual Studio 2022

*File → Open → Folder…* on `C:\Projects\mapper`. VS detects `CMakePresets.json`
automatically and populates the configuration dropdown. Set `mapper` as the
startup item.

Debugging Qt types (`QString`, `QPointF`) shows raw memory unless you install
Qt's `.natvis` visualizers. Copy `qt6.natvis` from the Qt Creator installation
into `%USERPROFILE%\Documents\Visual Studio 2022\Visualizers\`.

### 6.2 Qt Creator

*File → Open File or Project…* → select `CMakeLists.txt`. Under
*Tools → Options → Kits*, confirm a kit exists pairing **Qt 6.11 MSVC2022 64-bit**
with the **MSVC 2022 x64** compiler and the **CDB** debugger. If the debugger
row is empty, you skipped "Qt Creator CDB Debugger Support" in §3.2 — rerun the
Qt Maintenance Tool and add it.

Qt Creator has the best Qt-native debugging and the built-in Designer. It is the
weakest of the four at general C++ refactoring.

### 6.3 VS Code

Install extensions: **C/C++** (ms-vscode.cpptools) and **CMake Tools**
(ms-vscode.cmake-tools). Open the folder; CMake Tools reads the presets. Select
`windows-debug` from the status bar. Set `"cmake.configureOnOpen": true` in
workspace settings.

Launch VS Code from a Developer Command Prompt (`code .`) so it inherits the
MSVC environment, or configure the CMake Tools kit to a Visual Studio kit.

### 6.4 CLion

*File → Open* the folder. CLion reads `CMakePresets.json` natively. In
*Settings → Build, Execution, Deployment → Toolchains*, select **Visual Studio**
with architecture `amd64`. Requires a paid license.

---

## 7. The vendored GCTP tree

`third_party/gctp/` holds two USGS General Cartographic Transformation Package
distributions — `gctpc` (C, v c.1.3, Feb 1996) and `gctp20`. This is Phase 4
material per the specification and **requires no setup work now.**

When Phase 4 begins, be aware:

- It is K&R-era C targeting DG/UX, SunOS, and IRIX. It has never been compiled
  by MSVC.
- Expect to need `_CRT_SECURE_NO_WARNINGS`, `/W0` on that target only, and
  small patches for missing `<unistd.h>` and `M_PI`.
- It builds as a separate static library target with warnings-as-errors
  disabled, exposed to C++ through an `extern "C"` header we write. The
  vendored sources stay unmodified where possible; any required patch goes in
  `third_party/gctp/patches/` with a note explaining why.
- `third_party/gctp/gctpc/test/` contains USGS validation vectors. Wire those
  into `ctest` as the acceptance criterion for the transform layer.

Warnings-as-errors is on for mapper's own targets via the `mapper_warnings`
interface target in the root `CMakeLists.txt`. GCTP will not link that target.

---

## 8. Verify your setup

Work down this list. Every item should pass before you pick up a task.

- [ ] `cl` in a Developer Command Prompt prints a v19.4x banner for x64.
- [ ] `cmake --version` ≥ 4.0 (3.21 minimum).
- [ ] `ninja --version` ≥ 1.11.
- [ ] `git --version` ≥ 2.40, and `git config user.email` returns your address.
- [ ] `dir C:\Qt\6.11.0\msvc2022_64\bin\Qt6Widgets.dll` exists.
- [ ] `echo %CMAKE_PREFIX_PATH%` prints the Qt msvc2022_64 path.
- [ ] `cmake --preset windows-debug` completes without a `Could NOT find Qt6` error.
- [ ] `cmake --build --preset windows-debug` completes with zero warnings.
- [ ] `ctest --preset windows-debug` reports 100% tests passed.
- [ ] `mapper.exe` launches and shows an empty main window.
- [ ] Opening `ne_110m_admin_0_countries.shp` renders world polygons. *(Phase 2+)*
- [ ] Setting a breakpoint in `ShapefileReader` and inspecting a `QString` shows
      readable text, not raw bytes.

---

## 9. Troubleshooting

**`CMake Error: Could NOT find Qt6 (missing: Qt6_DIR)`**
`CMAKE_PREFIX_PATH` is unset, wrong, or the terminal predates setting it. Confirm
the path points at the `msvc2022_64` directory — not `C:\Qt` and not
`C:\Qt\6.11.0`. Open a fresh terminal. Delete the `build\` directory and
reconfigure; CMake caches the failure.

**`'cl' is not recognized` / `No CMAKE_CXX_COMPILER could be found`**
You are in a plain shell. Use *Developer Command Prompt for VS 2022*, or run
`"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
first.

**`mapper.exe` exits immediately, or "Qt6Widgets.dll was not found"**
The Qt `bin` directory is not on `PATH`. Either add `C:\Qt\6.11.0\msvc2022_64\bin`
to your user `PATH`, or run `windeployqt.exe` against the built executable to
copy the DLLs alongside it:

```bat
C:\Qt\6.11.0\msvc2022_64\bin\windeployqt.exe build\windows-debug\app\mapper.exe
```

**`LNK2019: unresolved external symbol` referencing Qt symbols**
Architecture or toolchain mismatch — a 32-bit build against 64-bit Qt, or MinGW
Qt against MSVC. Confirm the preset uses `amd64` and that you installed the
**MSVC 2022 64-bit** Qt component, not MinGW.

**`LNK1104: cannot open file 'Qt6Widgetsd.lib'`**
Debug Qt libraries are missing. On Windows the online installer includes them
with the main component, but the *Qt Debug Information Files* selection is what
makes them debuggable. Rerun the Qt Maintenance Tool and add it.

**Mixing Debug and Release**
Windows MSVC cannot link a Debug build against Release Qt libraries — the CRT
differs. Never point a Release preset at a Debug Qt or vice versa. If you see
inexplicable heap corruption at startup, this is the first thing to check.

**Build fails with paths truncated at ~260 characters**
Long paths are not enabled. See the registry command in §2.3, then restart the
shell.

**Antivirus makes builds glacially slow**
Add `C:\Projects\mapper\build`, `C:\Qt`, and the MSVC toolchain directory to
Windows Defender's exclusion list. Ninja builds generate thousands of small
files and real-time scanning dominates the wall clock.

**Everything worked yesterday, nothing works today**
Delete `build\` and reconfigure. CMake caches compiler and Qt paths; a Qt
Maintenance Tool update or a VS update invalidates them without CMake noticing.

---

## 10. Not covered

- **Linux and macOS.** The specification names both as portability targets
  (NFR-4) and CI covers Linux in Phase 5. Add sections here when that work
  starts — the CMake presets should extend rather than fork.
- **CI runners.** GitHub Actions setup is Phase 5. It will need a headless Qt
  install; `jurplel/install-qt-action` or `aqtinstall` is the usual answer, not
  the GUI installer documented here.
- **Packaging.** `windeployqt` is mentioned above as a debugging aid only.
  Producing a distributable installer is Phase 5 and depends on the licensing
  decision in `SPECIFICATION.md` §8 Q1 — LGPLv3 requires dynamic linking and
  the ability for a user to relink against their own Qt.

---

## 11. Maintenance

Update this document when the pinned Qt version changes, when a new
prerequisite is added, or when a setup failure costs someone more than fifteen
minutes — that last case is the most valuable and the most often skipped. Log
amendments below.

| Date | Change |
|------|--------|
| 2026-08-07 | Initial version. Windows-only. Qt 6.11 as working default. |
| 2026-08-07 | Corrected repo root to `C:\Projects\mapper\source\mapper`; GCTP relocated to `third_party/gctp/`; added HTTPS remote fallback. |

---

**Sources for version and platform claims:**
[Qt supported platforms](https://doc.qt.io/qt-6/supported-platforms.html) ·
[Qt release/EOL dates](https://endoflife.date/qt) ·
[Qt LTS policy](https://www.qt.io/development/qt-framework/qt-lts) ·
[Qt CMake getting started](https://doc.qt.io/qt-6/cmake-get-started.html) ·
[CMake releases](https://cmake.org/download/)
