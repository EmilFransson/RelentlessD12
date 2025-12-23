#Root/Relentless-Editor
set(SourceFiles 
		"src/Core/RelentlessEditorApp.cpp" "src/Core/EditorLayer.h" "src/Core/EditorLayer.cpp"
 "src/Panels/OutlinerPanel.h" "src/Panels/OutlinerPanel.cpp" "src/Panels/MetricsPanel.h" "src/Panels/MetricsPanel.cpp" "src/Panels/PropertiesPanel.h" "src/Panels/PropertiesPanel.cpp" "src/Panels/ContentBrowserPanel.h" "src/Panels/ContentBrowserPanel.cpp"   "src/Panels/SceneRendererPanel.h" "src/Panels/SceneRendererPanel.cpp" "src/Panels/InspectorPanel.h" "src/Panels/InspectorPanel.cpp"  "src/UI/OutlinerTreeItem.h"    "src/UI/OutlinerTreeItem.cpp"       "src/Core/Selection.h" "src/Core/Selection.cpp" "src/Core/Editor.h" "src/Core/Editor.cpp" "src/Core/EntityFolders.h" "src/Core/EntityFolders.cpp" "src/Core/RelentlessEditorApp.h" "../Relentless/src/UI/Panel.cpp" "src/Panels/ViewportPanel.h" "src/Panels/ViewportPanel.cpp" "src/Controller/TransformGizmoController.h" "src/Controller/TransformGizmoController.cpp" "src/Controller/PerspectiveCameraController.h" "src/Controller/PerspectiveCameraController.cpp" "src/Panels/DetailsPanel.h" "src/Panels/DetailsPanel.cpp"       "src/UI/Views/Outliner/EntityOutlinerView.h" "src/UI/Views/Outliner/EntityOutlinerView.cpp" "src/UI/OutlinerTableRow.h" "src/UI/OutlinerTableRow.cpp" "src/UI/DragDrop/OutlinerDragDropOperation.h" "src/UI/DragDrop/OutlinerDragDropOperation.cpp" "src/UI/Views/Outliner/EntityOutlinerPolicies.h" "src/UI/Views/Outliner/EntityOutlinerPolicies.cpp" "src/Panels/TestPanel.h" "src/Panels/TestPanel.cpp" "src/Panels/IEditorPanel.h" "src/Panels/IEditorPanel.cpp" "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.cpp" "src/UI/Views/Details/Customizations/IDetailCustomization.h" "src/UI/Views/Details/Customizations/IDetailCustomization.cpp" "src/UI/Views/Details/IDetailsView.h" "src/UI/Views/Details/IDetailsView.cpp" "src/UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h" "src/UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.cpp" "src/UI/Views/Details/Customizations/TransformComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/TransformComponentDetailCustomization.cpp" "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.cpp"   "src/UI/Views/Details/DetailNode.h" "src/UI/Views/Details/DetailNode.cpp" "src/UI/Views/Details/TableRows/EntityDetailRow.h" "src/UI/Views/Details/TableRows/EntityDetailRow.cpp" "src/UI/Views/Details/EntityDetailsView.h" "src/UI/Views/Details/EntityDetailsView.cpp" "src/UI/Views/Details/TableRows/EntityDetailCategoryRow.cpp" "src/UI/Views/Details/TableRows/EntityDetailCategoryRow.h" "src/UI/Views/Details/Customizations/DirectionalLightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/DirectionalLightComponentDetailCustomization.h" "src/UnitTests/FolderUnitTests.h" "src/UnitTests/FolderUnitTests.cpp" "src/Panels/ContentBrowserPanelEx.h" "src/Panels/ContentBrowserPanelEx.cpp" "src/UI/Views/Assets/AssetView.h" "src/UI/Views/Assets/AssetView.cpp" "src/UI/Views/Details/Customizations/EntityDetailCustomization.h" "src/UI/Views/Details/Customizations/EntityDetailCustomization.cpp" "src/UI/Views/Details/Customizations/PointLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/PointLightComponentDetailCustomization.cpp"   "src/UI/Views/Details/Customizations/ILightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/ILightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/SpotLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/SpotLightComponentDetailCustomization.cpp")
		
set(ExecutableName "Relentless-Editor")
set (ExternalIncludePath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes/")
set (ExternalLibPath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/libs")

add_executable("${ExecutableName}" "${SourceFiles}")

target_include_directories("${ExecutableName}" PRIVATE "${CMAKE_SOURCE_DIR}/Relentless/src/")
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

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Resources/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_RESOURCE_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

target_compile_definitions(${ExecutableName} PUBLIC RLS_EDITOR)
