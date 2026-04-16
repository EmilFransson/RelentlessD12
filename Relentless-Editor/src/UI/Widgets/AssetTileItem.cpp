#include "AssetTileItem.h"

#include "Core/Editor.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "Thumbnail/AssetThumbnailData.h"

#include "UI/Views/TileView.h"

namespace Relentless
{
	constexpr Color DEFAULT_BACKGROUND_COLOR = Color(56.0f, 56.0f, 56.0f, 255.0f);
	constexpr float TILE_BORDER_SIZE = 2.0f;
	constexpr float TILE_PADDING = 3.0f;
	constexpr float TYPE_LINE_HEIGHT = 2.0f;

	AssetTileItem::AssetTileItem(const AssetThumbnailData& aAssetThumbnailData, const Vector2& aSize, TileView<SharedPtr<AssetThumbnailData>>* pTileView) noexcept
		:m_BackgroundColor{ DEFAULT_BACKGROUND_COLOR },
		 m_Size{ aSize },
		 m_pTileView{ pTileView } 
	{
		m_Tiled = true;
		m_CustomHoverLogic = true;

		const Vector2 thumbnailSize = Vector2(aSize.x - 10.0f, (aSize.y * 0.5f) - 4.0f);
		m_pThumbnail = aAssetThumbnailData.MakeThumbnailWidget(thumbnailSize);

		const AssetData& assetData = aAssetThumbnailData.GetAssetData();

		const AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		const IAssetDefinition* pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(assetData);
		RLS_ASSERT(pAssetDefinition, "[AssetTileItem::AssetTileItem]: Asset Definition is invalid for asset.");

		m_TypeColor = pAssetDefinition->GetAssetColor();
		m_DisplayName = pAssetDefinition->GetAssetDisplayName();
		m_Name = assetData.Name;
	}

	const Color& AssetTileItem::GetBackgroundColor() const noexcept
	{
		static constexpr Color selectedColor = Color(30.0f, 120.0f, 255.0f, 200.0f);
		static constexpr Color hoveredColor = Color(87.0f, 87.0f, 87.0f, 255.0f);
		static constexpr Color defaultColor = Color(56.0f, 56.0f, 56.0f, 255.0f);

		if (m_pTileView->IsItemSelected(m_pTileView->GetItemFromWidget(this)))
			return selectedColor;
		else if (IsHovered())
			return hoveredColor;
		else
			return defaultColor;
	}

	uint32 AssetTileItem::GetNumColumns() noexcept
	{
		return 1u;
	}

	AssetTileItem* AssetTileItem::SetBackgroundColor(const Color& aColor) noexcept
	{
		m_BackgroundColor = aColor;
		return this;
	}

	Vector2 AssetTileItem::ReportSize() const noexcept
	{
		return m_Size;
	}

	void AssetTileItem::OnRenderColumn(MAYBE_UNUSED uint32 aColumn) noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		const Vector2 thumbnailSize = m_pThumbnail->ReportSize();

		const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(m_Size.x, m_Size.y));
		ImGui::SetNextItemAllowOverlap();

		ImGui::SetCursorScreenPos(cursorPos);

		const ImVec2 thumbnailMinPoint(cursorPos);
		const ImVec2 thumbnailMaxPoint(thumbnailMinPoint.x + m_Size.x, thumbnailMinPoint.y + m_Size.y);

		constexpr const ImU32 black = IM_COL32(0, 0, 0, 128);
		pDrawList->AddRectFilled(thumbnailMinPoint, thumbnailMaxPoint, black, 7.0f);

		const ImVec2 thumbnailClientMinPoint = ImVec2(thumbnailMinPoint.x + TILE_BORDER_SIZE, thumbnailMinPoint.y + TILE_BORDER_SIZE);
		const ImVec2 thumbnailClientMaxPoint = ImVec2(thumbnailMaxPoint.x - TILE_BORDER_SIZE, thumbnailMaxPoint.y - TILE_BORDER_SIZE);
		const float midY = thumbnailClientMinPoint.y + (thumbnailClientMaxPoint.y - thumbnailClientMinPoint.y) * 0.5f;

		const Color backgroundColor = GetBackgroundColor();
		const ImU32 bgColor = IM_COL32(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
		pDrawList->AddRectFilled(thumbnailClientMinPoint, thumbnailClientMaxPoint, bgColor, 7.0f);

		const ImVec2 thumbnailImageMinPoint = ImVec2(thumbnailClientMinPoint.x + TILE_PADDING, thumbnailClientMinPoint.y + TILE_PADDING);
		const ImVec2 thumbnailImageMaxPoint = ImVec2(thumbnailClientMaxPoint.x - TILE_PADDING, midY);

		constexpr const ImU32 imgColor = IM_COL32(255, 255, 255, 255);
		pDrawList->AddRectFilled(thumbnailImageMinPoint, thumbnailImageMaxPoint, imgColor, 1.0f);

		ImGui::SetCursorScreenPos(thumbnailImageMinPoint);
		m_pThumbnail->Render();
		ImGui::SetCursorScreenPos(ImVec2(thumbnailImageMinPoint.x, thumbnailImageMaxPoint.y + 5.0f));

		ImVec2 cursorPosLocal = ImGui::GetCursorPos();

		// Width available for the text inside the client rect
		const float textRegionWidth = (thumbnailClientMaxPoint.x - TILE_PADDING) - (thumbnailClientMinPoint.x + TILE_PADDING);

		ImGui::SetWindowFontScale(0.85f);
		ImGui::PushTextWrapPos(cursorPosLocal.x + textRegionWidth);
		ImGui::TextWrapped("%s", m_Name.c_str());
		ImGui::PopTextWrapPos();
		ImGui::SetWindowFontScale(1.0f);

		ImGui::SetWindowFontScale(0.75f);
		const float textHeight = ImGui::CalcTextSize(m_DisplayName.c_str()).y;
		ImGui::SetCursorScreenPos(ImVec2(thumbnailImageMinPoint.x, thumbnailClientMaxPoint.y - textHeight));
		ImGui::PushTextWrapPos(cursorPosLocal.x + textRegionWidth);
		ImGui::TextWrapped("%s", m_DisplayName.c_str());
		ImGui::PopTextWrapPos();
		ImGui::SetWindowFontScale(1.0f);
	}

	void AssetTileItem::OnMouseButtonDown(MAYBE_UNUSED const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		m_OnClickedCallback(aPointerInfo);
	}

	void AssetTileItem::OnMouseButtonDoubleClick(MAYBE_UNUSED const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		m_OnDoubleClickedCallback(aPointerInfo, this);
	}
}