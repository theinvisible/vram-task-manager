# VRAM Task Manager

A lightweight Windows desktop tool that shows **per-process VRAM usage** for every GPU in the system — the column Task-Manager is missing. Built with Qt 6 Widgets and a dark, Fusion-styled UI.

![Platform](https://img.shields.io/badge/platform-Windows%2010%2F11-blue)
![C++20](https://img.shields.io/badge/C%2B%2B-20-orange)
![Qt](https://img.shields.io/badge/Qt-6-41cd52)

## Features

- **Per-process, per-GPU VRAM breakdown** (dedicated / shared / committed) sampled once per second
- **KPI card per DXGI adapter** with used / total / % utilization
- **NVML integration** for NVIDIA cards — uses the driver's resident frame-buffer figure when available (more accurate than WDDM counters on consumer cards)
- **Multi-GPU aware** — iGPU + dGPU setups, multiple NVIDIA adapters, mixed vendors
- **Live filtering** by process name, sortable columns
- Self-contained: ships with the Qt runtime DLLs next to the executable

## How it works

| Source | Used for |
|---|---|
| **DXGI** (`IDXGIFactory::EnumAdapters`) | Adapter inventory, LUIDs, dedicated / shared memory budget |
| **PDH** GPU Engine / GPU Process Memory counters | Per-process dedicated, shared and committed bytes broken down per `luid_*_phys_*` instance |
| **NVML** (loaded dynamically from the NVIDIA driver) | Accurate per-process resident frame buffer on NVIDIA GPUs |

LUIDs from PDH instances are resolved back to adapter indices via the DXGI inventory, so processes line up with the correct physical GPU.

## Requirements

### Runtime

- Windows 10 1809 (build 17763) or newer, x64
- For NVML data on NVIDIA GPUs: a recent NVIDIA driver (NVML ships with the driver — nothing extra to install)

### Build

- Visual Studio 2022 (MSVC v143) or compatible MSVC toolchain
- CMake ≥ 3.21
- Qt 6 (tested with 6.10.x), MSVC 64-bit kit, `Widgets` component

The CMake script auto-detects Qt under `H:/Qt` or `C:/Qt`. To use a different location, point CMake at it:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
```

## Building

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target vram-task-manager -j
```

`windeployqt` is invoked automatically as a post-build step, so `build/vram-task-manager.exe` is launch-ready — no need to put Qt on `PATH`.

### CLion

Open the folder, pick a Visual Studio toolchain, and build the `vram-task-manager` target. The CMake auto-detection picks up Qt at `H:/Qt` (desktop) / `C:/Qt` (notebook); override `Qt6_DIR` in the CMake profile if your install lives elsewhere.

## Packaging

A signed-free, single-EXE installer is produced by the Inno Setup script in `packaging/windows/installer.iss`. Locally:

```powershell
$staged = "$pwd\build\stage\vram-task-manager"   # directory with the .exe + Qt runtime
ISCC.exe `
  "/DAppVersion=0.1.0" `
  "/DStagedRoot=$staged" `
  "/DAppIconFile=$pwd\resources\app.ico" `
  packaging\windows\installer.iss
```

The GitHub Actions workflow (`.github/workflows/build-windows.yml`) does this end-to-end on every push: configures with Qt 6.10, builds with Ninja + MSVC, stages with `windeployqt`, and produces both a portable ZIP and an Inno Setup installer. Tagged `v*.*.*` pushes are attached to a GitHub release automatically.

## Project layout

```
src/
  main.cpp           # entry point + dark Fusion palette
  MainWindow.{h,cpp} # window, KPI cards, table, refresh loop
  VramModel.{h,cpp}  # Qt item model + sort role
  VramSampler.{h,cpp}# PDH GPU Engine / GPU Process Memory counters
  NvmlSampler.{h,cpp}# dynamic NVML loader and per-process FB query
  GpuInventory.{h,cpp}# DXGI adapter enumeration + LUID lookup
resources/           # app icon, Win32 resources, Qt resource bundle
packaging/windows/   # Inno Setup script
.github/workflows/   # CI build, package, release
```