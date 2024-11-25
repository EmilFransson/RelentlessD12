#pragma once
#include "ImGui/ImguiLayer.h"
#include "UI/UI.h"
#include "Assets/AssetMeta.h"

namespace Relentless
{
	namespace TreeDefaultColors
	{
		constexpr ImVec4 EvenRowColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		constexpr ImVec4 OddRowColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);
		constexpr ImVec4 RowHoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
		constexpr ImVec4 RowSelectedColor = ImVec4(64.0f / 255.0f, 87.0f / 255.0f, 111.0f / 255.0f, 1.0f);
		constexpr ImVec4 RowSelectedFocusedColor = ImVec4(30.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 200.0f / 255.0f);
		constexpr ImVec4 RowAncestorToSelectedColor = ImVec4(44.0f / 255.0f, 50.0f / 255.0f, 58.0f / 255.0f, 1.0f);
	}

	struct TableIcon
	{
		AssetHandle IconTextureHandle = NULL_HANDLE;
		ImVec2 SizeWeight{ 0.5f, 0.5f };
		ImVec4 Tint{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	struct TreeItemStyle
	{
		UI::Alignment Alignment = UI::Alignment::Left;
		ImVec4 LabelColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 IconTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		float IconWeight = 0.6f;
		float Spacing = -1.0f;
	};

	struct TreeItemData
	{
		std::vector<std::string> ColumnLabels;
		std::vector<TableIcon> ColumnIcons;
		std::vector<TreeItemStyle> ColumnStyles;
		ImVec4 BackgroundColor;
		bool UseDefaultItemColor = true;
	};
}