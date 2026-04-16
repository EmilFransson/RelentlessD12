#pragma once
#include <Relentless.h>

#include "ImGui/ImGuiIncludes.h"

#include "Tooltip.h"

#include "UI/DragDrop/DragDropOperation.h"

namespace Relentless
{
	enum class EHorizontalAlignmentPolicy : uint8 { Left = 0u, Center, Right };
	enum class EVerticalAlignmentPolicy : uint8 { Top = 0u, Center, Bottom };
	enum class ESizePolicy : uint8 { Auto = 0u, Fixed, Stretch };
	enum class ETextCommitType : uint8 { OnEnter = 0u, OnUserMovedFocus, OnCleared };

	struct WidgetGeometry
	{
		Vector2 AbsolutePosition = Vector2::Zero;
		Vector2 Size = Vector2::Zero;
	};

	struct ScopedStyleVar
	{
		int Count = 0;
		~ScopedStyleVar() { while (Count--) ImGui::PopStyleVar(); }
		void Push(ImGuiStyleVar idx, ImVec2 v) { ImGui::PushStyleVar(idx, v); ++Count; }
		void Push(ImGuiStyleVar idx, float v) { ImGui::PushStyleVar(idx, v); ++Count; }
	};

	class IBaseWidget : public RefCounted<IBaseWidget>
	{
	public:
		IBaseWidget() noexcept = default;
		virtual ~IBaseWidget() noexcept = default;

		void AssignSize(const Vector2& aSize) noexcept;

		NO_DISCARD const Vector2& GetAssignedSize() const noexcept;
		NO_DISCARD float GetFixedWidth() const noexcept;
		NO_DISCARD float GetFixedHeight() const noexcept;
		NO_DISCARD const Vector2& GetFixedSize() const noexcept;
		NO_DISCARD const WidgetGeometry& GetGeometry() const noexcept;
		NO_DISCARD const FloatRect& GetMargin() const noexcept;
		NO_DISCARD EHorizontalAlignmentPolicy GetHorizontalAlignmentPolicy() const noexcept;
		NO_DISCARD ESizePolicy GetHorizontalSizePolicy() const noexcept;
		NO_DISCARD const FloatRect& GetPadding() const noexcept;
		NO_DISCARD ESizePolicy GetVerticalSizePolicy() const noexcept;
		NO_DISCARD EVerticalAlignmentPolicy GetVerticalAlignmentPolicy() const noexcept;

		NO_DISCARD bool HasAssignedSize() const noexcept;

		NO_DISCARD virtual bool IsContainer() const noexcept { return false; };
		NO_DISCARD virtual bool SupportsDrag() const noexcept { return false; };
		NO_DISCARD bool IsEnabled() const noexcept;
		NO_DISCARD virtual bool IsHovered() const noexcept = 0;
		NO_DISCARD bool IsVisible() const noexcept;

		template<typename InstanceType>
		IBaseWidget* OnDragDetected(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const PointerInfo&)) noexcept;

		template<typename InstanceType>
		IBaseWidget* OnDragEnter(InstanceType* aInstance, void(InstanceType::* aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept;

		template<typename InstanceType>
		IBaseWidget* OnDragLeave(InstanceType* aInstance, void(InstanceType::* aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept;

		template<typename InstanceType>
		IBaseWidget* OnDragOver(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept;

		template<typename InstanceType>
		IBaseWidget* OnDrop(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept;

		virtual Reply OnDragDetected(const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept;
		virtual void OnDragEnter(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		virtual void OnDragLeave(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		virtual Reply OnDragOver(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		virtual Reply OnDrop(const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;

		virtual void Render() noexcept = 0;
		NO_DISCARD virtual Vector2 ReportSize() const noexcept { return Vector2::Zero; }
		NO_DISCARD virtual bool RequiresAssignedSize() const noexcept { return false; }

		IBaseWidget* SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy aAlignmentPolicy) noexcept;
		IBaseWidget* SetHorizontalSizePolicy(ESizePolicy aSizePolicy) noexcept;
		void SetIsEnabled(bool aIsEnabledState) noexcept;
		void SetIsVisible(bool aVisibleState) noexcept;
		void SetMargin(const FloatRect& aMargin) noexcept;
		IBaseWidget* SetPadding(const FloatRect& aPadding) noexcept;
		void SetSize(const Vector2& aSize) noexcept;
		IBaseWidget* SetVerticalSizePolicy(ESizePolicy aSizePolicy) noexcept;
		IBaseWidget* SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy aAlignmentPolicy) noexcept;

		Broadcaster<void(bool)> OnEnabledStateChanged;
		Broadcaster<void(bool)> OnVisibilityChanged;
	protected:
		void HandleDragDrop() noexcept;
	protected:
		WidgetGeometry m_Geometry;
		Callback<Reply(const WidgetGeometry&, const PointerInfo&)> m_OnDragDetectedCallback;
		Callback<void(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> m_OnDragEnterCallback;
		Callback<void(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> m_OnDragLeaveCallback;
		Callback<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> m_OnDragOverCallback;
		Callback<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> m_OnDropCallback;
	private:
		FloatRect m_Margin = FloatRect{};
		FloatRect m_Padding = FloatRect{};
		Vector2 m_AssignedSize = Vector2::Zero;
		Vector2 m_FixedSize = Vector2::Zero;

		EHorizontalAlignmentPolicy m_HorizontalAlignmentPolicy = EHorizontalAlignmentPolicy::Left;
		EVerticalAlignmentPolicy m_VerticalAlignmentPolicy = EVerticalAlignmentPolicy::Top;

		ESizePolicy m_SizePolicy = ESizePolicy::Auto;
		ESizePolicy m_VerticalSizePolicy = ESizePolicy::Auto;

		bool m_IsEnabled = true;
		bool m_IsVisible = true;
		bool m_IsDraggingOver = false;
	};

	template<typename InstanceType>
	IBaseWidget* IBaseWidget::OnDragDetected(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const PointerInfo&)) noexcept
	{
		m_OnDragDetectedCallback = [aInstance, aMethod](const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) { return (aInstance->*aMethod)(aGeometry, aPointerInfo); };
		return this;
	}

	template<typename InstanceType>
	IBaseWidget* IBaseWidget::OnDragEnter(InstanceType* aInstance, void(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept
	{
		m_OnDragEnterCallback = [aInstance, aMethod](const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) { return (aInstance->*aMethod)(aGeometry, aDragDropOperation); };
		return this;
	}

	template<typename InstanceType>
	IBaseWidget* IBaseWidget::OnDragLeave(InstanceType* aInstance, void(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept
	{
		m_OnDragLeaveCallback = [aInstance, aMethod](const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) { return (aInstance->*aMethod)(aGeometry, aDragDropOperation); };
		return this;
	}

	template<typename InstanceType>
	IBaseWidget* IBaseWidget::OnDragOver(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept
	{
		m_OnDragOverCallback = [aInstance, aMethod](const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) { return (aInstance->*aMethod)(aGeometry, aDragDropOperation); };
		return this;
	}

	template<typename InstanceType>
	IBaseWidget* IBaseWidget::OnDrop(InstanceType* aInstance, Reply(InstanceType::*aMethod)(const WidgetGeometry&, const Ref<DragDropOperationBase>&)) noexcept
	{
		m_OnDropCallback = [aInstance, aMethod](const WidgetGeometry& aGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) { return (aInstance->*aMethod)(aGeometry, aDragDropOperation); };
		return this;
	}
}