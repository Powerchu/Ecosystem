# Ecosystem Project - Ecosystem Simulation - Mac OS Build Guide

This document provides instructions for building and running the Ecosystem Project simulation on Mac OS.

## Prerequisites

### Required Software
- **macOS 10.12** or later
- **Homebrew** (package manager for Mac)
- **Xcode Command Line Tools**

### Installation Steps

1. **Install Homebrew** (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. **Install Xcode Command Line Tools**:
   ```bash
   xcode-select --install
   ```

3. **Install dependencies via Homebrew**:
   ```bash
   brew install cmake glfw
   ```

## Building the Project

### Quick Build (Recommended)
Use the provided build script:
```bash
chmod +x build_mac.sh
./build_mac.sh
```

### Manual Build
If you prefer to build manually:

1. **Create build directory**:
   ```bash
   mkdir -p build
   cd build
   ```

2. **Configure with CMake**:
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_GLFW=ON
   ```

3. **Build the project**:
   ```bash
   make -j$(sysctl -n hw.ncpu)
   ```

## Running the Application

After successful build:
```bash
cd build/bin
./EcosystemProject
```

Or from the project root:
```bash
./build/bin/EcosystemProject
```

## Project Structure

```
Ecosystem/
├── build/                     # Build directory (created during build)
│   └── bin/
│       └── EcosystemProject     # Executable
├── CMakeLists.txt            # Cross-platform build configuration
├── build_mac.sh              # Mac build script
├── Ecosystem_Project/       # Source code
│   ├── Include/              # Header files
│   ├── Source/               # Implementation files
│   └── Dependencies/         # Third-party libraries
└── README_MAC.md            # This file
```

## Features

The ecosystem simulation includes:
- **Ecosystem Management**: Simulate predator-prey relationships
- **Creature Behavior**: Fox and Rabbit AI with evolution
- **Terrain System**: Grass growth and energy distribution
- **Visualization Tools**: Real-time ecosystem monitoring
- **Data Logging**: Population and behavior analytics

## Dependencies

The project uses these cross-platform libraries:
- **GLFW**: Window management and input handling
- **OpenGL**: Graphics rendering
- **ImGui**: Immediate mode GUI framework
- **Standard C++17**: Core language features

## Troubleshooting

### Build Issues

**Error: `cmake not found`**
```bash
brew install cmake
```

**Error: `glfw not found`**
```bash
brew install glfw
```

**OpenGL Issues**
- macOS includes OpenGL by default
- If you encounter OpenGL errors, ensure your Mac supports OpenGL 3.0+

### Runtime Issues

**Application won't start**
- Ensure all dependencies are installed
- Check that you're running from the correct directory
- Verify executable permissions: `chmod +x build/bin/EcosystemProject`

**Performance Issues**
- The simulation is computationally intensive
- Close other applications to free up system resources
- Consider reducing simulation parameters in the GUI

## Development Notes

### Platform Differences from Windows Version
- Replaced `FreeConsole()` with cross-platform equivalent
- Uses macOS OpenGL framework instead of `opengl32.lib`
- CMake build system instead of Visual Studio projects
- macOS frameworks (Cocoa, IOKit, CoreVideo) for windowing

### Compiler Warnings
The build may show warnings about:
- Initialization order in constructors (safe to ignore)
- Unused private fields (future development)
- Sign comparison in loops (performance not affected)

These warnings don't affect functionality and are inherited from the original Windows codebase.

### Stability Improvements
The Mac version includes enhanced error handling:
- Graceful handling of pathfinding edge cases
- Safe bounds checking for creature indexing
- Non-fatal error reporting instead of crashes
- Improved resilience during ecosystem simulation

## License

This project is licensed under the terms specified in the main LICENSE file.

## Support

For build issues specific to Mac OS, check:
1. Ensure all prerequisites are installed
2. Verify Homebrew is functioning: `brew doctor`
3. Check CMake version: `cmake --version` (should be 3.16+)
4. Verify GLFW installation: `brew list glfw`

For ecosystem simulation questions, refer to the original project documentation.