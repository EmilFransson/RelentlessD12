#Root/Relentless-Editor
set(SourceFiles 
		"src/Core/RelentlessEditorApp.cpp" "src/Core/EditorLayer.h" "src/Core/EditorLayer.cpp"
 "src/Panels/OutlinerPanel.h" "src/Panels/OutlinerPanel.cpp" "src/Panels/MetricsPanel.h" "src/Panels/MetricsPanel.cpp"                         "src/Core/Editor.h" "src/Core/Editor.cpp"   "src/Core/RelentlessEditorApp.h" "src/UI/Widgets/Panel.cpp" "src/Panels/ViewportPanel.h" "src/Panels/ViewportPanel.cpp" "src/Controller/TransformGizmoController.h" "src/Controller/TransformGizmoController.cpp" "src/Controller/PerspectiveCameraController.h" "src/Controller/PerspectiveCameraController.cpp" "src/Panels/EntityDetailsPanel.h" "src/Panels/EntityDetailsPanel.cpp"       "src/UI/Views/Outliner/EntityOutlinerView.h" "src/UI/Views/Outliner/EntityOutlinerView.cpp" "src/UI/Views/Outliner/TableRows/OutlinerTableRow.h" "src/UI/Views/Outliner/TableRows/OutlinerTableRow.cpp" "src/UI/DragDrop/OutlinerDragDropOperation.h" "src/UI/DragDrop/OutlinerDragDropOperation.cpp" "src/UI/Views/Outliner/EntityOutlinerPolicies.h" "src/UI/Views/Outliner/EntityOutlinerPolicies.cpp" "src/Panels/WidgetShowcasePanel.h" "src/Panels/WidgetShowcasePanel.cpp"   "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.cpp" "src/UI/Views/Details/Customizations/IDetailCustomization.h"  "src/UI/Views/Details/IDetailsView.h" "src/UI/Views/Details/IDetailsView.cpp"     "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.cpp"   "src/UI/Nodes/DetailNode.h" "src/UI/Nodes/DetailNode.cpp"   "src/UI/Views/Details/EntityDetailsView.h" "src/UI/Views/Details/EntityDetailsView.cpp"     "src/UnitTests/FolderUnitTests.h" "src/UnitTests/FolderUnitTests.cpp" "src/Panels/ContentBrowserPanel.h" "src/Panels/ContentBrowserPanel.cpp" "src/UI/Views/Assets/AssetView.h" "src/UI/Views/Assets/AssetView.cpp"           "src/Assets/Definition/IAssetDefinition.h" "src/Assets/Definition/IAssetDefinition.cpp" "src/Assets/Definition/MaterialAssetDefinition.h" "src/Assets/Definition/MaterialAssetDefinition.cpp" "src/Assets/Definition/Texture2DAssetDefinition.h" "src/Assets/Definition/Texture2DAssetDefinition.cpp" "src/Subsystem/AssetImportSubsystem.h" "src/Subsystem/AssetImportSubsystem.cpp"   "src/Assets/Factory/MaterialFactory.h" "src/Assets/Factory/MaterialFactory.cpp" "src/Assets/Factory/ModelFactory.h" "src/Assets/Factory/ModelFactory.cpp"   "src/Assets/Factory/TextureFactory.h" "src/Assets/Factory/TextureFactory.cpp"    "src/Subsystem/AssetDefinitionRegistry.h" "src/Subsystem/AssetDefinitionRegistry.cpp" "src/Assets/Definition/MeshAssetDefinition.h" "src/Assets/Definition/MeshAssetDefinition.cpp"  "src/Thumbnail/AssetThumbnailPool.h" "src/Thumbnail/AssetThumbnailPool.cpp" "src/Module/ContentBrowserModule.h" "src/Module/ContentBrowserModule.cpp" "src/Assets/Factory/FactoryBase.h" "src/Assets/Factory/FactoryBase.cpp" "src/Assets/Factory/MeshFactory.h" "src/Assets/Factory/MeshFactory.cpp"  "src/Subsystem/IEditorSubsystem.h"  "src/Subsystem/SelectionSubsystem.h" "src/Subsystem/SelectionSubsystem.cpp" "src/Subsystem/EntityFoldersSubsystem.h" "src/Subsystem/EntityFoldersSubsystem.cpp" "src/Subsystem/EditorSceneBridgeSubsystem.h" "src/Subsystem/EditorSceneBridgeSubsystem.cpp" "src/Module/UIModule.h" "src/Module/UIModule.cpp"
 "src/UI/Widgets/EditableTextBox.h" "src/UI/Widgets/EditableTextBox.cpp" "src/UI/Widgets/WidgetSwitcher.h" "src/UI/Widgets/WidgetSwitcher.cpp" "src/UI/Widgets/Border.h" "src/UI/Widgets/Border.cpp" "src/UI/Widgets/IWidgetContainer.h"        "src/UI/Widgets/IntDrag.h" "src/UI/Widgets/IntDrag.cpp"   "src/UI/Widgets/AssetThumbnail.h" "src/UI/Widgets/AssetThumbnail.cpp" "src/UI/Widgets/FloatEntryBox.h" "src/UI/Widgets/FloatEntryBox.cpp" "src/UI/Widgets/Panel.h" "src/UI/Widgets/Panel.cpp" "src/UI/DragDrop/DragDropOperation.h" "src/UI/DragDrop/DragDropOperation.cpp" "src/UI/Widgets/ContextMenu.h" "src/UI/Widgets/ContextMenu.cpp" "src/UI/Widgets/Separator.h" "src/UI/Widgets/Separator.cpp" "src/UI/Widgets/ITableRow.h" "src/UI/Widgets/ITableRow.cpp" "src/UI/Widgets/Tooltip.h" "src/UI/Widgets/Tooltip.cpp" "src/UI/Widgets/Float3Drag.h" "src/UI/Widgets/Float3Drag.cpp" "src/UI/Widgets/IWidget.h"  "src/UI/Widgets/CheckBox.h" "src/UI/Widgets/CheckBox.cpp"   "src/UI/Widgets/CollapsibleSection.h" "src/UI/Widgets/CollapsibleSection.cpp" "src/UI/Widgets/Table.h" "src/UI/Widgets/Table.cpp" "src/UI/Widgets/Label.h" "src/UI/Widgets/Label.cpp" "src/UI/Widgets/FloatSlider.h" "src/UI/Widgets/FloatSlider.cpp" "src/UI/Widgets/ComboBox.h" "src/UI/Widgets/ComboBox.cpp" "src/UI/Widgets/ColorPicker.h" "src/UI/Widgets/ColorPicker.cpp" "src/UI/Widgets/FloatDrag.h" "src/UI/Widgets/FloatDrag.cpp"   "src/UI/Widgets/Button.h" "src/UI/Widgets/Button.cpp" "src/UI/Widgets/SearchBar.h" "src/UI/Widgets/SearchBar.cpp" "src/UI/Widgets/Canvas.h" "src/UI/Widgets/Canvas.cpp" "src/UI/Widgets/IntSlider.h" "src/UI/Widgets/IntSlider.cpp"     
                      "src/UI/Views/ListView.h" "src/UI/Views/ListView.cpp" "src/UI/Views/TreeView.h" "src/UI/Views/TileView.h" "src/UI/Meta/ThumbnailInfo.h"  "src/Core/EntryPoint.cpp" "src/ImGui/ImGuiLayer.h" "src/ImGui/ImGuiLayer.cpp" "src/ImGui/ImGuiIncludes.h" "src/Subsystem/EditorViewportSubsystem.h" "src/Subsystem/EditorViewportSubsystem.cpp" "src/Subsystem/EditorRendererBridgeSubsystem.h" "src/Subsystem/EditorRendererBridgeSubsystem.cpp" "src/Event/EditorEvents.h"   "src/UI/Views/Details/ViewportDetailsView.h" "src/UI/Views/Details/ViewportDetailsView.cpp"     "src/UI/Views/Details/Customizations/Viewport/ViewportCameraDetailCustomization.h" "src/UI/Views/Details/Customizations/Viewport/ViewportCameraDetailCustomization.cpp" "src/UI/Widgets/HorizontalBox.h" "src/UI/Widgets/HorizontalBox.cpp" "src/Panels/TestLayoutPanel.h" "src/Panels/TestLayoutPanel.cpp" "src/UI/Widgets/VerticalBox.h" "src/UI/Widgets/VerticalBox.cpp" "src/UI/Widgets/Spacer.h" "src/UI/Widgets/Spacer.cpp" "src/UI/Widgets/NumericEntryBox.h"  "src/UI/Meta/ImGuiHelpers.h" "src/UI/Widgets/Slider.h" "src/UI/Widgets/SpinBox.h" "src/Property/IPropertyHandleBase.h" "src/Property/PropertyHandle.h" "src/UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"  "src/UI/Views/Details/TableRows/DetailPropertyRow.h" "src/UI/Views/Details/TableRows/DetailPropertyRow.cpp" "src/Details/DetailCustomizationRegistry.h" "src/Details/DetailCustomizationRegistry.cpp" "src/UI/Views/Details/TableRows/DetailCategoryRow.h" "src/UI/Views/Details/TableRows/DetailCategoryRow.cpp" "src/Module/DetailsModule.h" "src/Module/DetailsModule.cpp" "src/UI/Views/Details/Context/ViewportDetailsContext.h" "src/Property/MultiPropertyHandle.h"   "src/Property/EntityPropertyHandle.h" "src/UI/Views/Details/LayoutBuilders/DetailLayoutBuilderHelpers.h"   "src/UI/Views/Details/Customizations/Entity/TransformComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/TransformComponentDetailCustomization.cpp" "src/UI/Views/Details/Context/EntityDetailsContext.h" "src/UI/Views/Details/Customizations/Entity/DirectionalLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/DirectionalLightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/Entity/PointLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/PointLightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/Entity/SpotLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/SpotLightComponentDetailCustomization.cpp" "src/Assets/Factory/EnvironmentFactory.h" "src/Assets/Factory/EnvironmentFactory.cpp" "src/Assets/Definition/EnvironmentAssetDefinition.h" "src/Assets/Definition/EnvironmentAssetDefinition.cpp" "src/UI/Views/Details/Customizations/Entity/SkyLightComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/SkyLightComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/Entity/SkyBoxComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/SkyBoxComponentDetailCustomization.cpp" "src/UI/Views/Details/Customizations/Entity/PostProcessVolumeComponentDetailCustomization.h" "src/UI/Views/Details/Customizations/Entity/PostProcessVolumeComponentDetailCustomization.cpp" "src/ImGui/ImGuiFonts.h" "src/ImGui/ImGuiFonts.cpp" "src/Thumbnail/AssetThumbnailData.h" "src/Thumbnail/AssetThumbnailData.cpp" "src/UI/Widgets/AssetTileItem.h" "src/UI/Widgets/AssetTileItem.cpp" "src/UI/DragDrop/AssetDragDropOperation.h" "src/UI/DragDrop/AssetDragDropOperation.cpp" "src/UI/Widgets/IBaseWidget.h" "src/UI/Widgets/IBaseWidget.cpp" "src/UI/Widgets/IStylableWidget.h" "src/UI/Widgets/IStylableWidget.cpp" "src/UI/Widgets/AssetDropTarget.h" "src/UI/Widgets/AssetDropTarget.cpp" "src/UI/Nodes/AssetDetailNode.h" "src/UI/Nodes/AssetDetailNode.cpp" "src/Property/AssetPropertyHandle.h" "src/UI/Views/Details/LayoutBuilders/AssetDetailPropertyRowBuilder.h" "src/UI/Views/Details/LayoutBuilders/AssetDetailPropertyRowBuilder.cpp" "src/UI/Views/Details/LayoutBuilders/SlotBuilder.h" "src/UI/Brush/Brush.h" "src/UI/Views/Details/TableRows/DetailGroupRow.h" "src/UI/Views/Details/TableRows/DetailGroupRow.cpp" "src/UI/Views/Details/LayoutBuilders/IDetailGroupBuilder.h" "src/UI/Views/Details/LayoutBuilders/IDetailGroupBuilder.cpp" "src/Panels/EnvironmentEditorPanel.h" "src/Panels/EnvironmentEditorPanel.cpp" "src/UI/Views/Details/EnvironmentDetailsView.h" "src/UI/Views/Details/EnvironmentDetailsView.cpp" "src/UI/Views/Details/Context/EnvironmentDetailsContext.h" "src/UI/Views/Details/Customizations/Assets/EnvironmentDetailCustomization.h" "src/UI/Views/Details/Customizations/Assets/EnvironmentDetailCustomization.cpp")
		
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
    "vendor/includes/ImGuizmo/GraphEditor.cpp"
)

