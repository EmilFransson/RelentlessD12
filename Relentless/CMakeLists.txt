#Relentless/Relentless

set(SourcesList "src/rlspch.h" "Relentless.h" "src/Core/Application.h" "src/Core/Application.cpp" "EntryPoint.h"
"src/EventSystem/IEvent.h" "src/EventSystem/MouseEvents.h" "src/EventSystem/WindowEvents.h" "src/EventSystem/Layer.h" 
"src/EventSystem/LayerStack.h" "src/EventSystem/LayerStack.cpp" "src/Core/Core.h" "src/EventSystem/KeyboardEvents.h"
 "src/EventSystem/EventBus.h" "src/EventSystem/EventBus.cpp"  "src/Input/Keyboard.h" 
"src/Input/Keyboard.cpp" "src/EventSystem/EventPublisher.h" "src/EventSystem/EventPublisher.cpp" 
"src/Input/Mouse.h" "src/Input/Mouse.cpp" "src/Core/Log.h" "src/Core/Log.cpp" "src/Core/Window.h" 
"src/Core/Window.cpp" "src/Core/Utility.h" "src/Core/Utility.cpp" "src/Graphics/D3D12Core.h" 
"src/Graphics/D3D12Core.cpp" "src/Graphics/d3dx12.h" "src/Graphics/D3D12Debug.h" 
"src/Graphics/D3D12Command.h" "src/Graphics/D3D12Command.cpp" "src/Graphics/D3D12Debug.cpp" 
"src/Graphics/DescriptorHeap.h" "src/Graphics/DescriptorHeap.cpp" "src/Graphics/Renderer/RenderCommand.h" 
"src/Graphics/Renderer/RenderCommand.cpp" "src/ImGui/ImguiLayer.h" "src/ImGui/ImguiLayer.cpp" 
"src/Graphics/Renderer/Renderer3D.h"  "src/Graphics/Renderer/Renderer3D.cpp" "src/Graphics/Resources/IResource.h" 
"src/Graphics/Resources/IResource.cpp" "src/Graphics/Resources/Texture.h" "src/Graphics/Resources/Texture.cpp" 
"src/Graphics/MemoryManager.h" "src/Graphics/MemoryManager.cpp" "src/Graphics/Resources/DepthStencil.h"
"src/Graphics/Resources/DepthStencil.cpp"   
"src/Mesh/Vertex.h" "src/Graphics/Renderer/Camera/ICamera.h" "src/Graphics/Renderer/Camera/ICamera.cpp" 
"src/Graphics/Renderer/Camera/PerspectiveCamera.h" "src/Graphics/Renderer/Camera/PerspectiveCamera.cpp" 
"src/Core/Timer.h" "src/Core/Timer.cpp" "src/Graphics/Renderer/Camera/OrthographicCamera.h" 
"src/Graphics/Renderer/Camera/OrthographicCamera.cpp"  
   
"src/ECS/EntityManager.h"
"src/ECS/Component.h" "src/ECS/Component.cpp" "src/ECS/System.h" "src/Scene/Scene.h" "src/Scene/Scene.cpp" "src/Graphics/Shaders/Shader.h" "src/Graphics/Shaders/Shader.cpp" "src/Graphics/Shaders/ShaderCompiler.h" "src/Graphics/Shaders/ShaderCompiler.cpp" "src/Graphics/Shaders/ShaderLibrary.h" "src/Graphics/Shaders/ShaderLibrary.cpp" "vendor/includes/ImGuizmo/ImCurveEdit.cpp" "vendor/includes/ImGuizmo/ImCurveEdit.h" "vendor/includes/ImGuizmo/ImGradient.cpp" "vendor/includes/ImGuizmo/ImGradient.h" "vendor/includes/ImGuizmo/ImGuizmo.cpp" "vendor/includes/ImGuizmo/ImGuizmo.h" "vendor/includes/ImGuizmo/ImSequencer.cpp" "vendor/includes/ImGuizmo/ImSequencer.h" "src/Graphics/Resources/VertexBuffer.h" "src/Graphics/Resources/VertexBuffer.cpp" "src/Graphics/Resources/IndexBuffer.h" "src/Graphics/Resources/IndexBuffer.cpp" "src/Graphics/Resources/UploadBuffer.h" "src/Graphics/Resources/UploadBuffer.cpp" "src/Graphics/Resources/ConstantBuffer.h" "src/Graphics/Resources/ConstantBuffer.cpp"   "src/Graphics/Resources/StructuredBuffer.h" "src/Graphics/Resources/StructuredBuffer.cpp" "src/Scene/LightManager.h" "src/Scene/LightManager.cpp" "src/Utility/FileDialogs.h" "src/Utility/FileDialogs.cpp" "src/Core/Profiler.h" "src/Core/Profiler.cpp" "src/Graphics/Renderer/RenderPass.h" "src/Graphics/Renderer/RenderPass.cpp" "src/Graphics/Renderer/Pipeline.h" "src/Graphics/Renderer/Pipeline.cpp" "src/Graphics/Renderer/FrameBuffer.h" "src/Graphics/Renderer/FrameBuffer.cpp" "src/Graphics/Renderer/Properties.h" "src/Graphics/Renderer/SceneRenderer.h" "src/Graphics/Renderer/SceneRenderer.cpp" "src/Graphics/Renderer/MasterRenderer.h" "src/Graphics/Renderer/MasterRenderer.cpp" "src/Graphics/Renderer/RenderUtility.h" "src/Graphics/Renderer/RenderUtility.cpp" "src/Graphics/Renderer/Properties.cpp" "src/Graphics/Shaders/ShaderReflector.h" "src/Graphics/Shaders/ShaderReflector.cpp" "src/Graphics/Resources/Buffer.h" "src/Graphics/Resources/Buffer.cpp" "src/Graphics/Resources/Material.h" "src/Graphics/Resources/Material.cpp" "src/Scene/SceneSerializer.h" "src/Scene/SceneSerializer.cpp" "src/Mesh/Mesh.h" "src/Mesh/Mesh.cpp"       "src/Utility/SerializeUtilities.h" "src/Utility/SerializeUtilities.cpp"       "src/ECS/ECSCommon.h"   "src/Assets/AssetRegistry.h" "src/Assets/AssetRegistry.cpp" "src/Assets/AssetMeta.h" "src/Assets/AssetMeta.cpp" "src/Assets/AssetManager.h" "src/Assets/AssetManager.cpp"     "src/Assets/Serializer.h" "src/Assets/Serializer.cpp"   "src/Assets/Importer.h" "src/Assets/Importer.cpp" "src/Assets/ImportSettings.h" "src/Utility/Common.h" "src/Utility/Common.cpp")

