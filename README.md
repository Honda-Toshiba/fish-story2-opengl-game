# Fish Story 2 🐟
OpenGL-based 2D game developed as a university project, including rendering, input handling, and basic game logic
**The greatest game ever made** - Inspired by Kenshi and SpongeBob Simulator

A 3D underwater adventure game built with C++ and OpenGL for the DMET 502 Computer Graphics course at GUC.

## 📋 Project Overview

Fish Story 2 is a two-level 3D game where you play as a Kingfish navigating underwater environments:

### Level 1: Open Ocean
- Swim freely in a vast ocean environment
- Eat smaller fish to grow larger
- Avoid sharks and fish hooks
- Collect seashells for power-ups
- Dynamic day/night lighting system

### Level 2: Dark Cave (Coming Soon)
- Navigate through dangerous underwater caves
- Avoid stalactites and crabs
- Collect coins and angler fish for light
- Reach the treasure chest at the end

## 🎮 Controls

- **WASD** - Move forward/backward/left/right
- **Space** - Move up
- **Shift** - Move down
- **Hold Left Mouse** - Sprint
- **Keys 1/2** - Toggle between first-person and third-person camera
- **Mouse Movement** - Look around / Control fish direction
- **ESC** - Exit game

## 🌊 Features

### Current Implementation (Level 1 Base)
- ✅ Cross-platform support (Windows & Linux)
- ✅ Animated Kingfish player model
- ✅ Beautiful underwater ocean environment with:
  - Dynamic water caustics
  - God rays lighting effect
  - Underwater fog and depth
  - Animated water surface
  - Sandy ocean floor
- ✅ First-person and third-person camera modes
- ✅ Swimming physics with sprint functionality
- ✅ Boundary system to keep player in play area
- ✅ Smooth swimming animations

### Upcoming Features
- 🔄 Collectible fish (targets)
- 🔄 Obstacles (sharks, fish hooks)
- 🔄 Power-ups (seashells)
- 🔄 Scoring system
- 🔄 Sound effects
- 🔄 Level 2: Cave environment

## 🛠️ Building the Project

### Prerequisites

#### Linux
```bash
sudo apt-get install build-essential cmake
sudo apt-get install libglew-dev libglfw3-dev libglm-dev libassimp-dev
```

#### Windows
- Install [CMake](https://cmake.org/download/)
- Install [MinGW-w64](https://www.mingw-w64.org/) or Visual Studio
- Install OpenGL development libraries (GLEW, GLFW, GLM, Assimp)

### Build Instructions

#### Linux
```bash
chmod +x build.sh
./build.sh
cd build
./FishStory2
```

#### Windows
```batch
build.bat
cd build
FishStory2.exe
```

#### Manual Build
```bash
mkdir build
cd build
cmake ..
make
./FishStory2
```

## 📁 Project Structure

```
Fish-Story-2/
├── src/
│   ├── main.cpp          # Entry point
│   ├── Game.cpp/h        # Main game loop and logic
│   ├── Player.cpp/h      # Player (Kingfish) controller
│   ├── Camera.cpp/h      # Camera system (FP/TP)
│   ├── Ocean.cpp/h       # Ocean environment renderer
│   ├── Model.cpp/h       # 3D model loader
│   ├── Shader.cpp/h      # Shader management
│   └── stb_image.h       # Image loading library
├── shaders/
│   ├── vertex.glsl       # Vertex shader with water animation
│   └── fragment.glsl     # Fragment shader with underwater effects
├── models/
│   └── Kingfish/         # Player 3D model and textures
├── CMakeLists.txt        # Build configuration
├── build.sh              # Linux build script
├── build.bat             # Windows build script
└── README.md             # This file
```

## 🎨 Technical Highlights

### Graphics Features
- **Modern OpenGL 3.3+** with shader-based rendering
- **Phong lighting model** with ambient, diffuse, and specular components
- **Procedural caustics** for realistic underwater light patterns
- **God rays effect** simulating sunlight penetration
- **Depth-based fog** for atmospheric underwater perspective
- **Animated water surface** using sine wave displacement
- **Model loading** via Assimp for .obj, .3ds, and other formats

### Animation System
- Procedural fish swimming motion
- Mouth opening/closing animation
- Wave-based body movement
- Smooth camera following

### Cross-Platform Design
- Platform-agnostic OpenGL implementation
- CMake build system for universal compilation
- Conditional compilation for Windows/Linux differences

## 📝 Course Information

- **Course:** DMET 502 Computer Graphics
- **Semester:** Winter 2025
- **Institution:** German University in Cairo
- **Instructor:** Prof. Dr. Rimon Elias

## 📄 License

This project is developed for educational purposes as part of a university course.

---

**Note:** This is currently the base implementation for Level 1. Additional features including obstacles, collectibles, scoring, and Level 2 are in development.
