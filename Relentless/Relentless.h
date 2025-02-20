#pragma once

//Core:
#include <src/Core/Application.h>
#include <src/Core/Core.h>
#include <src/Core/Time.h>
#include <src/Core/Any.h>

//Logging:
#include <src/Core/Log.h>

//Utility:
#include <src/Utility/FileDialogs.h>
#include <src/Utility/Common.h>
#include "src/Utility/StringUtils.h"
#include <src/Callback/Broadcaster.h>
#include <src/Callback/Callback.h>

//Events:
#include <src/ImGui/ImguiLayer.h>
#include <src/EventSystem/Layer.h>
#include <src/EventSystem/IEvent.h>
#include <src/EventSystem/KeyboardEvents.h>

//UI:
#include <src/UI/UI.h>
#include <src/UI/Tree/Tree.h>
#include <src/UI/Tree/TreeItem.h>
#include <src/UI/Tree/TreeStyle.h>
#include <src/UI/Tree/TreeInteraction.h>
#include <src/UI/Tree/TreeTypes.h>
#include <src/UI/DragDropBehavior.h>

//Graphics:
#include <src/Graphics/D3D12Core.h>
#include <src/Graphics/Renderer/RenderCommand.h>
#include <src/Graphics/Renderer/RenderUtility.h>
#include <src/Graphics/Renderer/Renderer3D.h>
#include <src/Graphics/Renderer/MasterRenderer.h>
#include <src/Graphics/Renderer/SceneRenderer.h>
#include <src/Graphics/Renderer/UtilityRenderer.h>
#include <src/Graphics/MemoryManager.h>
#include <src/Graphics/RHI/Device.h>
#include <src/Graphics/RHI/ResourceViews.h>
#include <src/Graphics/RHI/CommandContext.h>
#include <src/Graphics/Renderer/RenderTypes.h>

//Resources
#include <src/Graphics/Resources/Texture.h>
#include <src/Graphics/Resources/VertexBuffer.h>
#include <src/Graphics/Resources/IndexBuffer.h>
#include <src/Assets/AssetManager.h>
#include <src/Mesh/Mesh.h>

//IO
#include <src/Core/Window.h>
#include <src/Input/Mouse.h>
#include <src/Input/Keyboard.h>

//Mesh-related:
#include <src/Mesh/Vertex.h>

//Camera
#include <src/Graphics/Renderer/Camera/PerspectiveCamera.h>

//ECS
#include <src/Scene/Scene.h>
#include <src/ECS/EntityManager.h>
#include <src/ECS/ECSCommon.h>

//Serialization
#include <src/Scene/SceneSerializer.h>