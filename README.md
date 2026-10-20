# 3D Basketball Shooting Simulator

A high-performance, modular 3D Basketball Shooting Simulator written in **C (C11 standard)** with **OpenGL** and **FreeGLUT**.

---

## 🏛 Architecture

The codebase is organized into clean, procedural modules:

| Module | Files | Responsibility |
|---|---|---|
| **Entry & Callbacks** | `main.c` | Window creation, GLUT callback registration, and event dispatch. |
| **Game Controller** | `game.h`, `game.c` | Turn-based state machine, tournament target points, practice mode, camera tracking. |
| **Physics & Ball** | `ball.h`, `ball.c` | 3D projectile motion, backboard & rim elastic collisions, gravity, spin, ring-buffer motion trail, and net score detection. |
| **Player Kinematics** | `player.h`, `player.c` | Player movement kinematics, boundary clipping, orientation facing basket, team jersey palette. |
| **Graphics & HUD** | `renderer.h`, `renderer.c` | Maple court, 3D hoops, animated player models, bleachers, 2D billboard animated audience, non-overlapping HUD overlay. |

---

## 🎮 Gameplay Features

1. **Multiplayer Tournament Mode (1 - 4 Players)**
   - Local turn-based shooting competition.
   - Dynamic jersey colors: Red (P1), Blue (P2), Green (P3), Yellow (P4).
   - Configurable Winning Target Points: **3, 5, 7, 11, 21 PTS**.
2. **Solo Practice Mode (Free Play)**
   - Unlimited shots with continuous score tracking.
   - Instant ball reset for smooth practice.
3. **Realistic 3D Basketball Physics & Geometry**
   - Precise 3-Point Line mathematical detection (7.24m arc radius).
   - Elastic backboard & rim restitution with spin dynamics.
   - Parabolic trajectory simulation with air resistance and floor friction.
4. **Arcade-Style 2D HUD**
   - Non-overlapping multi-tier layout.
   - Segmented power & angle meters.
   - Real-time shot zone indicator (3-POINT ZONE vs 2-POINT ZONE).
   - Dynamic celebration banners for made baskets.

---

## 🕹 Controls

| Action | Key / Input |
|---|---|
| **Move Player** | `W` (Forward), `S` (Backward), `A` (Left), `D` (Right) |
| **Adjust Elevation Angle** | `UP` / `DOWN` Arrow Keys (15° – 85°) |
| **Adjust Shot Power** | `LEFT` / `RIGHT` Arrow Keys (10% – 100%) |
| **Shoot Ball** | `SPACEBAR` |
| **Pass Turn / Reset Ball** | `R` or `N` |
| **Camera Orbit & Pan** | Left Click + Mouse Drag |
| **Camera Zoom & Height** | `E` (Zoom In / Higher) / `Q` (Zoom Out / Lower) |
| **Target Score (Menu)** | `UP` / `DOWN` / `LEFT` / `RIGHT` / `T` / `+` / `-` |
| **Select Mode (Menu)** | `P` (Practice Mode), `1` - `4` (Multiplayer Match) |
| **Return to Menu / Exit** | `M` (Menu) / `ESC` (Quit) |

---

## 🔨 Compilation Instructions

### Using GCC (MinGW on Windows)
```bash
gcc -O2 -Wall -Wextra -std=c11 main.c game.c ball.c player.c renderer.c -o BasketballSimulator.exe -lfreeglut -lopengl32 -lglu32
```

### Using Makefile
```bash
make
./BasketballSimulator.exe
```

### Linux (Debian/Ubuntu)
```bash
sudo apt install build-essential freeglut3-dev libglu1-mesa-dev
make
./BasketballSimulator.exe
```
