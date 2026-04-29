#pragma once

//Core:
#include <src/Core/Any.h>
#include <src/Core/Application.h>
#include <src/Utility/Common.h>
#include <src/Core/Core.h>
#include <src/Core/CoreTypes.h>
#include <src/Core/DLLExport.h>
#include <src/Core/EngineMain.h>
#include <src/Core/Log.h>
#include <src/Core/Ref.h>
#include <src/Core/StaticTypeInfo.h>
#include <src/Core/Time.h>
#include <src/Core/Window.h>

//Data structures:
#include <src/DataStructure/DenseSet.h>

//Callback:
#include <src/Callback/Broadcaster.h>
#include <src/Callback/Callback.h>
#include <src/Callback/CoreBroadcasters.h>

//Utility:
#include <src/Utility/FileDialogs.h>
#include <src/Utility/FilepathUtils.h>
#include <src/Utility/StringUtils.h>
#include <src/Utility/TextFilterExpressionEvaluator.h>

//Factory:
#include <src/Assets/Factory/FeedbackContext.h>
#include <src/Assets/Factory/IFactory.h>

//Files & paths
#include <src/File/File.h>
#include <src/File/FilePath.h>
#include <src/Utility/SystemPaths.h>

//Events:
#include <src/EventSystem/Layer.h>
#include <src/EventSystem/IEvent.h>
#include <src/EventSystem/KeyboardEvents.h>
#include <src/EventSystem/MouseEvents.h>

//Modules:
#include <src/Module/AssetRegistryModule.h>
#include <src/Module/AssetToolsModule.h>
#include <src/Module/IModule.h>
#include <src/Module/ModuleManager.h>
#include <src/Module/RenderModule.h>

//Graphics:
#include <src/Graphics/Renderer/Renderer.h>
#include <src/Graphics/Renderer/RenderTypes.h>
#include <src/Graphics/RHI/Device.h>
#include <src/Graphics/RHI/ResourceViews.h>
#include <src/Graphics/RHI/CommandContext.h>
#include <src/Graphics/RHI/Swapchain.h>
#include <src/Graphics/Renderer/Service/RenderBakeService.h>
#include <src/Graphics/Renderer/RenderQualitySettings.h>

//Assets:
#include <src/Assets/AssetManager.h>
#include <src/Assets/IAsset.h>
#include <src/Assets/ImportSettings.h>

//Core Asset types
#include <src/Assets/CoreTypes/Environment.h>
#include <src/Assets/CoreTypes/Material.h>
#include <src/Assets/CoreTypes/Mesh.h>
#include <src/Assets/CoreTypes/Texture2D.h>
#include <src/Assets/CoreTypes/TextureCube.h>

//Threading:
#include <src/Threading/ThreadPool.h>

//IO
#include <src/Input/Mouse.h>
#include <src/Input/Keyboard.h>

//Mesh-related:
#include <src/Mesh/Vertex.h>

//Camera
#include <src/Graphics/Renderer/Camera/PerspectiveCamera.h>

//ECS
#include <src/ECS/Component.h>
#include <src/ECS/EntityManager.h>
#include <src/ECS/ECSCommon.h>
#include <src/ECS/ISystem.h>
#include <src/ECS/Components/LightComponent.h>
#include <src/ECS/Components/SkyBoxComponent.h>
#include <src/ECS/Components/SkyLightComponent.h>
#include <src/ECS/Components/TransformComponent.h>
#include <src/Scene/Scene.h>

//Serialization
#include <src/Serialization/Archive.h>

//Subsystem:
#include <src/Subsystem/ISystemManager.h>
#include <src/Subsystem/ISubsystem.h>

//Project:
#include <src/Project/Project.h>

//Memory:
#include <src/Memory/ArenaAllocator.h>
#include <src/Memory/ArenaVector.h>
#include <src/Memory/SharedFromThis.h>

//Property:
#include <Property/PropertyUtils.h>