#pragma once
#include "IBaseWidget.h"

namespace Relentless
{
	template<class DerivedType>
	class IWidget : public IBaseWidget
	{
	public:
		IWidget() noexcept = default;
		virtual ~IWidget() noexcept = default;
		
		void AddFlags(int someFlags) noexcept;

		void ForceKeyboardFocus() noexcept;

		NO_DISCARD int GetFlags() const noexcept;

		NO_DISCARD bool HasFlags(int someFlags) const noexcept;

		NO_DISCARD virtual bool IsHovered() const noexcept override;

		virtual void OnMouseButtonDown(const WidgetGeometry&, const PointerInfo&) noexcept;
		virtual void OnMouseButtonDoubleClick(const WidgetGeometry&, const PointerInfo&) noexcept;
		virtual void OnMouseButtonUp(const WidgetGeometry&, const PointerInfo&) noexcept {};
		virtual void OnMouseEnter(const WidgetGeometry&, const PointerInfo&) noexcept {};
		virtual void OnMouseLeave(const PointerInfo&) noexcept {};
		virtual void OnMouseMove(const WidgetGeometry&, const PointerInfo&) noexcept {};

		template<typename InstanceType>
		DerivedType* OnMouseEnter(InstanceType* aInstance, void(InstanceType::*aMethod)(DerivedType*)) noexcept;

		template<typename T>
		DerivedType* OnMouseEnter(T&& aCallback) noexcept;

		template<typename InstanceType>
		DerivedType* OnMouseExit(InstanceType* aInstance, void(InstanceType::*aMethod)(DerivedType*)) noexcept;

		template<typename T>
		DerivedType* OnMouseExit(T&& callback) noexcept;

		void RemoveFlags(int someFlags) noexcept;
		virtual void Render() noexcept override;

		DerivedType* SetFlags(int someFlags) noexcept;
		DerivedType* SetTooltip(Ref<Tooltip> aTooltip) noexcept;
		DerivedType* SetTooltipText(StringView aText) noexcept;

		Broadcaster<void(const WidgetGeometry&, const PointerInfo&)> OnMouseDown;
		Broadcaster<void(const WidgetGeometry&, const PointerInfo&)> OnMouseDoubleClick;
		Broadcaster<void()> OnPreRenderEnd;
		Broadcaster<void()> OnRenderEnd;
		Broadcaster<void()> OnPostRenderEnd;
	protected:
		virtual void OnMouseEnter_private() noexcept;
		virtual void OnMouseExit_private() noexcept;
		virtual void OnPreRender() noexcept {};
		virtual void OnRender() noexcept = 0;
		virtual void OnPostRender() noexcept {};
		
		void ResolveGeometry() noexcept;
		void ResolveHoverState() noexcept;
		void ResolveMouseStates() noexcept;

		void ShowTooltipIfApplicable() noexcept;

		Callback<void(DerivedType*)> m_OnMouseEnterCallback;
		Callback<void(DerivedType*)> m_OnMouseExitCallback;
	protected:
		bool m_IsHovered = false;
		bool m_ShouldForceKeyboardFocus = false;
		Ref<Tooltip> m_pTooltip = nullptr;
	private:
		int m_Flags = 0;
		float m_ElapsedHoverTime = 0.0f;
		float m_ShowTooltipThreshold = 0.15f;
	};

	/******PUBLIC******/

