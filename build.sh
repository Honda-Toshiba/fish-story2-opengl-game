#!/bin/bash

echo "=========================================="
echo "  Building Fish Story 2"
echo "=========================================="

# Create build directory
mkdir -p build
cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build the project
echo "Building project..."
make -j$(nproc)

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "=========================================="
    echo "  Build successful!"
    echo "=========================================="
    echo ""
    echo "To run the game, execute:"
    echo "  cd build"
    echo "  ./FishStory2"
    echo ""
else
    echo ""
    echo "Build failed. Please check the error messages above."
    exit 1
fi
