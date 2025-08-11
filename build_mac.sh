#!/bin/bash

# Mac OS build script for EcosystemProject 
echo "Building EcosystemProject for Mac OS..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Error: Homebrew is not installed. Please install Homebrew first:"
    echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    exit 1
fi

# Install dependencies via Homebrew
echo "Installing dependencies..."
brew install cmake glfw

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring build..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_GLFW=ON

# Build the project
echo "Building..."
make -j$(sysctl -n hw.ncpu)

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build successful!"
    echo "Executable location: build/bin/EcosystemProject"
    echo ""
    echo "To run the application:"
    echo "  cd build/bin"
    echo "  ./EcosystemProject"
else
    echo "❌ Build failed!"
    exit 1
fi