#Relentless/Relentless

set(SourcesList "src/rlspch.h" "src/Relentless.h" "src/Relentless/Application.h" "src/Relentless/Application.cpp" "src/Relentless/EntryPoint.h" "src/Relentless/Events/EventType.h" "src/Relentless/Events/EventCategory.h" "src/Relentless/Events/IEvent.h" "src/Relentless/Events/IEvent.cpp" "src/Relentless/Events/MouseEvents.h" "src/Relentless/Events/WindowEvents.h" "src/Relentless/Events/IEventListener.h")

add_library(Relentless STATIC ${SourcesList})

target_include_directories(Relentless PUBLIC "src")

target_precompile_headers("${PROJECT_NAME}" PRIVATE "src/rlspch.h")

set_property(TARGET Relentless PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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

target_compile_definitions(Relentless PUBLIC "$<$<CONFIG:DEBUG>:RLS_DEBUG>")
target_compile_definitions(Relentless PUBLIC "$<$<CONFIG:RELEASE>:RLS_RELEASE>")