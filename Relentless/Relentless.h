#pragma once

//Core:
#include <src/Core/Application.h>
#include <src/Core/Core.h>
#include <src/Core/CoreTypes.h>
#include <src/Core/Time.h>
#include <src/Core/Any.h>
#include <src/Core/IAsset.h>

//Logging:
#include <src/Core/Log.h>

//Callback:
#include <src/Callback/Broadcaster.h>
#include <src/Callback/Callback.h>

//Utility:
#include <src/Utility/FileDialogs.h>
#include <src/Utility/Common.h>
#include <src/Utility/StringUtils.h>
#include <src/Utility/TextFilterExpressionEvaluator.h>

//File
#include "src/File/FilePath.h"

//Events:
#include <src/ImGui/ImguiLayer.h>
#include <src/EventSystem/Layer.h>
#include <src/EventSystem/IEvent.h>
#include <src/EventSystem/KeyboardEvents.h>
#include <src/EventSystem/MouseEvents.h>

//UI:
#include <src/UI/Button.h>
#include <src/UI/Canvas.h>
#include <src/UI/ColorPicker.h>
#include <src/UI/CheckBox.h>
#include <src/UI/CollapsibleSection.h>
#include <src/UI/ComboBox.h>
#include <src/UI/DragDropBehavior.h>
#include <src/UI/FloatDrag.h>
#include <src/UI/Float3Drag.h>
#include <src/UI/FloatSlider.h>
#include <src/UI/HorizontalBox.h>
#include <src/UI/IntSlider.h>
#include <src/UI/ITableRow.h>
#include <src/UI/IWidget.h>
#include <src/UI/Label.h>
#include <src/UI/SearchBar.h>
#include <src/UI/Table.h>
#include <src/UI/Tooltip.h>
#include <src/UI/Tree/Tree.h>
#include <src/UI/Tree/TreeItem.h>
#include <src/UI/Tree/TreeStyle.h>
#include <src/UI/Tree/TreeInteraction.h>
#include <src/UI/Tree/TreeTypes.h>
#include <src/UI/UI.h>
#include <src/UI/VerticalBox.h>

#include <src/UI/Nodes/ITreeNode.h>
#include <src/UI/Nodes/DetailCategoryNode.h>
#include <src/UI/Nodes/DetailRowNode.h>
#include <src/UI/Nodes/IDetailsTreeNode.h>

#include <src/UI/Details/IDetailsView.h>
#include <src/UI/Details/DetailPropertyRow.h>
#include <src/UI/Details/DetailCategoryRow.h>
#include <src/UI/List/ListView.h>

//Graphics:
#include <src/Graphics/Renderer/SceneRenderer.h>
#include <src/Graphics/Renderer/UtilityRenderer.h>
#include <src/Graphics/RHI/Device.h>
#include <src/Graphics/RHI/ResourceViews.h>
#include <src/Graphics/RHI/CommandContext.h>
#include <src/Graphics/Renderer/RenderTypes.h>

//Resources
#include <src/Assets/AssetManager.h>
#include <src/Mesh/Mesh.h>

//IO
#include <src/Input/Mouse.h>
#include <src/Input/Keyboard.h>

//Mesh-related:
#include <src/Mesh/Vertex.h>

//Camera
#include <src/Graphics/Renderer/Camera/PerspectiveCamera.h>

//ECS
#include <src/Scene/Scene.h>
#include <src/ECS/EntityManager.h>
#include <src/ECS/ECSCommon.h>

//Serialization
#include <src/Scene/SceneSerializer.h>

//Assets
#include <src/Assets/Factory/ModelFactory.h>
#include <src/Assets/Factory/TextureFactory.h>