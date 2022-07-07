#Relentless/Relentless-Editor
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES "src/main.cpp")

include_directories(${PROJECT_SOURCE_DIR}/Relentless)

add_executable(Relentless-Editor WIN32 "src/RelentlessEditorApp.cpp" )
target_compile_options(Relentless-Editor PRIVATE /W4 /WX)

target_link_libraries(Relentless-Editor Relentless)

target_include_directories(Relentless-Editor PUBLIC "src")

target_precompile_headers(Relentless-Editor REUSE_FROM Relentless)

set_property(TARGET Relentless-Editor PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set_target_properties(Relentless-Editor PROPERTIES LINKER_LANGUAGE CXX)

set_property(TARGET Relentless-Editor PROPERTY FOLDER "Executables")