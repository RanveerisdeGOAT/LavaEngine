# LavaEngine [0.0.1-indev.giting]

**LavaEngine** is a modular C++ game framework designed to provide the infrastructure needed to build games and interactive applications without imposing a predefined engine architecture.

Unlike traditional engines such as Unity or Unreal, LavaEngine does not require concepts such as Scenes, GameObjects, Entities, Components, or a mandatory game lifecycle. Instead, it provides a small set of general-purpose primitives that developers can combine and organize however they choose.

The central concept of LavaEngine is the **Container**.

---

## Containers

A **Container** is an isolated context that can hold modules, resources, interfaces, and other state, while providing controlled communication with other Containers.

A Container does not necessarily represent a scene or even a game. It could represent:

- A game
- A level
- A level editor
- A simulation
- An asset processor
- A dedicated server
- A UI system
- A networking system
- A collection of shared state
- Or any custom system the developer needs

For example:

```text
LavaEngine
│
├── Game Container
│   ├── ECS
│   ├── Physics
│   ├── World
│   └── Renderer
│
├── Level Container
│   └── LevelData
│
└── Editor Container
    ├── UI
    └── World Editor
```

There is no requirement for these Containers to follow the same structure.

A game might use an ECS and physics system, while an asset-processing Container might contain nothing but importers and asset data.

The Container is fundamentally a **boundary for composition, state, and communication**, rather than a predefined type of application object.

---

## Container Communication

Containers communicate through explicit mechanisms rather than relying on hidden global state.

LavaEngine provides three primary communication mechanisms:

```text
Resources
    Share data and state.

Interfaces
    Provide access to behavior.

Events
    Notify other Containers that something happened.
```

### Resources

A Container can export a resource for other Containers to access.

For example:

```cpp
auto levelData = level.exportResource<LevelData>();

game.importResource(levelData);
```

Resources do not necessarily have to be copied.

They can be represented using handles or shared references, allowing multiple Containers to access the same underlying resource.

This is particularly useful for large resources such as:

- Voxel data
- World data
- Meshes
- Textures
- Audio data
- GPU buffers
- Vulkan resources

For example:

```text
Level Container
      │
      └── GPU World Buffer
              │
              ├────────→ Game Container
              │
              └────────→ Editor Container
```

Both Containers can potentially access the same underlying resource without unnecessary CPU or GPU copies.

---

### Interfaces

Resources are useful when a Container needs access to data.

When a Container needs to request behavior, it can instead use an interface.

For example:

```text
Editor
   │
   │ requires
   ↓
WorldEditorInterface
   │
   ↓
World
```

The Editor does not need to know how the World is internally implemented.

It only needs to know what the `WorldEditorInterface` provides.

This creates a clear boundary between systems and prevents Containers from becoming tightly coupled to each other's implementations.

---

### Events

Events provide a mechanism for transient communication.

For example:

```text
Game ──── PlayerDied ─────→ UI
Game ──── LevelLoaded ────→ Editor
Editor ── AssetChanged ───→ Game
```

Events are intended for notifications rather than replacing resources or interfaces.

A useful distinction is:

```text
Resources
    "Here is something."

Interfaces
    "Here is something you can do."

Events
    "Something happened."
```

---

# Modules

**Modules** provide functionality.

They are optional and are added to Containers explicitly.

Examples include:

- ECS
- Physics
- Rendering
- Audio
- Networking
- UI
- Asset management
- Logger

For example:

```cpp
Container game;

game.addModule<ECS>();
game.addModule<Physics>();
game.addModule<Renderer>();
```

A different Container might only require:

```cpp
Container assetCompiler;

assetCompiler.addModule<ModelImporter>();
assetCompiler.addModule<TextureCompiler>();
```

LavaEngine does not need to know what `ModelImporter` or `TextureCompiler` are.

Developers can create their own modules without modifying the framework.

This is a fundamental part of LavaEngine's design:

> **Functionality should be added to the framework through modules rather than being forced into its core.**

---

# Resources

Resources represent data or objects that exist within the framework.

Examples include:

```text
World
LevelData
Mesh
Texture
Material
Shader
GPU Buffer
Audio Data
Player Data
```

Resources are separate from Modules.

A Module performs functionality, while a Resource represents something that functionality operates on.

For example:

```text
Container
│
├── Modules
│   ├── Renderer
│   ├── Physics
│   └── ECS
│
└── Resources
    ├── World
    ├── Mesh
    └── GPU Buffer
```

