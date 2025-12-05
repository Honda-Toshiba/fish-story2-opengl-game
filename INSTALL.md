# Dependencies Installation Guide

## Linux (Ubuntu/Debian)

### Install all dependencies:
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libglew-dev \
    libglfw3-dev \
    libglm-dev \
    libassimp-dev \
    libxi-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev
```

### Verify installation:
```bash
pkg-config --modversion glew glfw3 assimp
```

## Linux (Fedora/RHEL)

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    glew-devel \
    glfw-devel \
    glm-devel \
    assimp-devel \
    libXi-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel
```

## Windows

### Option 1: Using vcpkg (Recommended)

1. Install vcpkg:
```batch
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

2. Install dependencies:
```batch
vcpkg install glew:x64-windows
vcpkg install glfw3:x64-windows
vcpkg install glm:x64-windows
vcpkg install assimp:x64-windows
```

3. Integrate with CMake:
```batch
vcpkg integrate install
```

4. Build with CMake:
```batch
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

### Option 2: Manual Installation

1. **GLEW**: Download from http://glew.sourceforge.net/
2. **GLFW**: Download from https://www.glfw.org/download.html
3. **GLM**: Download from https://github.com/g-truc/glm/releases
4. **Assimp**: Download from https://github.com/assimp/assimp/releases

Extract all libraries and set up include/lib paths in CMake.

## macOS

```bash
brew install cmake glew glfw glm assimp
```

## Troubleshooting

### Linux: Missing OpenGL
```bash
sudo apt-get install mesa-utils libgl1-mesa-dev
```

### Windows: MinGW not found
Install MinGW-w64 from: https://www.mingw-w64.org/downloads/

### CMake can't find libraries
Set environment variables or specify paths:
```bash
cmake .. -DGLEW_INCLUDE_DIR=/path/to/glew/include -DGLEW_LIBRARY=/path/to/glew/lib
```

## Verify Your Setup

After installation, run:
```bash
mkdir build && cd build
cmake ..
```

If CMake completes without errors, all dependencies are correctly installed!
