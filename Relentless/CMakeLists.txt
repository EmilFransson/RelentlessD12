#Relentless/Relentless

set(SourcesList "src/rlspch.h" "Relentless.h" "src/Core/Application.h" "src/Core/Application.cpp" "EntryPoint.h"
"src/EventSystem/IEvent.h" "src/EventSystem/MouseEvents.h" "src/EventSystem/WindowEvents.h" "src/EventSystem/Layer.h" 
"src/EventSystem/LayerStack.h" "src/EventSystem/LayerStack.cpp" "src/Core/Core.h" "src/EventSystem/KeyboardEvents.h"
 "src/EventSystem/EventBus.h" "src/EventSystem/EventBus.cpp"  "src/Input/Keyboard.h" 
"src/Input/Keyboard.cpp" "src/EventSystem/EventPublisher.h" "src/EventSystem/EventPublisher.cpp" 
"src/Input/Mouse.h" "src/Input/Mouse.cpp" "src/Core/Log.h" "src/Core/Log.cpp"  
 "src/Core/Utility.h" "src/Core/Utility.cpp"  
 "src/Graphics/d3dx12.h"  
   
   
 "src/ImGui/ImguiLayer.h" "src/ImGui/ImguiLayer.cpp" 
    
  
  
   
"src/Mesh/Vertex.h" "src/Graphics/Renderer/Camera/Camera.h" "src/Graphics/Renderer/Camera/Camera.cpp" 
"src/Graphics/Renderer/Camera/PerspectiveCamera.h" "src/Graphics/Renderer/Camera/PerspectiveCamera.cpp" 
"src/Core/Time.h" "src/Core/Time.cpp"  
  
   
"src/ECS/EntityManager.h"
"src/ECS/Component.h" "src/ECS/Component.cpp" "src/ECS/ISystem.h" "src/Scene/Scene.h" "src/Scene/Scene.cpp" "src/Graphics/Shaders/Shader.h" "src/Graphics/Shaders/Shader.cpp" "src/Graphics/Shaders/ShaderCompiler.h" "src/Graphics/Shaders/ShaderCompiler.cpp" "src/Graphics/Shaders/ShaderLibrary.h" "src/Graphics/Shaders/ShaderLibrary.cpp" "src/Utility/FileDialogs.h" "src/Utility/FileDialogs.cpp" "src/Core/Profiler.h" "src/Core/Profiler.cpp" "src/Graphics/Renderer/SceneRenderer.h" "src/Graphics/Renderer/SceneRenderer.cpp" "src/Assets/CoreTypes/Material.h" "src/Assets/CoreTypes/Material.cpp"   "src/Assets/CoreTypes/Mesh.h" "src/Assets/CoreTypes/Mesh.cpp" "src/Utility/SerializeUtilities.h" "src/Utility/SerializeUtilities.cpp" "src/ECS/ECSCommon.h" "src/Assets/AssetRegistry.h" "src/Assets/AssetRegistry.cpp" "src/Assets/AssetMeta.h" "src/Assets/AssetMeta.cpp" "src/Assets/AssetManager.h" "src/Assets/AssetManager.cpp"           "src/Utility/Common.h" "src/Utility/Common.cpp" "src/Threading/ThreadPool.h"  "src/Threading/ThreadSafeQueue.h"  "src/Callback/Broadcaster.h"  "src/Utility/SystemPaths.h" "src/Utility/SystemPaths.cpp" "src/Utility/Rules.h" "src/Utility/FilepathUtils.h" "src/Utility/FilepathUtils.cpp" "src/File/File.h" "src/File/File.cpp"   "src/Math/CommonMath.h" "src/Graphics/Renderer/UtilityRenderer.h" "src/Graphics/Renderer/UtilityRenderer.cpp" "src/Callback/Callback.h" "src/Utility/Serializer.h" "src/Utility/Serializer.cpp" "src/Math/MathTypes.h" "src/Core/Any.h" "src/Utility/StringUtils.h" "src/Utility/StringUtils.cpp" "src/Graphics/RHI/Device.h" "src/Graphics/RHI/Device.cpp" "src/Core/Ref.h" "src/Graphics/RHI/D3D.h" "src/Graphics/RHI/CommandContext.h" "src/Graphics/RHI/CommandContext.cpp" "src/Graphics/RHI/DeviceResource.h" "src/Graphics/RHI/DeviceResource.cpp" "src/Graphics/RHI/Buffer.h" "src/Graphics/RHI/Buffer.cpp" "src/Core/CoreTypes.h" "src/Graphics/RHI/RHI.h" "src/Graphics/RHI/RHI.cpp" "src/Graphics/RHI/ResourceViews.h" "src/Graphics/RHI/ResourceViews.cpp" "src/Graphics/RHI/PipelineState.h" "src/Graphics/RHI/PipelineState.cpp" "src/Graphics/RHI/Fence.h" "src/Graphics/RHI/Fence.cpp" "src/Graphics/RHI/CommandQueue.h" "src/Graphics/RHI/CommandQueue.cpp" "src/Graphics/RHI/Texture.h" "src/Graphics/RHI/Texture.cpp" "src/Graphics/RHI/RootSignature.h" "src/Graphics/RHI/RootSignature.cpp" "src/Graphics/Shaders/Interop/ShaderInterop.h" "src/Graphics/RHI/ScratchAllocator.h" "src/Graphics/RHI/ScratchAllocator.cpp" "src/Graphics/RHI/RingBufferAllocator.h" "src/Graphics/RHI/RingBufferAllocator.cpp" "src/Graphics/RHI/DescriptorManager.h" "src/Graphics/RHI/DescriptorManager.cpp" "src/Graphics/RHI/DescriptorHeap.h" "src/Graphics/RHI/DescriptorHeap.cpp" "src/Graphics/Renderer/Renderer.h" "src/Graphics/Renderer/Renderer.cpp" "src/Graphics/Renderer/RenderTypes.h" "src/Graphics/Renderer/Techniques/ForwardRenderer.h" "src/Graphics/Renderer/Techniques/ForwardRenderer.cpp" "src/Core/Span.h"   "src/Graphics/RHI/Swapchain.h" "src/Graphics/RHI/Swapchain.cpp" "src/Core/Window.h" "src/Core/Window.cpp" "src/Graphics/Renderer/Techniques/ToyRenderer.h" "src/Graphics/Renderer/Techniques/ToyRenderer.cpp" "src/Core/MinWindows.h" "src/File/FilePath.h" "src/Graphics/Renderer/Techniques/EditorGrid.h" "src/Graphics/Renderer/Techniques/EditorGrid.cpp" "src/Graphics/Renderer/Techniques/PostProcessing.h" "src/Graphics/Renderer/Techniques/PostProcessing.cpp" "src/Math/CommonMath.cpp" "src/Graphics/Renderer/Techniques/DepthPrePass.h" "src/Graphics/Renderer/Techniques/DepthPrePass.cpp" "src/Graphics/Renderer/Techniques/HBAOPlus.h" "src/Graphics/Renderer/Techniques/HBAOPlus.cpp" "src/Graphics/Renderer/Techniques/Outlines.h" "src/Graphics/Renderer/Techniques/Outlines.cpp" "src/Assets/IAsset.h" "src/Utility/TextFilterExpressionEvaluator.h" "src/Utility/TextFilterExpressionEvaluator.cpp" "src/ECS/Systems/DeferredEntityDeletionSystem.h" "src/ECS/Systems/DeferredEntityDeletionSystem.cpp" "src/Core/Folder.h" "src/Core/Folder.cpp" "src/Graphics/Renderer/Techniques/AutoExposure.h" "src/Graphics/Renderer/Techniques/AutoExposure.cpp" "src/Assets/IAsset.cpp" "src/Project/Project.h" "src/Project/Project.cpp" "src/Module/ModuleManager.h" "src/Module/ModuleManager.cpp" "src/Module/IModule.h" "src/Module/IModule.cpp" "src/Assets/CoreTypes/Texture2D.h" "src/Assets/CoreTypes/Texture2D.cpp" "src/Subsystem/ISubsystem.h"  "src/Subsystem/ISystemManager.h"  "src/Module/AssetRegistryModule.h" "src/Module/AssetRegistryModule.cpp" "src/Serialization/Archive.h" "src/Serialization/Archive.cpp" "src/Core/StaticTypeInfo.h" "src/DataStructure/DenseSet.h" "src/Assets/Factory/IFactory.h" "src/Assets/Factory/IFactory.cpp" "src/Assets/ImportSettings.h" "src/Assets/Factory/FeedbackContext.h" "src/Assets/Factory/FeedbackContext.cpp" "src/Module/AssetToolsModule.h" "src/Module/AssetToolsModule.cpp"   "src/ECS/ISystem.cpp" "src/Module/NativeScriptingModule.h" "src/Module/NativeScriptingModule.cpp" "src/Process/Process.h" "src/Core/EngineMain.h" "src/Core/EngineMain.cpp" "src/Core/DLLExport.h")

