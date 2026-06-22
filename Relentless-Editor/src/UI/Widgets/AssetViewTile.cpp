#include "AssetViewTile.h"

#include "ImGui/ImGuiFonts.h"

#include "UI/Views/Assets/Items/AssetViewItem.h"
#include "UI/Views/TileView.h"

#include "UI/Widgets/Label.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	constexpr Color DEFAULT_BACKGROUND_COLOR	= Colors::Normalize(56.0f, 56.0f, 56.0f, 255.0f);
	constexpr Color HOVERED_BACKGROUND_COLOR	= Colors::Normalize(87.0f, 87.0f, 87.0f, 255.0f);
	constexpr Color SELECTED_BACKGROUND_COLOR	= Colors::Normalize(30.0f, 120.0f, 255.0f, 200.0f);
	constexpr Color DROP_SHADOW_COLOR			= Color(0.0f, 0.0f, 0.0f, 0.6f);

	NO_DISCARD bool IsCreateInfoValid(const AssetViewTileCreateInfo& aCreateInfo) noexcept
	{
		return aCreateInfo.Size.x > 0.0f && aCreateInfo.Size.y > 0.0f && aCreateInfo.Thumbnail != nullptr;
	}

	AssetViewTile::AssetViewTile(const AssetViewTileCreateInfo& aCreateInfo, TileView<SharedPtr<AssetViewItem>>* aTileView) noexcept
		: m_pTileView{ aTileView }
		, m_pThumbnail{ aCreateInfo.Thumbnail }
		, m_IsAssetTile{ aCreateInfo.IsAssetTile }
	{
		RLS_ASSERT(IsCreateInfoValid(aCreateInfo), "[AssetViewTile::AssetViewTile]: One or more create info parameters is invalid.");
		
		m_CustomHoverLogic = true;
		m_Tiled = true;
		m_HasHoverRect = true;

		m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(Colors::Transparent.R(), Colors::Transparent.G(), Colors::Transparent.B(), Colors::Transparent.A()));
		m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(Colors::Transparent.R(), Colors::Transparent.G(), Colors::Transparent.B(), Colors::Transparent.A()));
		m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(Colors::Transparent.R(), Colors::Transparent.G(), Colors::Transparent.B(), Colors::Transparent.A()));

		m_DefaultBackgroundColor = m_IsAssetTile ? DEFAULT_BACKGROUND_COLOR : Colors::Transparent;

		m_pRoot = RLS_NEW VerticalBox();
		m_pRoot->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pRoot->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pRoot->SetMouseScrollingEnabled(false);
		m_pRoot->SetSize(aCreateInfo.Size);
		m_pRoot->SetBackgroundColor(m_IsAssetTile ? DROP_SHADOW_COLOR : Colors::Transparent);

		m_pTileBox = m_pRoot->AddWidget(RLS_NEW VerticalBox());
		m_pTileBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pTileBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pTileBox->SetMouseScrollingEnabled(false);
		m_pTileBox->SetBackgroundColor(m_DefaultBackgroundColor);
		m_pTileBox->SetMargin(FloatRect(0.0f, 0.0f, 4.0f, 4.0f));

		m_pTileBox->OnMouseEnter(this, &AssetViewTile::OnMouseEnterTile);
		m_pTileBox->OnMouseExit(this, &AssetViewTile::OnMouseExitTile);
		m_pTileBox->OnMouseDown.Connect(this, &AssetViewTile::OnTileMouseDown);
		m_pTileBox->OnMouseUp.Connect(this, &AssetViewTile::OnTileMouseUp);
		m_pTileBox->OnMouseDoubleClick.Connect(this, &AssetViewTile::OnTileMouseDoubleClick);

		m_pThumbnail->OnMouseDown.Connect(this, &AssetViewTile::OnTileMouseDown);
		m_pThumbnail->OnMouseDoubleClick.Connect(this, &AssetViewTile::OnTileMouseDoubleClick);

		m_pTileBox->AddWidget(aCreateInfo.Thumbnail)
			->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center)
			->SetMargin(FloatRect::WithTop(2.0f));

		VerticalBox* pNameBox = m_pTileBox->AddWidget(RLS_NEW VerticalBox());
		pNameBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pNameBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pNameBox->SetMargin(FloatRect(2.0f, 0.0f, 2.0f, 0.0f));

		Label* pLabel = pNameBox->AddWidget(RLS_NEW Label(aCreateInfo.Name))
			->SetWrap(m_IsAssetTile)
			->SetTextAlign(m_IsAssetTile ? ETextAlign::Left : ETextAlign::Center)
			->SetHighlightedSubstring(aCreateInfo.HighlightedSubstring);
		
		if (m_IsAssetTile)
		{
			m_pTileBox->AddWidget(RLS_NEW Label(aCreateInfo.DisplayName))
				->SetFont(UI::Fonts::Get("Small"))
				->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Bottom)
				->SetMargin(FloatRect(2.0f, 0.0f, 2.0f, 0.0f));
		}
	}

	AssetViewTile::~AssetViewTile() noexcept
	{
		m_pThumbnail->OnMouseDown.Detach(this);
		m_pThumbnail->OnMouseDoubleClick.Detach(this);
	}

	uint32 AssetViewTile::GetNumColumns() noexcept
	{
		return 1u;
	}

	Vector2 AssetViewTile::ReportSize() const noexcept
	{
		return m_pRoot->GetFixedSize();
	}

	AssetViewTile* AssetViewTile::SetDefaultBackgroundColor(const Color& aColor) noexcept
	{
		m_DefaultBackgroundColor = aColor;
		return this;
	}

	void AssetViewTile::OnRenderColumn(MAYBE_UNUSED uint32 aColumn) noexcept
	{
		m_pRoot->AssignSize(m_pRoot->GetFixedSize());
		m_pRoot->Render();

		m_HoverRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

		const SharedPtr<AssetViewItem>& pItem = m_pTileView->GetItemFromWidget(this);
		const bool isSelected = m_pTileView->IsItemSelected(pItem);
		if (isSelected && !m_IsSelected)
			OnTileSelected();
		else if (!isSelected && m_IsSelected)
			OnTileDeselected();

		m_IsSelected = isSelected;
	}

	void AssetViewTile::HandleDragDrop() noexcept
	{
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();

		if (SupportsDrag() && m_HoverRect.Contains(ImGui::GetMousePos()) && !uiModule.HasActiveDragDrop() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern | ImGuiDragDropFlags_SourceNoPreviewTooltip))
			{
				const Reply reply = OnDragDetected(m_Geometry, Mouse::CreatePointerInfo());
				if (reply.IsHandled())
					uiModule.SetActiveDragDropOperation(reply.GetDragDropOperation());

				ImGui::SetDragDropPayload("RLS_DRAGOP", nullptr, 0);
				ImGui::EndDragDropSource();
			}
		}

		if (uiModule.HasActiveDragDrop())
		{
			const ImVec2 mouse = ImGui::GetMousePos();
			const bool cursorOverRow = m_HasHoverRect && m_HoverRect.Contains(mouse);

			if (cursorOverRow)
			{
				const Ref<DragDropOperationBase>& pOp = uiModule.GetActiveDragDropOperation();

				uiModule.SetDragOverTarget(this, m_Geometry);

				const Reply reply = OnDragOver(m_Geometry, pOp);
				if (reply.IsHandled() && Mouse::IsButtonReleased(RLS_Button::Left))
				{
					OnDrop(m_Geometry, pOp);
					uiModule.ClearDragOverTarget();
					uiModule.ClearActiveDragDropOperation();
				}
			}
		}
	}

	bool AssetViewTile::IsSelected() const noexcept
	{
		return m_IsSelected;
	}

	void AssetViewTile::OnMouseEnterTile(VerticalBox* aTileBox) noexcept
	{
		if (IsSelected())
			return;

		aTileBox->SetBackgroundColor(HOVERED_BACKGROUND_COLOR);
		
		if (!m_IsAssetTile)
			m_pRoot->SetBackgroundColor(DROP_SHADOW_COLOR);
	}

	void AssetViewTile::OnMouseExitTile(VerticalBox* aTileBox) noexcept
	{
		if (IsSelected())
			return;

		aTileBox->SetBackgroundColor(m_DefaultBackgroundColor);

		if (!m_IsAssetTile)
			m_pRoot->SetBackgroundColor(Colors::Transparent);
	}

	void AssetViewTile::OnTileDeselected() noexcept
	{
		if (m_pTileBox->IsHovered())
			m_pTileBox->SetBackgroundColor(HOVERED_BACKGROUND_COLOR);
		else
		{
			m_pTileBox->SetBackgroundColor(m_DefaultBackgroundColor);
			if (!m_IsAssetTile)
				m_pRoot->SetBackgroundColor(Colors::Transparent);
		}
	}

	void AssetViewTile::OnTileMouseDown(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		if (!IsSelected() && aPointerInfo.EffectingButton == RLS_Button::Left)
			m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
		else if (IsSelected() && Keyboard::IsKeyDown(RLS_Key::LCtrl) && aPointerInfo.EffectingButton == RLS_Button::Left)
			m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
	}

	void AssetViewTile::OnTileMouseDoubleClick(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		m_OnDoubleClickedCallback.ExecuteIfSet(aPointerInfo, this);
	}

	void AssetViewTile::OnTileMouseUp(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		if (IsSelected() && !Keyboard::IsKeyDown(RLS_Key::LCtrl) && aPointerInfo.EffectingButton == RLS_Button::Left)
			m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
		else if (aPointerInfo.EffectingButton == RLS_Button::Right)
			m_OnClickedCallback.ExecuteIfSet(aPointerInfo);
	}

	void AssetViewTile::OnTileSelected() noexcept
	{
		m_pTileBox->SetBackgroundColor(SELECTED_BACKGROUND_COLOR);

		if (!m_IsAssetTile)
			m_pRoot->SetBackgroundColor(DROP_SHADOW_COLOR);
	}
}