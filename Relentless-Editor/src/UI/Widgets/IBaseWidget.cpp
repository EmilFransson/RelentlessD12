#include "IBaseWidget.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

namespace Relentless
{
	void IBaseWidget::AssignSize(const Vector2& aSize) noexcept
	{
		m_AssignedSize = aSize;
	}

	const Vector2& IBaseWidget::GetAssignedSize() const noexcept
	{
		return m_AssignedSize;
	}

	float IBaseWidget::GetFixedWidth() const noexcept
	{
		return m_FixedSize.x;
	}

	float IBaseWidget::GetFixedHeight() const noexcept
	{
		return m_FixedSize.y;
	}

	const Vector2& IBaseWidget::GetFixedSize() const noexcept
	{
		return m_FixedSize;
	}

	const WidgetGeometry& IBaseWidget::GetGeometry() const noexcept
	{
		return m_Geometry;
	}

	int IBaseWidget::GetHoverFlags() const noexcept
	{
		return m_HoverFlags;
	}

	const FloatRect& IBaseWidget::GetMargin() const noexcept
	{
		return m_Margin;
	}

	EHorizontalAlignmentPolicy IBaseWidget::GetHorizontalAlignmentPolicy() const noexcept
	{
		return m_HorizontalAlignmentPolicy;
	}

	ESizePolicy IBaseWidget::GetHorizontalSizePolicy() const noexcept
	{
		return m_SizePolicy;
	}

	const FloatRect& IBaseWidget::GetPadding() const noexcept
	{
		return m_Padding;
	}

	ESizePolicy IBaseWidget::GetVerticalSizePolicy() const noexcept
	{
		return m_VerticalSizePolicy;
	}

	EVerticalAlignmentPolicy IBaseWidget::GetVerticalAlignmentPolicy() const noexcept
	{
		return m_VerticalAlignmentPolicy;
	}

	bool IBaseWidget::HasAssignedSize() const noexcept
	{
		return m_AssignedSize != Vector2::Zero;
	}

	bool IBaseWidget::IsEnabled() const noexcept
	{
		return m_IsEnabled;
	}

	bool IBaseWidget::IsVisible() const noexcept
	{
		return m_IsVisible;
	}

	Reply IBaseWidget::OnDragDetected(const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		if (m_OnDragDetectedCallback.IsSet())
			return m_OnDragDetectedCallback(aGeometry, aPointerInfo);

		return Reply::Unhandled();
	}

	void IBaseWidget::OnDragEnter(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		m_OnDragEnterCallback.ExecuteIfSet(aGeometry, aDragDropOperation);
	}

	void IBaseWidget::OnDragLeave(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		m_OnDragLeaveCallback.ExecuteIfSet(aGeometry, aDragDropOperation);
	}

	Reply IBaseWidget::OnDragOver(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (m_OnDragOverCallback.IsSet())
			return m_OnDragOverCallback(aGeometry, aDragDropOperation);

		return Reply::Unhandled();
	}

	Reply IBaseWidget::OnDrop(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (m_OnDropCallback.IsSet())
			return m_OnDropCallback(aGeometry, aDragDropOperation);

		return Reply::Unhandled();
	}

	IBaseWidget* IBaseWidget::SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy aAlignmentPolicy) noexcept
	{
		m_HorizontalAlignmentPolicy = aAlignmentPolicy;
		return this;
	}

	IBaseWidget* IBaseWidget::SetHorizontalSizePolicy(ESizePolicy aSizePolicy) noexcept
	{
		m_SizePolicy = aSizePolicy;
		return this;
	}

	IBaseWidget* IBaseWidget::SetHoverFlags(int aHoverFlags) noexcept
	{
		m_HoverFlags = aHoverFlags;
		return this;
	}

	void IBaseWidget::SetIsEnabled(bool aIsEnabledState) noexcept
	{
		if (m_IsEnabled == aIsEnabledState)
			return;

		m_IsEnabled = aIsEnabledState;
		OnEnabledStateChanged(m_IsEnabled);
	}

	void IBaseWidget::SetIsVisible(bool aVisibleState) noexcept
	{
		if (m_IsVisible == aVisibleState)
			return;

		m_IsVisible = aVisibleState;
		OnVisibilityChanged(m_IsVisible);
	}

	void IBaseWidget::SetMargin(const FloatRect& aMargin) noexcept
	{
		m_Margin = aMargin;
	}

	IBaseWidget* IBaseWidget::SetPadding(const FloatRect& aPadding) noexcept
	{
		m_Padding = aPadding;
		return this;
	}

	void IBaseWidget::SetSize(const Vector2& aSize) noexcept
	{
		m_FixedSize = aSize;
	}

	IBaseWidget* IBaseWidget::SetVerticalSizePolicy(ESizePolicy aSizePolicy) noexcept
	{
		m_VerticalSizePolicy = aSizePolicy;
		return this;
	}

	IBaseWidget* IBaseWidget::SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy aAlignmentPolicy) noexcept
	{
		m_VerticalAlignmentPolicy = aAlignmentPolicy;
		return this;
	}

	void IBaseWidget::HandleDragDrop() noexcept
	{
		if (SupportsDrag() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();

			if (!uiModule.HasActiveDragDrop())
			{
				const Reply reply = OnDragDetected(m_Geometry, Mouse::CreatePointerInfo());
				if (reply.IsHandled())
					uiModule.SetActiveDragDropOperation(reply.GetDragDropOperation());
			}

			if (uiModule.HasActiveDragDrop())
			{
				Ref<DragDropOperationBase> pDragDropOperation = uiModule.GetActiveDragDropOperation();
				if (Ref<IBaseWidget> pPreview = pDragDropOperation->GetPreview())
				{
					if (pPreview->IsContainer() || pPreview->RequiresAssignedSize())
						pPreview->AssignSize(pPreview->ReportSize());

					pPreview->Render();
				}
			}

			ImGui::SetDragDropPayload("RLS_DRAGOP", nullptr, 0);
			ImGui::EndDragDropSource();
		}
		else if (ImGui::BeginDragDropTarget())
		{
			UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
			if (uiModule.HasActiveDragDrop())
			{
				const Ref<DragDropOperationBase>& pDragDropOperation = uiModule.GetActiveDragDropOperation();

				if (!m_IsDraggingOver)
				{
					OnDragEnter(m_Geometry, pDragDropOperation);
					m_IsDraggingOver = true;
				}

				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("RLS_DRAGOP", ImGuiDragDropFlags_AcceptBeforeDelivery))
				{
					if (OnDragOver(m_Geometry, pDragDropOperation).IsHandled() && pPayload->IsDelivery())
					{
						OnDrop(m_Geometry, pDragDropOperation);
						uiModule.ClearActiveDragDropOperation();
					}
				}
			}
			
			ImGui::EndDragDropTarget();
		}
		else if (m_IsDraggingOver)
		{
			UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
			if (uiModule.HasActiveDragDrop())
			{
				const Ref<DragDropOperationBase>& pDragDropOperation = uiModule.GetActiveDragDropOperation();
				OnDragLeave(m_Geometry, pDragDropOperation);
			}
			m_IsDraggingOver = false;
		}
	}
}