# EcosystemProject

A real-time ecosystem simulation featuring evolving creatures with genetic algorithms, built with C++ and ImGui.

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows-blue)
![C++](https://img.shields.io/badge/C%2B%2B-11%2B-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

EcosystemProject is an interactive ecosystem simulation where creatures with different traits (size, speed, sense) compete for survival in a dynamic environment. The simulation features:

- **Real-time Evolution**: Creatures reproduce and mutate their traits over generations
- **Interactive Visualization**: Live monitoring with ImGui-based tools
- **Terrain Simulation**: Dynamic grass growth and pathfinding systems
- **Species Diversity**: Predator-prey relationships between foxes and rabbits
- **Data Analytics**: Population tracking and evolutionary data visualization

## Features

### 🧬 Genetic Evolution
- Creatures evolve over time through reproduction and mutation
- Traits include size, speed, and sensory range
- Natural selection drives population dynamics
- Configurable mutation rates and thresholds

### 🌿 Dynamic Environment
- Procedurally generated terrain with grass growth
- Resource competition affects survival
- Pathfinding algorithms for creature movement
- Environmental pressures shape evolution

### 🦊🐰 Species Simulation
- **Foxes**: Predatory behavior, hunt rabbits for survival
- **Rabbits**: Herbivorous, focus on finding vegetation and avoiding predators
- Balanced ecosystem with predator-prey dynamics

### 🛠️ Interactive Tools
- **Spawn Tool**: Add creatures at specific locations
- **Viewer Tool**: Inspect creature properties and statistics
- **Log Tool**: Monitor simulation events and data
- Real-time parameter adjustment

## Screenshots

> *Screenshots coming soon - simulation in action!*

## Getting Started

### Prerequisites

#### macOS
```bash
# Install dependencies using Homebrew
brew install cmake glfw

# For development (optional)
brew install clang-format
```

#### Windows
- Visual Studio 2017+ with C++ support
- CMake 3.10+
- GLFW library

### Building

#### Quick Build (macOS)
```bash
git clone --recursive https://github.com/yourusername/Ecosystem.git
cd Ecosystem
chmod +x build_mac.sh
./build_mac.sh
```

#### Manual Build
```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/Ecosystem.git
cd Ecosystem

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

### Running

```bash
# From build directory
cd bin
./EcosystemProject
```

## Project Structure

```
Ecosystem/
├── Ecosystem_Project/
│   ├── Include/
│   │   ├── Creatures/          # Creature class definitions
│   │   ├── Data/               # Data structures and utilities
│   │   ├── EcoSystem/          # Core simulation logic
│   │   │   └── Tools/          # Interactive tools (spawn, viewer, etc.)
│   │   └── Utils/              # Utility classes (UUID, etc.)
│   ├── Source/                 # Implementation files
│   └── Dependencies/
│       └── libs/               # GLFW and GL3W libraries
├── external/
│   └── imgui/                  # ImGui submodule (v1.92.1)
├── build/                      # Build artifacts
└── docs/                       # Documentation
```

## Architecture

### Core Components

- **EcoSystem**: Main simulation engine managing creatures and environment
- **Terrain**: Handles pathfinding, grass growth, and environmental features
- **Creatures**: Base class for all living entities (Fox, Rabbit)
- **Tools**: Interactive UI components for simulation control
- **Data**: Evolution tracking and statistical analysis

### Key Technologies

- **C++11+**: Modern C++ with STL containers and algorithms
- **ImGui v1.92.1**: Immediate mode GUI for real-time interaction
- **OpenGL 3.2+**: Cross-platform graphics rendering
- **GLFW**: Window management and input handling
- **CMake**: Cross-platform build system

## Configuration

The simulation can be customized through various parameters:

- **Population limits**: Maximum creatures per species
- **Mutation rates**: How frequently traits change
- **Environmental factors**: Grass growth rates, terrain complexity
- **Reproduction thresholds**: Energy requirements for breeding

## Development

### Code Style
This project follows [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html):

```bash
# Format code
clang-format -i $(find . -name "*.cpp" -o -name "*.h")
```

### Adding New Features

1. **New Creature Types**: Extend the `Creature` base class
2. **Tools**: Implement new UI tools in `EcoSystem/Tools/`
3. **Utilities**: Add reusable code to `Utils/` namespace
4. **Evolution Traits**: Modify `EvolutionData` structure

### Git Submodules

This project uses git submodules for dependencies:

```bash
# Initialize submodules (first time)
git submodule update --init --recursive

# Update submodules
git submodule update --remote
```

## Dependencies

### Runtime Dependencies
- **GLFW 3.x**: Window and input management
- **OpenGL 3.2+**: Graphics rendering

### Build Dependencies
- **CMake 3.10+**: Build system
- **C++11 compatible compiler**: GCC 7+, Clang 5+, MSVC 2017+

### Bundled Dependencies
- **ImGui v1.92.1**: GUI framework (git submodule)
- **GL3W**: OpenGL loading library
- **STB**: Image and font loading (bundled with ImGui)

## Performance

- **Real-time simulation**: 60+ FPS on modern hardware
- **Scalable populations**: Supports hundreds of creatures
- **Efficient pathfinding**: Optimized A* algorithm implementation
- **Memory management**: RAII and smart pointer usage

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository
2. **Create** a feature branch: `git checkout -b feature/amazing-feature`
3. **Follow** Google C++ style guidelines
4. **Add tests** for new functionality
5. **Commit** changes: `git commit -m 'Add amazing feature'`
6. **Push** to branch: `git push origin feature/amazing-feature`
7. **Open** a Pull Request

### Development Setup

```bash
# Install development tools
brew install clang-format cmake

# Run formatting
make format

# Run tests (if available)
make test
```

## Roadmap

- [ ] **Multi-threading**: Parallel simulation processing
- [ ] **Save/Load**: Persistent simulation states
- [ ] **Advanced AI**: Neural network-based creature behavior
- [ ] **3D Visualization**: Three-dimensional environment rendering
- [ ] **Web Interface**: Browser-based monitoring dashboard
- [ ] **Performance Profiling**: Built-in performance analysis tools

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **ImGui**: Omar Cornut and contributors for the excellent immediate mode GUI
- **GLFW**: The GLFW team for cross-platform window management
- **Research Inspiration**: Based on genetic algorithm and ecosystem simulation research
- **Academic Background**: Originally developed for CS380 Research Project

## Contact

- **Issues**: Please report bugs via [GitHub Issues](https://github.com/yourusername/Ecosystem/issues)
- **Discussions**: Use [GitHub Discussions](https://github.com/yourusername/Ecosystem/discussions) for questions
- **Email**: [your.email@example.com](mailto:your.email@example.com)

---

⭐ **Star this repository if you find it interesting!**

*Built with ❤️ for computational biology and evolutionary simulation enthusiasts.*