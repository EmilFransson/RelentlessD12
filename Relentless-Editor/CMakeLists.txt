#Root/Relentless-Editor
set(SourceFiles 
		"src/Core/RelentlessEditorApp.cpp" "src/Core/EditorLayer.h" "src/Core/EditorLayer.cpp"
 "src/Panels/SceneHierarchyPanel.h" "src/Panels/SceneHierarchyPanel.cpp" "src/Panels/MetricsPanel.h" "src/Panels/MetricsPanel.cpp" "src/Panels/PropertiesPanel.h" "src/Panels/PropertiesPanel.cpp" "src/Panels/ContentBrowserPanel.h" "src/Panels/ContentBrowserPanel.cpp"   "src/Panels/SceneRendererPanel.h" "src/Panels/SceneRendererPanel.cpp")
		
set(ExecutableName "Relentless-Editor")
set (ExternalIncludePath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes/")
set (ExternalLibPath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/libs")

add_executable("${ExecutableName}" "${SourceFiles}")

target_include_directories("${ExecutableName}" PRIVATE "src/")
target_include_directories("${ExecutableName}" PRIVATE "${CMAKE_SOURCE_DIR}/Relentless/" "${ExternalIncludePath}")

target_link_libraries("${ExecutableName}" PRIVATE "Relentless")
target_compile_options("${ExecutableName}" PRIVATE "/W4")

target_compile_definitions("${ExecutableName}" PRIVATE RUNTIME_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions("${ExecutableName}" PRIVATE PROJECT_WORKSPACE="${CMAKE_SOURCE_DIR}/")
target_compile_definitions("${ExecutableName}" PRIVATE PROJECT_BIN="${CMAKE_BINARY_DIR}/${ExecutableName}")

##### Section for linking external libraries #####
file(GLOB_RECURSE libFiles
	  ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.lib)

foreach(var ${libFiles})
	target_link_libraries("${ExecutableName}" PRIVATE "${var}")
endforeach()

file(GLOB_RECURSE dlls
	  ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.dll ${ExternalLibPath}/${CMAKE_BUILD_TYPE}/*.pdb)

foreach(var ${dlls})
	add_custom_command(
	TARGET "${ExecutableName}" POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy 
	"${var}"
	${CMAKE_BINARY_DIR}/Relentless-Editor/

	)
endforeach()