set(LibraryName "Relentless")

set(YamlIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/Yaml")
set(HBAOIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/HBAO+")

set (ExternalIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/")
set (ExternalLibPath "${CMAKE_SOURCE_DIR}/Relentless/vendor/libs")

add_library("${LibraryName}" SHARED "${SourcesList}")

target_include_directories("${LibraryName}" PUBLIC "${CMAKE_SOURCE_DIR}/Relentless/src/")
target_include_directories("${LibraryName}" PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories("${LibraryName}" SYSTEM PUBLIC "${ExternalIncludePath}" "${YamlIncludePath}" "${HBAOIncludePath}")

target_compile_options("${LibraryName}" PRIVATE "/W4")

set_target_properties("${LibraryName}" PROPERTIES LINKER_LANGUAGE "CXX")
target_link_libraries("${LibraryName}" PRIVATE "d3d12" "dxgi" "dxguid" "dxcompiler" "d3dcompiler" "Rpcrt4" "windowscodecs")

target_precompile_headers("${LibraryName}" PUBLIC "src/rlspch.h")

##### Section for linking external libraries #####
file(GLOB_RECURSE libFiles
      ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.lib)

foreach(var ${libFiles})
    target_link_libraries("${LibraryName}" PRIVATE "${var}")
endforeach()

file(GLOB_RECURSE dlls
      ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.dll ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.pdb)

foreach(var ${dlls})
    add_custom_command(
    TARGET "${LibraryName}" POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${var}"
    ${CMAKE_BINARY_DIR}/Relentless-Editor/

    )
endforeach()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12")

add_custom_command(
    TARGET ${LibraryName} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12Core.dll"
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12/"
)

add_custom_command(
    TARGET ${LibraryName} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
    "${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12SDKLayers.dll"
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12/"
)

target_compile_definitions(${LibraryName} PRIVATE RLS_BUILD_DLL)
target_compile_definitions(${LibraryName} PRIVATE STATIC_TYPE_INFO_USE_MEMBER_POINTER=0)
target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:DEBUG>:RLS_DEBUG>")
target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:RELEASE>:RLS_RELEASE>")
target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:RLS_RELWITHDEBINFO>")
target_compile_definitions(${LibraryName} PUBLIC SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Shaders/HLSL/")
target_compile_definitions(${LibraryName} PUBLIC YAML_CPP_STATIC_DEFINE)

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH1 "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "ENGINE_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH1}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH2 "${CMAKE_SOURCE_DIR}/Relentless-Editor/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH2}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH3 "${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(${LibraryName} PUBLIC "MAIN_ENGINE_DIRECTORY=\"${WINDOWS_STYLE_PATH3}\"")
