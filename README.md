# 🏀 3D Basketball Shooting Simulator

<div align="center">

[![Language](https://img.shields.io/badge/Language-C11-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Graphics](https://img.shields.io/badge/Graphics-OpenGL%20%7C%20FreeGLUT-5586A4?style=for-the-badge&logo=opengl&logoColor=white)](https://www.opengl.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-informational?style=for-the-badge&logo=windows&logoColor=white)](https://github.com/ronit404/Basketball_Simulator)
[![License](https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge)](LICENSE)

**A high-performance, procedural 3D Basketball Shooting Simulator built from scratch in C with OpenGL and FreeGLUT.**

[Features](#-key-features) • [Controls](#-controls-reference) • [Physics Engine](#-physics--mathematical-model) • [Architecture](#-modular-architecture) • [Building & Running](#-compilation--installation)

---

</div>

## 🌟 Overview

The **3D Basketball Shooting Simulator** delivers an arcade-style, physics-accurate basketball shooting experience rendered in real-time 3D. Written in strict procedural **C (C11)**, it requires no heavy external game engines—leveraging raw **OpenGL fixed-function pipeline and FreeGLUT** for fast rendering, responsive controls, and cross-platform compatibility.

Whether you want to test your jump shot in solo practice mode or compete in a local multiplayer shootout tournament, the simulator models realistic projectile mechanics, backboard and rim rebounds, FIBA-standard court dimensions, and an animated stadium atmosphere.

---

## 🎯 Key Features

### 🧮 Realistic 3D Ballistics & Collisions
- **Parabolic Projectile Integration**: True 3D projectile motion with gravitational acceleration ($g = 9.81\,\text{m/s}^2$).
- **Multi-Surface Elastic Collision**: Calibrated coefficients of restitution ($e$) for hardwood floor ($0.70$), tempered glass backboard ($0.65$), and steel rim ($0.60$).
- **Rotational Spin & Motion Trail**: Dynamic backspin animation and an alpha-fading ring-buffer motion trail ($128$ trajectory samples).
- **Precision Detection**: Torus-ball proximity testing for clean "swish" baskets vs. rim clanks, with net entry tracking.

### 👥 Game Modes & Tournament Play
- **Local Multiplayer Tournament (1–4 Players)**:
  - Turn-based shootout competition with dedicated team jersey colors:
    - 🔴 **Player 1**: Crimson Red
    - 🔵 **Player 2**: Royal Blue
    - 🟢 **Player 3**: Emerald Green
    - 🟡 **Player 4**: Golden Amber
  - Configurable target victory scores: **3, 5, 7, 11, or 21 Points**.
- **Solo Practice Mode (Free Play)**:
  - Unlimited continuous shooting with live statistics and instantaneous ball return.

### 🏟 Arena Aesthetics & Immersion
- **FIBA Standard Court Markings**: Accurate regulation dimensions featuring 3-point arc ($6.75\,\text{m}$ radius), free-throw key, lane lines, and center circle.
- **Maple Hardwood Parquet**: Procedural court floor rendering with reflective specular highlights and dynamic soft drop shadows.
- **Animated Stadium Crowd**: Tiered stadium seating populated with 2D billboard spectators that jump and cheer upon successful buckets.
- **Glassmorphic Arcade HUD**:
  - Segmented power meter ($10\% - 100\%$) and elevation dial ($15^\circ - 85^\circ$).
  - Real-time shot zone indicator: `[ 3-POINT ZONE ]` vs `[ 2-POINT ZONE ]`.
  - Dynamic celebratory floating banners (`"SWISH!"`, `"3-POINTER!"`, `"BUCKET!"`).

### 🎥 Dynamic Free-Orbit Camera
- Smooth 360° mouse-drag orbital camera with pitch and yaw clamping.
- Real-time zoom and height adjustment for optimal viewing angles from court-level to broadcast perspective.

---

## 🕹 Controls Reference

| Action | Control Key / Input | Details |
|---|---|---|
| **Move Player** | <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> | Move forward, left, backward, right inside court |
| **Elevation Angle** | <kbd>▲</kbd> / <kbd>▼</kbd> (Up / Down) | Adjust launch arc between $15^\circ$ and $85^\circ$ |
| **Shot Power** | <kbd>◄</kbd> / <kbd>►</kbd> (Left / Right) | Modulate release velocity from $10\%$ to $100\%$ |
| **Shoot Ball** | <kbd>SPACEBAR</kbd> | Release shot from current player position |
| **Reset / Pass Turn** | <kbd>R</kbd> or <kbd>N</kbd> | Return ball to hands or pass turn to next player |
| **Camera Orbit** | <kbd>Left Click</kbd> + Drag | Pan and orbit camera around the court |
| **Camera Zoom / Altitude**| <kbd>E</kbd> / <kbd>Q</kbd> | Zoom camera in/higher or out/lower |
| **Target Score (Menu)** | <kbd>▲</kbd> <kbd>▼</kbd> <kbd>◄</kbd> <kbd>►</kbd> / <kbd>T</kbd> | Cycle winning score limit ($3, 5, 7, 11, 21$) |
| **Select Mode (Menu)** | <kbd>P</kbd> or <kbd>1</kbd> - <kbd>4</kbd> | Start Solo Practice (<kbd>P</kbd>) or Multiplayer (<kbd>1</kbd>–<kbd>4</kbd>) |
| **Pause / Return to Menu** | <kbd>M</kbd> | Return to Main Menu |
| **Quit Game** | <kbd>ESC</kbd> | Exit application |

---

## 📐 Court & Spatial Geometry

```text
       +-----------------------------------------------------------+
       |                                                           |
       |     +-------------------------+                           |
       |     |          LANE           |       3-POINT ARC         |
       |=====|[B]  (O)                 |      (Radius: 6.75m)      |
       |     |  Backboard & Rim        |           .-'""'-.        |
       |     +-------------------------+         .'        '.      |
       |                                        /            \     |
       |                                       |              |    |
       |                                       |    PLAYER    |    |
       |                                       |      (P)     |    |
       |                                        \            /     |
       |                                         '.        .'      |
       |                                           '-....-'        |
       +-----------------------------------------------------------+
```

### Key Dimensional Parameters

| Element | Real-World Value | In-Engine Coordinate |
|---|---|---|
| **Hoop Height** | $3.05\,\text{m}$ ($10\,\text{ft}$) | $Y = 3.05$ |
| **Hoop Center** | $1.575\,\text{m}$ from baseline | $(X = 12.425,\, Y = 3.05,\, Z = 0.0)$ |
| **Backboard** | $1.80\,\text{m} \times 1.05\,\text{m}$ | $X = 12.80$, thickness $0.05\,\text{m}$ |
| **Rim Diameter** | $0.46\,\text{m}$ (radius $0.23\,\text{m}$) | Steel tube radius $0.016\,\text{m}$ |
| **3-Point Arc** | $6.75\,\text{m}$ radius from hoop | Boundary evaluation in $XZ$-plane |
| **Corner 3PT** | $6.60\,\text{m}$ from basket | Flat sideline corner distance |

---

## 🔬 Physics & Mathematical Model

### 1. Projectile Motion
When launched at an angle $\theta$ with release velocity $v_0$ directed towards target azimuthal angle $\phi$:

$$\begin{aligned}
v_x &= v_0 \cos(\theta) \cos(\phi) \\
v_y &= v_0 \sin(\theta) \\
v_z &= v_0 \cos(\theta) \sin(\phi)
\end{aligned}$$

At each tick $\Delta t$:
$$\mathbf{v}(t + \Delta t) = \mathbf{v}(t) + \mathbf{g} \Delta t \quad \text{where } \mathbf{g} = [0, -9.81, 0]^T$$
$$\mathbf{p}(t + \Delta t) = \mathbf{p}(t) + \mathbf{v}(t) \Delta t$$

### 2. Collision Restitution & Friction
- **Floor Impact ($y \le r_{\text{ball}}$)**:
  $$v_y \leftarrow -e_{\text{ground}} \cdot v_y, \quad (v_x, v_z) \leftarrow \mu_{\text{ground}} \cdot (v_x, v_z)$$
- **Backboard Impact**: Elastic collision response against the planar rectangle bounded by $Y \in [y_{\min}, y_{\max}]$ and $Z \in [z_{\min}, z_{\max}]$.
- **Rim Collision**: Toroidal distance calculation against rim points; ball velocity reflects across normal vector $\hat{\mathbf{n}}$ from rim center:
  $$\mathbf{v}' = \mathbf{v} - (1 + e_{\text{rim}})(\mathbf{v} \cdot \hat{\mathbf{n}})\hat{\mathbf{n}}$$

---

## 🏛 Modular Architecture

The project adheres to clean modular separation of concerns with isolated headers:

```
Basketball_Simulator/
├── main.c           # Entry point, GLUT initialization, and main event loop
├── game.h / .c      # Game state machine, rules, scoring, and turn manager
├── ball.h / .c      # Ballistics simulation, collision geometry, and shot detection
├── player.h / .c    # Player kinematics, positional limits, and rotation
├── renderer.h / .c  # OpenGL drawing routines (court, hoop, crowd, player, HUD)
├── Makefile         # Cross-platform build script
├── .gitignore       # Clean version control exclusions
└── README.md        # Comprehensive documentation
```

### Module Responsibilities

| Module | Primary Responsibility |
|---|---|
| **Entry Point** (`main.c`) | Window creation, FreeGLUT event dispatching, timer ticks ($60\,\text{FPS}$). |
| **Game Controller** (`game.c`) | State flow (`MENU` $\rightarrow$ `PLAYING` $\rightarrow$ `GAME_OVER`), scoring tally, banner alerts. |
| **Physics Subsystem** (`ball.c`) | 3D numerical integration, restitution matrix, hoop zone calculations. |
| **Player Model** (`player.c`) | Kinematics, movement dampening, boundary clamping, jersey color mapping. |
| **Renderer Engine** (`renderer.c`) | Double-buffered OpenGL rendering, lighting, material shading, bitmap HUD overlays. |

---

## 🔨 Compilation & Installation

### Prerequisites
- A modern C compiler supporting **C11** (`gcc`, `clang`, or MSVC).
- **OpenGL** libraries (pre-installed on most modern operating systems).
- **FreeGLUT** development package.

---

### Windows (MinGW / MSYS2)

1. **Install FreeGLUT via MSYS2**:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-freeglut make
   ```

2. **Compile using Makefile**:
   ```bash
   make
   ./BasketballSimulator.exe
   ```

3. **Or Compile Directly with GCC**:
   ```bash
   gcc -O2 -Wall -Wextra -std=c11 main.c game.c ball.c player.c renderer.c -o BasketballSimulator.exe -lfreeglut -lopengl32 -lglu32 -lm
   ```

---

### Linux (Ubuntu / Debian / Mint)

1. **Install Dependencies**:
   ```bash
   sudo apt update
   sudo apt install -y build-essential freeglut3-dev libglu1-mesa-dev
   ```

2. **Build and Run**:
   ```bash
   make
   ./BasketballSimulator.exe
   ```

---

### macOS

1. **Install Dependencies via Homebrew**:
   ```bash
   brew install freeglut
   ```

2. **Build using Makefile**:
   ```bash
   make
   ./BasketballSimulator.exe
   ```

---

## ⚙️ Customization & Tuning

All gameplay and physics constants are centralized for simple experimentation:

- **Adjust Gravity or Bounce**: Edit `GRAVITY_ACCEL`, `GROUND_RESTITUTION`, or `RIM_ELASTICITY` in [ball.h](file:///d:/Basketball_CG_Simulator/ball.h).
- **Change Hoop or Court Proportions**: Edit `HOOP_CENTER_X`, `RIM_HEIGHT`, or `THREE_POINT_RADIUS` in [ball.h](file:///d:/Basketball_CG_Simulator/ball.h).
- **Customize Player Colors or Movement**: Edit speeds and palette in [player.h](file:///d:/Basketball_CG_Simulator/player.h) and [player.c](file:///d:/Basketball_CG_Simulator/player.c).
- **Tweak Arena Lighting & Shading**: Adjust ambient, diffuse, and specular light coordinates in [renderer.c](file:///d:/Basketball_CG_Simulator/renderer.c).

---

## 📜 License

This project is licensed under the **MIT License** — you are free to modify, distribute, and integrate this software into your own academic or commercial projects.

---

<div align="center">
Developed with ❤️ by <a href="https://github.com/ronit404">ronit404</a>
</div>
