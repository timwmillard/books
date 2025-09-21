# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C application called "books" that uses the Sokol libraries for cross-platform graphics and Dear ImGui (via cimgui) for GUI. The project is built with CMake and includes a Makefile wrapper for convenience.

## Build System

The project uses CMake with platform-specific configurations:
- **macOS**: Metal backend with Cocoa/QuartzCore/Metal/MetalKit frameworks
- **Windows**: D3D11 backend with gdi32/ole32/d3d11/dxgi libraries
- **Linux**: OpenGL Core backend with GL/m/pthread/dl libraries

## Common Commands

### Building
```bash
make                    # Build the project (creates build dir, runs cmake, builds)
make cmake-build        # Build only (assumes build dir exists)
make run                # Build and run the application
```

### Development
```bash
make clean              # Remove build directory
make update-deps        # Update all dependencies (sokol, imgui, cimgui)
```

### Direct CMake usage
```bash
cmake -B build          # Configure build directory
cmake --build build     # Build the project
./build/books           # Run the executable
```

## Architecture

### Source Structure
- `src/main.c` - Main application entry point (currently minimal)
- `deps/deps.c` - Dependencies implementation file that includes all Sokol implementations
- `deps/` - External dependencies (sokol headers, cimgui, imgui)

### Key Dependencies
- **Sokol**: Cross-platform libraries for app, graphics, audio, time, logging
- **cimgui**: C bindings for Dear ImGui
- **imgui**: Immediate mode GUI library (docking branch)
- **stb_image**: Image loading library

### Dependency Management
Dependencies are managed via wget commands in the Makefile. The project uses:
- Sokol from floooh/sokol master branch
- ImGui from ocornut/imgui docking branch
- cimgui from cimgui/cimgui docking_inter branch

## Development Notes

- The project uses clangd for language server support (configured in `.clangd`)
- All Sokol implementations are included in `deps/deps.c` with `SOKOL_IMPL` defined
- The main application is currently minimal - just prints "books" and exits
- Platform-specific backend selection is handled automatically by CMake