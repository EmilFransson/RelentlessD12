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
"src/ECS/Component.h" "src/ECS/Component.cpp" "src/ECS/System.h" "src/Scene/Scene.h" "src/Scene/Scene.cpp" "src/Graphics/Shaders/Shader.h" "src/Graphics/Shaders/Shader.cpp" "src/Graphics/Shaders/ShaderCompiler.h" "src/Graphics/Shaders/ShaderCompiler.cpp" "src/Graphics/Shaders/ShaderLibrary.h" "src/Graphics/Shaders/ShaderLibrary.cpp" "vendor/includes/ImGuizmo/ImCurveEdit.cpp" "vendor/includes/ImGuizmo/ImCurveEdit.h" "vendor/includes/ImGuizmo/ImGradient.cpp" "vendor/includes/ImGuizmo/ImGradient.h" "vendor/includes/ImGuizmo/ImGuizmo.cpp" "vendor/includes/ImGuizmo/ImGuizmo.h" "vendor/includes/ImGuizmo/ImSequencer.cpp" "vendor/includes/ImGuizmo/ImSequencer.h"               "src/Utility/FileDialogs.h" "src/Utility/FileDialogs.cpp" "src/Core/Profiler.h" "src/Core/Profiler.cpp"        "src/Graphics/Renderer/SceneRenderer.h" "src/Graphics/Renderer/SceneRenderer.cpp"          "src/Graphics/Resources/Material.h" "src/Graphics/Resources/Material.cpp" "src/Scene/SceneSerializer.h" "src/Scene/SceneSerializer.cpp" "src/Mesh/Mesh.h" "src/Mesh/Mesh.cpp"       "src/Utility/SerializeUtilities.h" "src/Utility/SerializeUtilities.cpp"       "src/ECS/ECSCommon.h"   "src/Assets/AssetRegistry.h" "src/Assets/AssetRegistry.cpp" "src/Assets/AssetMeta.h" "src/Assets/AssetMeta.cpp" "src/Assets/AssetManager.h" "src/Assets/AssetManager.cpp"     "src/Assets/Serializer.h" "src/Assets/Serializer.cpp"   "src/Assets/Importer.h" "src/Assets/Importer.cpp" "src/Assets/ImportSettings.h" "src/Utility/Common.h" "src/Utility/Common.cpp" "src/UI/UI.h" "src/UI/UI.cpp" "src/Threading/ThreadPool.h"  "src/Threading/ThreadSafeQueue.h"  "src/Callback/Broadcaster.h"  "src/Utility/SystemPaths.h" "src/Utility/SystemPaths.cpp" "src/Utility/Rules.h" "src/Utility/FilepathUtils.h" "src/Utility/FilepathUtils.cpp" "src/File/File.h" "src/File/File.cpp"   "src/Math/CommonMath.h"        "src/Graphics/Renderer/UtilityRenderer.h" "src/Graphics/Renderer/UtilityRenderer.cpp" "src/Callback/Callback.h"     "src/Utility/Serializer.h" "src/Utility/Serializer.cpp" "src/UI/Tree/Tree.h" "src/UI/Tree/Tree.cpp" "src/UI/Tree/TreeItem.h" "src/UI/Tree/TreeItem.cpp"     "src/UI/Tree/TreeStyle.h" "src/UI/Tree/TreeStyle.cpp" "src/UI/Tree/TreeInteraction.h" "src/UI/Tree/TreeInteraction.cpp" "src/Math/MathTypes.h" "src/UI/Tree/TreeTypes.h" "src/UI/DragDropBehavior.h" "src/UI/DragDropBehavior.cpp" "src/Core/Any.h" "src/Utility/StringUtils.h" "src/Utility/StringUtils.cpp" "src/Graphics/RHI/Device.h" "src/Graphics/RHI/Device.cpp" "src/Core/Ref.h" "src/Graphics/RHI/D3D.h" "src/Graphics/RHI/CommandContext.h" "src/Graphics/RHI/CommandContext.cpp" "src/Graphics/RHI/DeviceResource.h" "src/Graphics/RHI/DeviceResource.cpp" "src/Graphics/RHI/Buffer.h" "src/Graphics/RHI/Buffer.cpp" "src/Core/CoreTypes.h" "src/Graphics/RHI/RHI.h" "src/Graphics/RHI/RHI.cpp" "src/Graphics/RHI/ResourceViews.h" "src/Graphics/RHI/ResourceViews.cpp" "src/Graphics/RHI/PipelineState.h" "src/Graphics/RHI/PipelineState.cpp" "src/Graphics/RHI/Fence.h" "src/Graphics/RHI/Fence.cpp" "src/Graphics/RHI/CommandQueue.h" "src/Graphics/RHI/CommandQueue.cpp" "src/Graphics/RHI/Texture.h" "src/Graphics/RHI/Texture.cpp" "src/Graphics/RHI/RootSignature.h" "src/Graphics/RHI/RootSignature.cpp" "src/Graphics/Shaders/Interop/ShaderInterop.h" "src/Graphics/RHI/ScratchAllocator.h" "src/Graphics/RHI/ScratchAllocator.cpp" "src/Graphics/RHI/RingBufferAllocator.h" "src/Graphics/RHI/RingBufferAllocator.cpp" "src/Graphics/RHI/DescriptorManager.h" "src/Graphics/RHI/DescriptorManager.cpp" "src/Graphics/RHI/DescriptorHeap.h" "src/Graphics/RHI/DescriptorHeap.cpp" "src/Graphics/Renderer/Renderer.h" "src/Graphics/Renderer/Renderer.cpp" "src/Graphics/Renderer/RenderTypes.h" "src/Graphics/Renderer/Techniques/ForwardRenderer.h" "src/Graphics/Renderer/Techniques/ForwardRenderer.cpp" "src/Core/Span.h"   "src/Graphics/RHI/Swapchain.h" "src/Graphics/RHI/Swapchain.cpp" "src/Graphics/RHI/Window.h" "src/Graphics/RHI/Window.cpp" "src/Graphics/Renderer/Techniques/ToyRenderer.h" "src/Graphics/Renderer/Techniques/ToyRenderer.cpp" "src/Core/MinWindows.h" "src/File/FilePath.h" "src/Graphics/Renderer/Techniques/EditorGrid.h" "src/Graphics/Renderer/Techniques/EditorGrid.cpp" "src/Graphics/Renderer/Techniques/PostProcessing.h" "src/Graphics/Renderer/Techniques/PostProcessing.cpp" "src/Math/CommonMath.cpp" "src/Graphics/Renderer/Techniques/DepthPrePass.h" "src/Graphics/Renderer/Techniques/DepthPrePass.cpp" "src/Graphics/Renderer/Techniques/HBAOPlus.h" "src/Graphics/Renderer/Techniques/HBAOPlus.cpp" "src/Graphics/Renderer/Techniques/Outlines.h" "src/Graphics/Renderer/Techniques/Outlines.cpp"  "src/Assets/Factory/ModelFactory.h" "src/Assets/Factory/ModelFactory.cpp" "src/Core/IAsset.h" "src/Assets/Factory/IFactory.h" "src/Assets/Factory/IFactory.cpp" "src/Assets/Factory/TextureFactory.h" "src/Assets/Factory/TextureFactory.cpp" "src/UI/IWidget.h" "src/UI/IWidget.cpp" "src/UI/CheckBox.h" "src/UI/CheckBox.cpp" "src/UI/VerticalBox.h" "src/UI/VerticalBox.cpp" "src/UI/CollapsibleSection.h" "src/UI/CollapsibleSection.cpp" "src/UI/Table.h" "src/UI/Table.cpp" "src/UI/Label.h" "src/UI/Label.cpp" "src/UI/FloatSlider.h" "src/UI/FloatSlider.cpp" "src/UI/ComboBox.h" "src/UI/ComboBox.cpp" "src/UI/ColorPicker.h" "src/UI/ColorPicker.cpp" "src/UI/FloatDrag.h" "src/UI/FloatDrag.cpp" "src/UI/HorizontalBox.h" "src/UI/HorizontalBox.cpp" "src/UI/Button.h" "src/UI/Button.cpp")

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
    RELWITHDEBINFO_POSTFIX "${STATIC_POSTFIX}-relwithdebinfo"
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
target_compile_definitions(${LibraryName} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:RLS_RELWITHDEBINFO>")
target_compile_definitions(${LibraryName} PUBLIC SHADER_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Shaders/HLSL/")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH1 "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "ENGINE_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH1}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH2 "${CMAKE_SOURCE_DIR}/Relentless-Editor/Assets/")
target_compile_definitions(${LibraryName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH2}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH3 "${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions(${LibraryName} PUBLIC "MAIN_ENGINE_DIRECTORY=\"${WINDOWS_STYLE_PATH3}\"")
