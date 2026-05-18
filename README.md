<img width="2559" height="1439" alt="Relentless_Overview" src="https://github.com/user-attachments/assets/b1d69068-26a9-4423-a91a-ae8784b0120c" />

# Relentless - A D3D12 Game Engine
Relentless is an explorative game engine project with a personal goal of researching and experimenting with modern techniques and ideas pertaining to the development of a game engine, from scratch. This repo also serves as a hallmark portfolio piece, for showcasing work done and committed over multiple years. The project is under active development.

# Noteable Features
- **D3D12 Render Backend:**
  - Physically Based Rendering (PBR), Microfacet BRDF
  - Complex material support and authoring based on metalness workflow
  - Opaque, Alpha Masked & Alpha Blended rendering, including two sided support
  - Instanced rendering, automatic batching and sorting
  - Screen space refraction for transluscent objects, with IOR support (depth awareness is wip)
  - Fully bindless resources (direct descriptor heap indexing), no input layouts
  - Histogram Based Auto Exposure (eye adaptation)
  - HBAO+ SSAO (NVidia Gameworks integration)
  - Pixel perfect picking & selection outlines
  - Skybox environments, with support for blending, tinting & more
  - Skylight/IBL evaluation with user defined update & quality policies. Blending, tinting & more supported.
  - Multi-viewport editor architecture
  - FNV-1A hashed PSO cache for declarative pipeline state authoring
  - Game/Render thread proxy dispatch model (**wip, actual threading not yet active**)
  - Multi Sampled Anti Aliasing (MSAA)

- **Entity Component System (ECS)**
  - Pure; Entities are ids, Components are data, Systems are stateless component mutators
  - Based on the sparse set data structure for O(1) look-up, removal and insertion and O(n) iteration
  - Dirty-tagging used throughout for efficient system iterations

- **Editor**
  - ImGui backend
  - Custom declarative wrapper implemented for a full suite of widget types and sizing/alignment policies
  - Entity & Folder Outliner panel
  - Details panel for displaying component data
  - Material & Environment authoring panels
  - Content Browser panel with asset thumbnails (wip)
  - Drag/Drop support

- **Asset pipelines**
  - Synchronous & Asynchronous asset importing
  - Custom binary format for efficient asset (de-)serialization & GPU uploads
  - Per-asset-type factories for specializing and customizing asset imports (including optimizations, compression, etc...)
  
- **Architecture**
  - Engine built as DLL, editor consumes
  - Subsystems: automatically instanced classes with managed lifetime
  - Modules: basic building blocks for code organization and encapsulating functionality. Future goal includes compiling as separate compilation units (plugins...)
  - Threadpool for efficient work distribution

# Building
### Dependencies
- Windows SDK 10.0.26100.0
- Shader Model 6.6 compatible GPU
#### Build Tools
- [Visual Studio 2026](https://visualstudio.microsoft.com/)
- [Clang 21.1.8](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.8) (Standalone LLVM. Download, install & add to PATH)
- [CMake](https://cmake.org/download/) (C++26 compatible version - 4.1 or later)
#### Third-Party Libraries
Relentless integrates the following third-party libraries:
  1. [ImGui](https://github.com/ocornut/imgui) (subdirectory in /vendor)
  2. [ImGuizmo](https://github.com/cedricguillemet/imguizmo) (subdirectory in /vendor)
  3. [assimp](https://github.com/assimp/assimp) (subdirectory in /vendor)
  4. [meshoptimizer](https://github.com/zeux/meshoptimizer) (subdirectory in /vendor)
  5. [yaml-cpp](https://github.com/jbeder/yaml-cpp/) (subdirectory in /vendor)
  6. [HBAO+](https://github.com/NVIDIAGameWorks/HBAOPlus) (subdirectory in /vendor)
  7. [dense_hash_map](https://github.com/Jiwan/dense_hash_map) (subdirectory in /vendor)
  8. [StaticTypeInfo](https://github.com/TheLartians/StaticTypeInfo) (subdirectory in /vendor)
  9. [DirectXTK](https://github.com/microsoft/directxtk) (subdirectory in /vendor)
  10. [DirectXTex](https://github.com/microsoft/DirectXTex) (subdirectory in /vendor)
  11. [DirectXShaderCompiler](https://github.com/microsoft/directxshadercompiler) (subdirectory in /vendor)
  12. [DirectX12 Agility SDK](https://devblogs.microsoft.com/directx/directx12agility/) (subdirectory in /vendor)
  13. [IconFontCppHeaders](https://github.com/juliettef/iconfontcppheaders) (subdirectory in /vendor)
  14. [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) (Header only in /vendor)
  15. [spdlog](https://github.com/gabime/spdlog) (Retrieved via CMake FetchContent on configuration)
### Build Steps
1. Clone the repo:
  ```bash
   git clone --recursive https://github.com/EmilFransson/RelentlessD12.git
   cd RelentlessD12
```
2. Invoke build-and-run.bat:
 ```bash
   .\build-and-run.bat
```
# Usage
Note: Upon first start the user must currently self-assign and dock panels. Upon restarts this is handled automatically. Exit the application with File -> Exit.

### Scene Navigation & Transformation
With a scene viewport focused:
- Right Mouse Button + WASD: fly cam.
- Left Mouse Button: horizontal dollying.
- Left + Right Mouse Button: vertical dollying.
- Left Mouse Button + Alt: Orbiting. Change orbit offset by mouse scrolling.
- Left Mouse Button while hovering entity: selection. LCtrl for multi-selection.
- With 1+ entity selected: Q/W/E/R buttons to toggle between transformation gizmo modes.
- With 1+ entity selected: Delete button to delete entity.
- Alt + gizmo manipulation: entity duplication.
- H with 1+ entity selected: hide entity.
- LCtrl + H: set all entities visible. 
### Content Browser
Currently limited to (wip):
- Double clicking on a material or environment asset opens its corresponding editor panel.
### Details Panel
- Selecting an entity opens its corresponding details view, enabling component data editing. Bulk editing available.
### Outliner Panel
- Left Mouse Button: single selection.
- Left Mouse Button + LCtrl: toggle selection/add to selection.
- Left Mouse Button + LShift: range selection.
- F2 with entity selected: rename.
- Delete Button with 1+ entity selected: delete selection.
- Type in search bar: filter entity items.
- Clicking an entity eye-icon: toggle visibility.
- Note: folder functionality is currently broken in build (wip).
### Editor Panels
- Changes propagate directly to scene; asset data is scene-representative.
- LCrtl + S: Save asset.
- Drag & drop asset thumbnails from content browser to assign where applicable. Eligible drops are denoted with a green overlay.
# License
Source-available for viewing and evaluation. Copying, modification, and use
in other projects is not permitted without permission. See [LICENSE](LICENSE)
for full terms. Third-party libraries in `/vendor/` retain their original
licenses.
