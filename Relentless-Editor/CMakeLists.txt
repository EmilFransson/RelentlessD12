#Root/Relentless-Editor
set(SourceFiles 
		"src/Core/RelentlessEditorApp.cpp" "src/Core/EditorLayer.h" "src/Core/EditorLayer.cpp"
 "src/Panels/OutlinerPanel.h" "src/Panels/OutlinerPanel.cpp" "src/Panels/MetricsPanel.h" "src/Panels/MetricsPanel.cpp"                         "src/Core/Editor.h" "src/Core/Editor.cpp"   "src/Core/RelentlessEditorApp.h" "src/UI/Widgets/Panel.cpp" "src/Panels/ViewportPanel.h" "src/Panels/ViewportPanel.cpp" "src/Controller/TransformGizmoController.h" "src/Controller/TransformGizmoController.cpp" "src/Controller/PerspectiveCameraController.h" "src/Controller/PerspectiveCameraController.cpp" "src/Panels/EntityDetailsPanel.h" "src/Panels/EntityDetailsPanel.cpp"       "src/UI/Views/Outliner/EntityOutlinerView.h" "src/UI/Views/Outliner/EntityOutlinerView.cpp" "src/UI/OutlinerTableRow.h" "src/UI/OutlinerTableRow.cpp" "src/UI/DragDrop/OutlinerDragDropOperation.h" "src/UI/DragDrop/OutlinerDragDropOperation.cpp" "src/UI/Views/Outliner/EntityOutlinerPolicies.h" "src/UI/Views/Outliner/EntityOutlinerPolicies.cpp" "src/Panels/WidgetShowcasePanel.h" "src/Panels/WidgetShowcasePanel.cpp"   "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.cpp" "src/UI/Views/Details/Customizations/IDetailCustomization.h"  "src/UI/Views/Details/IDetailsView.h" "src/UI/Views/Details/IDetailsView.cpp" "src/UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h" "src/UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.cpp" "src/UI/Views/Details/Customizations/TransformComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/TransformComponentDetailCustomization.cpp" "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.cpp"   "src/UI/Views/Details/DetailNode.h" "src/UI/Views/Details/DetailNode.cpp" "src/UI/Views/Details/TableRows/EntityDetailRow.h" "src/UI/Views/Details/TableRows/EntityDetailRow.cpp" "src/UI/Views/Details/EntityDetailsView.h" "src/UI/Views/Details/EntityDetailsView.cpp" "src/UI/Views/Details/TableRows/EntityDetailCategoryRow.cpp" "src/UI/Views/Details/TableRows/EntityDetailCategoryRow.h" "src/UI/Views/Details/Customizations/DirectionalLightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/DirectionalLightComponentDetailCustomization.h" "src/UnitTests/FolderUnitTests.h" "src/UnitTests/FolderUnitTests.cpp" "src/Panels/ContentBrowserPanel.h" "src/Panels/ContentBrowserPanel.cpp" "src/UI/Views/Assets/AssetView.h" "src/UI/Views/Assets/AssetView.cpp" "src/UI/Views/Details/Customizations/EntityDetailCustomization.h" "src/UI/Views/Details/Customizations/EntityDetailCustomization.cpp" "src/UI/Views/Details/Customizations/PointLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/PointLightComponentDetailCustomization.cpp"   "src/UI/Views/Details/Customizations/ILightComponentDetailCustomization.h"  "src/UI/Views/Details/Customizations/SpotLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/SpotLightComponentDetailCustomization.cpp" "src/Assets/Definition/IAssetDefinition.h" "src/Assets/Definition/IAssetDefinition.cpp" "src/Assets/Definition/MaterialAssetDefinition.h" "src/Assets/Definition/MaterialAssetDefinition.cpp" "src/Assets/Definition/Texture2DAssetDefinition.h" "src/Assets/Definition/Texture2DAssetDefinition.cpp" "src/Subsystem/AssetImportSubsystem.h" "src/Subsystem/AssetImportSubsystem.cpp"   "src/Assets/Factory/MaterialFactory.h" "src/Assets/Factory/MaterialFactory.cpp" "src/Assets/Factory/ModelFactory.h" "src/Assets/Factory/ModelFactory.cpp"   "src/Assets/Factory/TextureFactory.h" "src/Assets/Factory/TextureFactory.cpp"    "src/Subsystem/AssetDefinitionRegistry.h" "src/Subsystem/AssetDefinitionRegistry.cpp" "src/Assets/Definition/MeshAssetDefinition.h" "src/Assets/Definition/MeshAssetDefinition.cpp"  "src/Thumbnail/AssetThumbnailPool.h" "src/Thumbnail/AssetThumbnailPool.cpp" "src/Module/ContentBrowserModule.h" "src/Module/ContentBrowserModule.cpp" "src/Assets/Factory/FactoryBase.h" "src/Assets/Factory/FactoryBase.cpp" "src/Assets/Factory/MeshFactory.h" "src/Assets/Factory/MeshFactory.cpp" "src/UI/Views/Details/Customizations/DetailCustomizationMeta.h" "src/Subsystem/IEditorSubsystem.h"  "src/Subsystem/SelectionSubsystem.h" "src/Subsystem/SelectionSubsystem.cpp" "src/Subsystem/EntityFoldersSubsystem.h" "src/Subsystem/EntityFoldersSubsystem.cpp" "src/Subsystem/EditorSceneBridgeSubsystem.h" "src/Subsystem/EditorSceneBridgeSubsystem.cpp" "src/Module/UIModule.h" "src/Module/UIModule.cpp"
 "src/UI/Widgets/EditableTextBox.h" "src/UI/Widgets/EditableTextBox.cpp" "src/UI/Widgets/WidgetSwitcher.h" "src/UI/Widgets/WidgetSwitcher.cpp" "src/UI/Widgets/Border.h" "src/UI/Widgets/Border.cpp" "src/UI/Widgets/IWidgetContainer.h"  "src/UI/Widgets/HorizontalBox.h" "src/UI/Widgets/HorizontalBox.cpp" "src/UI/Widgets/VerticalBox.h" "src/UI/Widgets/VerticalBox.cpp"   "src/UI/Widgets/IntDrag.h" "src/UI/Widgets/IntDrag.cpp"   "src/UI/Widgets/Thumbnail.h" "src/UI/Widgets/Thumbnail.cpp" "src/UI/Widgets/FloatEntryBox.h" "src/UI/Widgets/FloatEntryBox.cpp" "src/UI/Widgets/Panel.h" "src/UI/Widgets/Panel.cpp" "src/UI/Widgets/DragDropOperation.h" "src/UI/Widgets/DragDropOperation.cpp" "src/UI/Widgets/ContextMenu.h" "src/UI/Widgets/ContextMenu.cpp" "src/UI/Widgets/Separator.h" "src/UI/Widgets/Separator.cpp" "src/UI/Widgets/ITableRow.h" "src/UI/Widgets/ITableRow.cpp" "src/UI/Widgets/Tooltip.h" "src/UI/Widgets/Tooltip.cpp" "src/UI/Widgets/Float3Drag.h" "src/UI/Widgets/Float3Drag.cpp" "src/UI/Widgets/IWidget.h" "src/UI/Widgets/IWidget.cpp" "src/UI/Widgets/CheckBox.h" "src/UI/Widgets/CheckBox.cpp"   "src/UI/Widgets/CollapsibleSection.h" "src/UI/Widgets/CollapsibleSection.cpp" "src/UI/Widgets/Table.h" "src/UI/Widgets/Table.cpp" "src/UI/Widgets/Label.h" "src/UI/Widgets/Label.cpp" "src/UI/Widgets/FloatSlider.h" "src/UI/Widgets/FloatSlider.cpp" "src/UI/Widgets/ComboBox.h" "src/UI/Widgets/ComboBox.cpp" "src/UI/Widgets/ColorPicker.h" "src/UI/Widgets/ColorPicker.cpp" "src/UI/Widgets/FloatDrag.h" "src/UI/Widgets/FloatDrag.cpp"   "src/UI/Widgets/Button.h" "src/UI/Widgets/Button.cpp" "src/UI/Widgets/SearchBar.h" "src/UI/Widgets/SearchBar.cpp" "src/UI/Widgets/Canvas.h" "src/UI/Widgets/Canvas.cpp" "src/UI/Widgets/IntSlider.h" "src/UI/Widgets/IntSlider.cpp" "src/UI/Widgets/UI.h" "src/UI/Widgets/UI.cpp" "src/UI/Widgets/DragDropBehavior.h" "src/UI/Widgets/DragDropBehavior.cpp" 
          "src/UI/Details/DetailPropertyRow.h" "src/UI/Details/DetailPropertyRow.cpp" "src/UI/Nodes/DetailRowNode.h" "src/UI/Nodes/DetailRowNode.cpp" "src/UI/Nodes/DetailCategoryNode.h" "src/UI/Nodes/DetailCategoryNode.cpp" "src/UI/Nodes/ITreeNode.h" "src/UI/Nodes/ITreeNode.cpp" "src/UI/Nodes/IDetailsTreeNode.h"  "src/UI/Details/DetailCategoryRow.h" "src/UI/Details/DetailCategoryRow.cpp" "src/UI/Views/ListView.h" "src/UI/Views/ListView.cpp" "src/UI/Views/TreeView.h" "src/UI/Views/TileView.h" "src/UI/Meta/ThumbnailInfo.h"  "src/Core/EntryPoint.cpp" "src/ImGui/ImGuiLayer.h" "src/ImGui/ImGuiLayer.cpp" "src/ImGui/ImGuiIncludes.h")
		