add_library(ImGui STATIC
    "vendor/includes/ImGUI/imgui.cpp"
    "vendor/includes/ImGUI/imgui_demo.cpp"
    "vendor/includes/ImGUI/imgui_draw.cpp"
    "vendor/includes/ImGUI/imgui_tables.cpp"
    "vendor/includes/ImGUI/imgui_widgets.cpp"
    "vendor/includes/ImGUI/backends/imgui_impl_win32.cpp"
    "vendor/includes/ImGUI/backends/imgui_impl_dx12.cpp"
)

target_include_directories(ImGuizmo SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes")
target_include_directories(ImGui SYSTEM PUBLIC 
    "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes"
    "${CMAKE_SOURCE_DIR}/Relentless-Editor/vendor/includes/ImGUI"
)

# Disable warnings for this third-party target
if (MSVC)
    target_compile_options(ImGuizmo PRIVATE /w)
else()
    target_compile_options(ImGuizmo PRIVATE -w)
endif()

target_link_libraries(ImGuizmo PUBLIC ImGui)
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

if (CMAKE_CONFIGURATION_TYPES)
    # Multi-config (Visual Studio)
    add_custom_command(TARGET ${ExecutableName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$<TARGET_PDB_FILE:Relentless>>
            $<TARGET_FILE_DIR:${ExecutableName}>
    )
else()
    # Single-config (Ninja)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_custom_command(TARGET ${ExecutableName} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_PDB_FILE:Relentless>
                $<TARGET_FILE_DIR:${ExecutableName}>
        )
    endif()
endif()

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Assets/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_ASSET_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

string(REPLACE "/" "\\\\" WINDOWS_STYLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Resources/")
target_compile_definitions(${ExecutableName} PUBLIC "EDITOR_RESOURCE_DIRECTORY=\"${WINDOWS_STYLE_PATH}\"")

target_compile_definitions(${ExecutableName} PUBLIC RLS_EDITOR)