set(LibraryName "Relentless")


set(YamlIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/Yaml")
set(HBAOIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/HBAO+")

set (ExternalIncludePath "${CMAKE_SOURCE_DIR}/Relentless/vendor/includes/")
set (ExternalLibPath "${CMAKE_SOURCE_DIR}/Relentless/vendor/libs")


add_library("${LibraryName}" STATIC "${SourcesList}")

target_include_directories("${LibraryName}" PRIVATE "${CMAKE_SOURCE_DIR}/Relentless/src/")
target_include_directories("${LibraryName}" PRIVATE "${CMAKE_SOURCE_DIR}" "${ExternalIncludePath}" "${YamlIncludePath}" "${HBAOIncludePath}")
target_compile_options("${LibraryName}" PRIVATE "/W4")
set_target_properties("${LibraryName}" PROPERTIES LINKER_LANGUAGE "CXX")
target_link_libraries("${LibraryName}" PRIVATE "d3d12" "dxgi" "dxguid" "dxcompiler" "d3dcompiler" "Rpcrt4" "windowscodecs")

target_precompile_headers("${LibraryName}" PUBLIC "src/rlspch.h")

if(NOT BUILD_SHARED_LIBS)
    set(STATIC_POSTFIX "-static")
endif()

set_target_properties("${LibraryName}"
    PROPERTIES
    DEBUG_POSTFIX "${STATIC_POSTFIX}-debug"
    RELEASE_POSTFIX "${STATIC_POSTFIX}"
)

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
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${var}"
    ${CMAKE_BINARY_DIR}/Relentless-Editor/

    )
endforeach()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12")

add_custom_command(
    TARGET ${LibraryName} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12Core.dll"
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12/"
)

add_custom_command(
    TARGET ${LibraryName} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy 
    "${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12SDKLayers.dll"
    "${CMAKE_BINARY_DIR}/Relentless-Editor/D3D12/"
)

target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:DEBUG>:RLS_DEBUG>")
target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:RELEASE>:RLS_RELEASE>")
target_compile_definitions(${LibraryName} PUBLIC SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Shaders/HLSL/")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH1 "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "ENGINE_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH1}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH2 "${CMAKE_SOURCE_DIR}/Relentless-Editor/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH2}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH3 "${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(${LibraryName} PUBLIC "MAIN_ENGINE_DIRECTORY=\"${WINDOWS_STYLE_PATH3}\"")