set(ExecutableName "Relentless-Editor")
set (ExternalIncludePath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes/")
set (ExternalLibPath "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/libs")

add_executable("${ExecutableName}" "${SourceFiles}")

target_include_directories("${ExecutableName}" PRIVATE "${CMAKE_SOURCE_DIR}/Relentless-Editor/src/")
target_include_directories("${ExecutableName}" PRIVATE "${CMAKE_SOURCE_DIR}/Relentless/")
target_include_directories("${ExecutableName}" SYSTEM PRIVATE "${ExternalIncludePath}")

target_link_libraries("${ExecutableName}" PRIVATE "Relentless")
target_compile_options("${ExecutableName}" PRIVATE "/W4")

add_library(ImGuizmo STATIC
    "vendor/includes/ImGuizmo/ImGuizmo.cpp"
    "vendor/includes/ImGuizmo/ImCurveEdit.cpp"
    "vendor/includes/ImGuizmo/ImGradient.cpp"
    "vendor/includes/ImGuizmo/ImSequencer.cpp"
)

target_include_directories(ImGuizmo SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes")

# Disable warnings for this third-party target
if (MSVC)
    target_compile_options(ImGuizmo PRIVATE /w)
else()
    target_compile_options(ImGuizmo PRIVATE -w)
endif()

target_link_libraries(${ExecutableName} PRIVATE ImGuizmo)
target_link_libraries("${ExecutableName}" PRIVATE "dxgi")

target_compile_definitions("${ExecutableName}" PRIVATE RUNTIME_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_definitions("${ExecutableName}" PRIVATE PROJECT_WORKSPACE="${CMAKE_SOURCE_DIR}/")
target_compile_definitions("${ExecutableName}" PRIVATE PROJECT_BIN="${CMAKE_BINARY_DIR}/${ExecutableName}")
target_compile_definitions(${ExecutableName} PRIVATE STATIC_TYPE_INFO_USE_MEMBER_POINTER=0)

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
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	"${var}"
	${CMAKE_BINARY_DIR}/Relentless-Editor/

	)
endforeach()

#Engine DLL:
add_custom_command(TARGET ${ExecutableName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Relentless>
        $<TARGET_FILE_DIR:${ExecutableName}>
)

# Engine PDB:
add_custom_command(TARGET ${ExecutableName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_PDB_FILE:Relentless>
        $<TARGET_FILE_DIR:${ExecutableName}>
)

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Resources/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_RESOURCE_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

target_compile_definitions(${ExecutableName} PUBLIC RLS_EDITOR)