	template<class DerivedType>
	bool IWidget<DerivedType>::HasFlags(int someFlags) const noexcept
	{
		return (m_Flags & someFlags) == someFlags;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::AddFlags(int someFlags) noexcept
	{
		m_Flags |= someFlags;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::ForceKeyboardFocus() noexcept
	{
		m_ShouldForceKeyboardFocus = true;
	}

	template<class DerivedType>
	int IWidget<DerivedType>::GetFlags() const noexcept
	{
		return m_Flags;
	}

	template<class DerivedType>
	bool IWidget<DerivedType>::IsHovered() const noexcept
	{
		return m_IsHovered;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::OnMouseButtonDoubleClick(const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		OnMouseDoubleClick(aWidgetGeometry, aPointerInfo);
	}

	template<class DerivedType>
	void IWidget<DerivedType>::OnMouseButtonDown(const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept
	{
		OnMouseDown(aWidgetGeometry, aPointerInfo);
	}

	template<typename DerivedType>
	template<typename InstanceType>
	DerivedType* IWidget<DerivedType>::OnMouseEnter(InstanceType* aInstance, void(InstanceType::*aMethod)(DerivedType*)) noexcept
	{
		m_OnMouseEnterCallback = [aInstance, aMethod](DerivedType* aDerived) { return (aInstance->*aMethod)(aDerived); };
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	template<typename T>
	DerivedType* IWidget<DerivedType>::OnMouseEnter(T&& aCallback) noexcept
	{
		m_OnMouseEnterCallback = Callback<void(DerivedType*)>(std::forward<T>(aCallback));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	template<typename InstanceType>
	DerivedType* IWidget<DerivedType>::OnMouseExit(InstanceType* aInstance, void(InstanceType::*aMethod)(DerivedType*)) noexcept
	{
		m_OnMouseExitCallback = [aInstance, aMethod](DerivedType* aDerived) { return (aInstance->*aMethod)(aDerived); };
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	template<typename T>
	DerivedType* IWidget<DerivedType>::OnMouseExit(T&& aCallback) noexcept
	{
		m_OnMouseExitCallback = Callback<void(DerivedType*)>(std::forward<T>(aCallback));
		return static_cast<DerivedType*>(this);
	}

	template<class DerivedType>
	void IWidget<DerivedType>::RemoveFlags(int someFlags) noexcept
	{
		m_Flags &= ~someFlags;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::Render() noexcept
	{
		ImGui::PushID((const void*)this);

		const bool isEnabled = IsEnabled();

		if (!isEnabled)
			ImGui::BeginDisabled();

		OnPreRender();
		OnPreRenderEnd();

		if (m_ShouldForceKeyboardFocus)
		{
			ImGui::SetKeyboardFocusHere();
			m_ShouldForceKeyboardFocus = false;
		}

		ImGui::BeginGroup();
		OnRender();
		ImGui::EndGroup();

		ResolveGeometry();
		ResolveHoverState();
		ResolveMouseStates();
		
		OnRenderEnd();

		OnPostRender();
		OnPostRenderEnd();

		if (!isEnabled)
			ImGui::EndDisabled();

		ShowTooltipIfApplicable();

		ImGui::PopID();
	}

	template<class DerivedType>
	DerivedType* IWidget<DerivedType>::SetFlags(int someFlags) noexcept
	{
		m_Flags = someFlags;
		return static_cast<DerivedType*>(this);
	}

	template<class DerivedType>
	DerivedType* IWidget<DerivedType>::SetTooltip(Ref<Tooltip> aTooltip) noexcept
	{
		m_pTooltip = aTooltip;
		return static_cast<DerivedType*>(this);
	}

	template<class DerivedType>
	DerivedType* IWidget<DerivedType>::SetTooltipText(StringView aText) noexcept
	{
		if (!m_pTooltip)
			m_pTooltip = RLS_NEW Tooltip(aText);
		else
			m_pTooltip->SetText(aText);

		return static_cast<DerivedType*>(this);
	}

	/******PROTECTED******/

	template<class DerivedType>
	void IWidget<DerivedType>::OnMouseEnter_private() noexcept
	{
		m_OnMouseEnterCallback.ExecuteIfSet(static_cast<DerivedType*>(this));
		m_IsHovered = true;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::OnMouseExit_private() noexcept
	{
		m_OnMouseExitCallback.ExecuteIfSet(static_cast<DerivedType*>(this));
		m_IsHovered = false;
		m_ElapsedHoverTime = 0.0f;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::ResolveGeometry() noexcept
	{
		const ImVec2 absolutePosition = ImGui::GetItemRectMin();
		const ImVec2 size = ImGui::GetItemRectSize();
		m_Geometry.AbsolutePosition = Vector2(absolutePosition.x, absolutePosition.y);
		m_Geometry.Size = Vector2(size.x, size.y);
	}

	template<class DerivedType>
	void IWidget<DerivedType>::ResolveHoverState() noexcept
	{
		const bool hovers = ImGui::IsItemHovered(GetHoverFlags());
		if (hovers && !m_IsHovered)
		{
			OnMouseEnter(m_Geometry, Mouse::CreatePointerInfo());
			OnMouseEnter_private();
		}
		else if (!hovers && m_IsHovered)
		{
			OnMouseLeave(Mouse::CreatePointerInfo());
			OnMouseExit_private();
		}

		m_IsHovered = hovers;
	}

	template<class DerivedType>
	void IWidget<DerivedType>::ResolveMouseStates() noexcept
	{
		if (!m_IsHovered)
			return;

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Middle))
			OnMouseButtonDoubleClick(m_Geometry, Mouse::CreatePointerInfo());
		else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
			OnMouseButtonDown(m_Geometry, Mouse::CreatePointerInfo());
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right) || ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
			OnMouseButtonUp(m_Geometry, Mouse::CreatePointerInfo());

		if (const ImVec2 delta = ImGui::GetIO().MouseDelta; delta.x != 0.0f || delta.y != 0.0f)
			OnMouseMove(m_Geometry, Mouse::CreatePointerInfo());
	}

	template<class DerivedType>
	void IWidget<DerivedType>::ShowTooltipIfApplicable() noexcept
	{
		if (!m_pTooltip)
			return;

		if (m_IsHovered)
			m_ElapsedHoverTime += Time::GetDeltaTime();

		if (m_IsHovered && m_ElapsedHoverTime >= m_ShowTooltipThreshold)
			m_pTooltip->OnRender();
	}
}