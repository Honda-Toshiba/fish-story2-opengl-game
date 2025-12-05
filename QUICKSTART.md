# Quick Start Guide - Fish Story 2

## 🚀 Getting Started (2 Days to Deadline!)

### Step 1: Install Dependencies (5 minutes)

Run this command on your Linux system:

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake git \
    libglew-dev libglfw3-dev libglm-dev libassimp-dev \
    libxi-dev libxrandr-dev libxinerama-dev libxcursor-dev
```

### Step 2: Build the Project (2 minutes)

```bash
cd /home/hotdog/Git/Fish-Story-2
./build.sh
```

### Step 3: Run the Game

```bash
cd build
./FishStory2
```

## 🎮 Test the Base Implementation

Once running, you should see:
- ✅ Beautiful underwater ocean environment
- ✅ Kingfish model swimming around
- ✅ Water caustics and lighting effects
- ✅ WASD + Space/Shift controls working
- ✅ Camera switching with keys 1 and 2
- ✅ Sprint with left mouse button

## 📝 What's Next?

The base for Level 1 is complete! Now you need to add:

1. **Obstacles** (Sharks, Fish Hooks)
2. **Collectible Fish** (Clownfish, Goldfish)
3. **Power-ups** (Seashells)
4. **Scoring System**
5. **Sound Effects**
6. **Time Limit & Win Condition**
7. **Level 2** (Cave environment)

## 🐛 Troubleshooting

### "Cannot find -lGL"
```bash
sudo apt-get install mesa-common-dev libgl1-mesa-dev
```

### "glfw3 not found"
```bash
sudo apt-get install libglfw3-dev
```

### "assimp not found"
```bash
sudo apt-get install libassimp-dev
```

### Model not loading
Make sure the Kingfish model files are in:
```
models/Kingfish/Mesh_Kingfish.obj
models/Kingfish/Tex_Kingfish.png
```

## 💡 Tips

- The camera starts in **third-person mode** - press '2' to switch to first-person
- Use **left mouse button** to sprint (swim faster)
- The ocean has boundaries - you can't swim too far out
- Swimming motion is animated automatically

## 🎯 Priority Tasks (By Urgency)

1. **TODAY**: Get this base version running and tested
2. **TOMORROW**: Add obstacles, collectibles, and basic gameplay
3. **DEADLINE DAY**: Final testing, video recording, submission

Good luck! 🐟💨
