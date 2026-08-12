#include "AssetDropTarget.h"

#include "Module/UIModule.h"

#include "UI/DragDrop/AssetViewDragDropOperation.h"

namespace Relentless
{
	AssetDropTarget::AssetDropTarget() noexcept
	{
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		uiModule.OnDragDropOperationBegin.Connect(this, &AssetDropTarget::OnDragDropOperationBegin);
		uiModule.OnDragDropOperationEnd.Connect(this, &AssetDropTarget::OnDragDropOperationEnd);
	}

	AssetDropTarget::~AssetDropTarget() noexcept
	{
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		uiModule.OnDragDropOperationBegin.Detach(this);
		uiModule.OnDragDropOperationEnd.Detach(this);
	}

	Vector2 AssetDropTarget::ReportSize() const noexcept
	{
		return m_pWidget ? m_pWidget->ReportSize() : Vector2::Zero;
	}

	bool AssetDropTarget::RequiresAssignedSize() const noexcept
	{
		return m_pWidget ? (m_pWidget->IsContainer() || m_pWidget->RequiresAssignedSize()) : false;
	}

	void AssetDropTarget::OnRender() noexcept
	{
		if (!m_pWidget)
			return;

		if (m_pWidget->IsContainer() || m_pWidget->RequiresAssignedSize())
			m_pWidget->AssignSize(GetAssignedSize());

		ImGui::BeginGroup();
		m_pWidget->Render();
		ImGui::EndGroup();

		if (m_DrawDropAreaOverlay)
		{
			ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImVec4(m_DropAreaOverlayColor.R(), m_DropAreaOverlayColor.G(), m_DropAreaOverlayColor.B(), 0.15f)), 2.0f);
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImVec4(m_DropAreaOverlayColor.R(), m_DropAreaOverlayColor.G(), m_DropAreaOverlayColor.B(), 0.5f)), 2.0f, 0, 2.0f);
		}
		else if (m_pWidget->IsHovered())
			ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)), 2.0f, 0, 2.0f);
	}

	void AssetDropTarget::OnSlotDragEnter(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!m_OnAreAssetsAcceptableCallback.IsSet())
			return;

		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
		{
			m_DropAreaOverlayColor = Colors::Red;
			m_DrawDropAreaOverlay = true;
			return;
		}

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		if (!m_OnAreAssetsAcceptableCallback(assetViewDragDropOperation.GetAssets()))
		{
			m_DropAreaOverlayColor = Colors::Red;
			m_DrawDropAreaOverlay = true;
		}
	}

	void AssetDropTarget::OnSlotDragLeave(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!m_OnAreAssetsAcceptableCallback.IsSet())
			return;

		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
		{
			m_DrawDropAreaOverlay = false;
			return;
		}

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		if (!m_OnAreAssetsAcceptableCallback(assetViewDragDropOperation.GetAssets()))
			m_DrawDropAreaOverlay = false;
	}

	Reply AssetDropTarget::OnSlotDragOver(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!m_OnAreAssetsAcceptableCallback.IsSet())
			return Reply::Unhandled();

		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Unhandled();

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		return m_OnAreAssetsAcceptableCallback(assetViewDragDropOperation.GetAssets()) ? Reply::Handled() : Reply::Unhandled();
	}

	Reply AssetDropTarget::OnSlotDrop(MAYBE_UNUSED const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Unhandled();

		if (!m_OnAssetsDroppedCallback.IsSet())
			return Reply::Unhandled();

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		m_OnAssetsDroppedCallback(assetViewDragDropOperation.GetAssets());

		return Reply::Handled();
	}

	void AssetDropTarget::OnDragDropOperationBegin(const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		m_DrawDropAreaOverlay = aDragDropOperation->IsOfType<AssetViewDragDropOperation>() &&
			m_OnAreAssetsAcceptableCallback.IsSet() &&
			m_OnAreAssetsAcceptableCallback(aDragDropOperation->AsType<AssetViewDragDropOperation>().GetAssets());
	}

	void AssetDropTarget::OnDragDropOperationEnd(MAYBE_UNUSED const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		m_DrawDropAreaOverlay = false;
		m_DropAreaOverlayColor = Colors::Green;
	}
}