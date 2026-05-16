<img width="2559" height="1439" alt="Relentless_Overview" src="https://github.com/user-attachments/assets/b1d69068-26a9-4423-a91a-ae8784b0120c" />

# Relentless - A D3D12 Game Engine
Relentless is an explorative game engine project with a personal goal of researching and experimenting with modern techniques and ideas pertaining to the development of a game engine, from scratch. This repo also serves as a hallmark portfolio piece, for showcasing work done and committed over multiple years. The project is under active development.

# Features
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
  
