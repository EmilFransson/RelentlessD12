#Relentless/Relentless

set(SourcesList "src/rlspch.h" "src/Relentless.h" "src/Relentless/Application.h" "src/Relentless/Application.cpp" "src/Relentless/EntryPoint.h" "src/Relentless/Events/EventType.h" "src/Relentless/Events/EventCategory.h" "src/Relentless/Events/IEvent.h" "src/Relentless/Events/IEvent.cpp" "src/Relentless/Events/MouseEvents.h" "src/Relentless/Events/WindowEvents.h" "src/Relentless/Events/IEventListener.h" "src/Relentless/Core.h" "src/Relentless/Events/KeyboardEvents.h" "src/Relentless/Events/Layer.h" "src/Relentless/Events/Layer.cpp" "src/Relentless/Events/LayerStack.h" "src/Relentless/Events/LayerStack.cpp" "src/Relentless/Events/EventBuss.h" "src/Relentless/Events/EventBuss.cpp" "src/Relentless/Input/KeyCodes.h" "src/Relentless/Input/MouseCodes.h" "src/Relentless/Input/Keyboard.h" "src/Relentless/Input/Keyboard.cpp" "src/Relentless/Events/EventPublisher.h" "src/Relentless/Events/EventPublisher.cpp" "src/Relentless/Input/Mouse.h" "src/Relentless/Input/Mouse.cpp" "src/Relentless/Log.h" "src/Relentless/Log.cpp" "src/Relentless/Window.h" "src/Relentless/Window.cpp" "src/Relentless/Utility.h" "src/Relentless/Utility.cpp" "src/Relentless/Graphics/D3D12Core.h" "src/Relentless/Graphics/D3D12Core.cpp" "src/Relentless/Graphics/d3dx12.h" "src/Relentless/Graphics/D3D12Debug.h" "src/Relentless/Graphics/D3D12Command.h" "src/Relentless/Graphics/D3D12Command.cpp" "src/Relentless/Graphics/D3D12Debug.cpp" "src/Relentless/Graphics/DescriptorHeap.h" "src/Relentless/Graphics/DescriptorHeap.cpp" "src/Relentless/Graphics/Renderer/RenderCommand.h" "src/Relentless/Graphics/Renderer/RenderCommand.cpp" "src/Relentless/ImGui/ImguiLayer.h" "src/Relentless/ImGui/ImguiLayer.cpp")

add_library(Relentless STATIC ${SourcesList})

target_include_directories(Relentless PUBLIC "src")

target_precompile_headers("${PROJECT_NAME}" PRIVATE "src/rlspch.h")

#set_property(TARGET Relentless PROPERTY
#             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


if(NOT BUILD_SHARED_LIBS)
    set(STATIC_POSTFIX "-static")
endif()

set_target_properties("${PROJECT_NAME}"
    PROPERTIES
    DEBUG_POSTFIX "${STATIC_POSTFIX}-debug"
    RELEASE_POSTFIX "${STATIC_POSTFIX}"
)

set_target_properties(Relentless PROPERTIES LINKER_LANGUAGE CXX)

target_compile_options(Relentless PRIVATE /W4 /WX)

set_property(TARGET Relentless PROPERTY FOLDER "Libraries")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SourcesList})

target_link_libraries(Relentless PUBLIC spdlog)
target_link_libraries(Relentless PUBLIC imgui)

target_compile_definitions(Relentless PUBLIC "$<$<CONFIG:DEBUG>:RLS_DEBUG>")
target_compile_definitions(Relentless PUBLIC "$<$<CONFIG:RELEASE>:RLS_RELEASE>")

#file(GLOB_RECURSE imgui-inc "${imgui_SOURCE_DIR}/*.h")
#add_library(imgui INTERFACE ${imgui-inc})
#target_include_directories(imgui INTERFACE "${imgui_SOURCE_DIR}")

FetchContent_Declare(d3d12-agility
       URL https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.602.0
       )
FetchContent_MakeAvailable(d3d12-agility)
file(GLOB_RECURSE d3d12-agility-inc "${d3d12-agility_SOURCE_DIR}/build/native/include/*.h")
add_library(d3d12-agility INTERFACE ${d3d12-agility-inc})
target_include_directories(d3d12-agility INTERFACE "${d3d12-agility_SOURCE_DIR}/build/native/include")
target_link_libraries(d3d12-agility INTERFACE "d3d12.lib" "dxgi.lib")
target_link_libraries(${PROJECT_NAME} PRIVATE d3d12-agility) # LINK UR PROJECT HERE
#target_link_libraries(${PROJECT_NAME} PUBLIC imgui)


# Copy DLLS
add_custom_command(
    TARGET Relentless POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy    
            "${d3d12-agility_SOURCE_DIR}/build/native/bin/x64/D3D12Core.dll"
            ${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12/D3D12Core.dll
            
    COMMAND ${CMAKE_COMMAND} -E copy    
            "${d3d12-agility_SOURCE_DIR}/build/native/bin/x64/D3D12Core.pdb"
            ${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12/D3D12Core.pbd
            
    COMMAND ${CMAKE_COMMAND} -E copy    
            "${d3d12-agility_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll"
            ${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12/d3d12SDKLayers.dll
            
    COMMAND ${CMAKE_COMMAND} -E copy    
            "${d3d12-agility_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.pdb"
            ${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12/d3d12SDKLayers.pbd
    )

message(${CMAKE_BINARY_DIR}/Relentless-Editor/d3d12/d3d12SDKLayers.pbd)
message("${d3d12-agility_SOURCE_DIR}/build/native/bin/x64/d3d12SDKLayers.dll")