Resources can be exposed to other Containers using resource handles.

Conceptually:

```cpp
ResourceHandle<World> world;
```

This allows LavaEngine to manage resource ownership, lifetime, sharing, and access without requiring Containers to directly manage raw pointers.

---

# Outputs

LavaEngine treats output as an optional capability rather than a mandatory property of every Container.

The framework may provide a primary window output:

```text
LavaEngine.WindowOutput
```

A rendering module can connect its output to it.

For example:

```text
Game Container
      │
   Renderer
      │
      ↓
Window Output
      │
      ↓
    Display
```

However, a Container does not need to render anything.

For example:

```text
Server Container
├── World
├── Physics
└── Networking
```

A level-data Container could contain nothing that produces output:

```text
Level Container
└── LevelData
```

The LavaEngine application has a **single primary output Container**, but individual Containers are not required to produce output.

This allows LavaEngine to support:

- Games
- Dedicated servers
- Level editors
- Asset tools
- Simulations
- Headless applications
- Rendering experiments

without requiring every application or Container to have a window or renderer.

---

# Game and Editor

Tools do not need to be special parts of LavaEngine.

A game and its level editor can simply be separate Containers.

For example:

```text
              Shared World
               /       \
              /         \
           Game         Editor
            │             │
         Renderer       UI/Gizmos
            │
       Window Output
```

Both Containers can communicate with the same world through resources and interfaces.

The Editor is therefore not a privileged part of the framework.

It is simply another application built using LavaEngine's Container system.

This also allows more complex configurations:

```text
Level
  │
  └── World
       ├── Game
       └── Editor
```

The Game and Editor can operate on the same underlying resources while maintaining separate modules and responsibilities.

---

# Inspector

The **Inspector** is a development and debugging system that allows developers to visualize, monitor, and modify Containers and their contents at runtime.

The Inspector does not need to know what a Container represents.

It can inspect:

- Containers
- Modules
- Resources
- Exported resources
- Interfaces
- Variables
- Runtime state
- Performance information
- Logs
- Debug information
- GPU resources
- Custom user-defined data

For example:

```text
Inspector
│
├── Game Container
│   ├── Modules
│   │   ├── ECS
│   │   ├── Physics
│   │   └── Renderer
│   │
│   ├── Resources
│   │   ├── World
│   │   ├── Player
│   │   └── GPU World Buffer
│   │
│   ├── Exports
│   │   └── WorldEditorInterface
│   │
│   └── Variables
│       ├── gravity
│       ├── tickRate
│       └── debugRendering
│
└── Level Container
    ├── Resources
    │   └── LevelData
    └── Variables
        ├── seed
        └── chunkSize
```

## Inspectable Properties

Modules can expose variables to the Inspector.

For example:

```cpp
class Physics : public Module
{
public:
    float gravity = 9.81f;
    bool debugDraw = false;
};
```

The module can register these properties with LavaEngine:

```cpp
inspect
    .property("gravity", gravity)
    .property("debugDraw", debugDraw);
```

The Inspector can then automatically display:

```text
Physics

Gravity       [ 9.81 ]
Debug Draw    [ ✓ ]
```

Changing a writable property in the Inspector modifies the live object.

This allows developers to experiment with values without restarting the application.

For example:

```text
Render Distance
16 → 32 → 64 → 128

Physics Gravity
9.81 → 5.0 → 0.0
```

This can be used for:

- Rendering settings
- Physics parameters
- AI settings
- World generation
- Debug features
- Network configuration
- Audio settings

---

## Resource Inspection

Resources can also expose information to the Inspector.

For example:

```cpp
auto world = container.exportResource<World>();
```

The Inspector could display:

```text
World
├── Size
│   ├── X: 1024
│   ├── Y: 256
│   └── Z: 1024
│
├── Chunks
│   ├── Loaded: 481
│   └── Generated: 1024
│
└── Memory
    └── 128 MB
```

A GPU resource could expose:

```text
GPU Buffer
├── Size: 256 MB
├── Usage: Storage
├── Memory: Device Local
├── Handle: 0x...
└── State: Ready
```

Specialized resources can provide custom visualizers.

For example:

```text
Texture
    → Image preview

Mesh
    → 3D preview

World
    → World/chunk visualization

GPU Buffer
    → Buffer/resource viewer
```

The Inspector does not need to understand these resources itself.

