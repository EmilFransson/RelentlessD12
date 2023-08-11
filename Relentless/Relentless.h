#pragma once

//Core:
#include <src/Core/Application.h>
#include <src/Core/Core.h>
#include <src/Core/Timer.h>

//Logging:
#include <src/Core/Log.h>

//Utility:
#include <src/Utility/FileDialogs.h>

//Events:
#include <src/ImGui/ImguiLayer.h>
#include <src/EventSystem/Layer.h>
#include <src/EventSystem/IEvent.h>
#include <src/EventSystem/KeyboardEvents.h>

//Graphics:
#include <src/Graphics/D3D12Core.h>
#include <src/Graphics/Renderer/RenderCommand.h>
#include <src/Graphics/Renderer/RenderUtility.h>
#include <src/Graphics/Renderer/Renderer3D.h>
#include <src/Graphics/Renderer/MasterRenderer.h>
#include <src/Graphics/Renderer/SceneRenderer.h>
#include <src/Graphics/MemoryManager.h>

//Resources
#include <src/Graphics/Resources/Texture.h>
#include <src/Graphics/Resources/VertexBuffer.h>
#include <src/Graphics/Resources/IndexBuffer.h>
#include <src/Graphics/Resources/AssetManager.h>
#include <src/Mesh/Mesh.h>

//IO
#include <src/Core/Window.h>
#include <src/Input/Mouse.h>
#include <src/Input/Keyboard.h>

//Mesh-related:
#include <src/Mesh/Vertex.h>
#include <src/Mesh/MeshFactory.h>

//Camera
#include <src/Graphics/Renderer/Camera/PerspectiveCamera.h>
#include <src/Graphics/Renderer/Camera/OrthographicCamera.h>

//ECS
#include <src/Scene/Scene.h>
#include <src/Scene/SceneSerializer.h>
#include <src/ECS/EntityManager.h>