The resource or module can provide the information and visualization required to inspect it.

---

## Custom Inspector Interfaces

Simple types can be inspected automatically:

```text
bool
int
float
string
enum
vector
struct
array
handle
```

More complicated modules can provide custom Inspector interfaces.

For example, a Renderer could expose:

```text
Renderer
├── Statistics
│   ├── FPS
│   ├── Frame Time
│   ├── Draw Calls
│   └── Triangles
│
├── GPU Memory
│   ├── Buffers
│   ├── Textures
│   └── Allocations
│
└── Debug
    ├── Wireframe
    ├── Bounding Boxes
    └── Chunk Borders
```

An ECS module could expose:

```text
ECS
├── Entities
├── Components
├── Systems
└── Archetypes
```

The Inspector remains generic while modules provide domain-specific information.

---

## Inspector Commands

The Inspector can also expose actions rather than only variables.

For example:

```text
Physics

Gravity        9.81
Debug Draw     ✓

[Reset Simulation]
[Clear Bodies]
[Rebuild Broadphase]
```

These commands can be registered by modules and executed by the Inspector.

This allows development tools to interact with systems without requiring the Inspector to understand their internal implementation.

---

# Scheduler

LavaEngine does not require every Container to implement a fixed lifecycle such as:

```cpp
Start();
Update();
Render();
Stop();
```

Instead, individual Modules can register work with the framework's scheduler when necessary.

For example:

```text
Game Container
│
├── Physics ───── update
├── ECS ───────── update
├── Renderer ──── render
└── World ─────── no scheduled work
```

This means a data-only Container does not need to pretend to have an update loop.

The scheduler operates on the systems that actually need scheduling.

---

# Minimal Framework Core

The core of LavaEngine should remain intentionally small.

Conceptually:

```text
LavaEngine
├── Container
├── Module
├── Resource
├── ResourceHandle
├── Interface
├── Event
├── Scheduler
└── Output
```

Features such as:

```text
ECS
Physics
Audio
Networking
UI
Rendering
Animation
Scripting
Asset Management
```

should be implemented as optional modules rather than being fundamental requirements of the framework.

This keeps the core flexible and prevents LavaEngine from becoming tied to a particular game architecture.

---

# Container Graph

Containers do not necessarily need to form a strict hierarchy.

They can instead form a graph of dependencies and shared resources.

For example:

```text
              World
             /     \
            ↓       ↓
        Renderer   Physics
            │
            ↓
       Window Output
```

Or:

```text
             LavaEngine
             /       \
            ↓         ↓
          Game      Editor
            \         /
             \       /
                ↓
              World
```

This allows multiple independent systems to work with the same underlying resources without forcing them into a traditional scene hierarchy.

---

# Design Principles

LavaEngine is built around several principles.

### 1. No mandatory game architecture

LavaEngine does not require ECS, scenes, entities, components, game objects, or any other particular design pattern.

### 2. Containers are generic

A Container can represent anything the developer needs.

### 3. Modules are optional

Functionality is added explicitly through modules.

### 4. Resources are independent

Resources are not inherently tied to a particular module or Container.

### 5. Communication is explicit

Resources, interfaces, and events provide controlled communication between Containers.

### 6. Avoid unnecessary copying

Resource handles and shared resources allow large resources to be shared efficiently.

### 7. Keep the core small

The framework should provide fundamental infrastructure rather than implementing every possible engine feature.

### 8. Make systems inspectable

Modules and resources should be able to expose useful runtime information through the Inspector.

### 9. Provide escape hatches

Advanced developers should be able to access lower-level systems when the abstractions provided by LavaEngine are insufficient.

### 10. The developer owns the architecture

LavaEngine provides building blocks. The developer decides how those building blocks are organized.

---

# Philosophy

LavaEngine follows one fundamental principle:

> **The framework should make architecture possible, not make architecture mandatory.**

LavaEngine provides the infrastructure for composing systems, managing resources, connecting modules, communicating between isolated contexts, and inspecting applications at runtime.

The developer decides what those systems actually mean and how they are organized.

A developer can therefore use LavaEngine to build:

```text
A conventional game
A custom ECS architecture
A level editor
A simulation
A dedicated server
An asset pipeline
A rendering experiment
A completely custom engine
```

without forcing any of them into the same predefined architecture.

LavaEngine is not intended to be a smaller version of Unreal or Unity.

It is intended to be a **flexible foundation upon which different engine architectures can be built**.