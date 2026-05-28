#include "Panel.h"

#include "Module/UIModule.h"

#include "UI/Widgets/IWidget.h"

namespace Relentless
{
	PanelBase::PanelBase(const char* pName, ImGuiWindowFlags aFlags) noexcept
		: m_Name{ std::format("{}###{}", pName, (const void*)this) }, m_Flags{ aFlags }
	{
	}

	PanelBase::~PanelBase() noexcept{ }

	bool PanelBase::AcceptsMouseInput() const noexcept
	{
		return !ImGui::GetIO().WantCaptureMouse;
	}

	IPanel::~IPanel() noexcept {}

	bool IPanel::OnEvent(IEvent& aEvent) noexcept
	{
		switch (aEvent.GetEventType())
		{
		case EventType::KeyPressedEvent:				return OnKeyPressedEvent(static_cast<KeyPressedEvent&>(aEvent));
		case EventType::KeyReleasedEvent:				return OnKeyReleasedEvent(static_cast<KeyReleasedEvent&>(aEvent));
		case EventType::LeftMouseButtonPressedEvent:	return OnLeftMouseButtonPressedEvent(static_cast<LeftMouseButtonPressedEvent&>(aEvent));
		case EventType::LeftMouseButtonReleasedEvent:	return OnLeftMouseButtonReleasedEvent(static_cast<LeftMouseButtonReleasedEvent&>(aEvent));
		case EventType::RightMouseButtonPressedEvent:	return OnRightMouseButtonPressedEvent(static_cast<RightMouseButtonPressedEvent&>(aEvent));
		case EventType::RightMouseButtonReleasedEvent:	return OnRightMouseButtonReleasedEvent(static_cast<RightMouseButtonReleasedEvent&>(aEvent));
		case EventType::MiddleMouseButtonPressedEvent:	return OnMiddleMouseButtonPressedEvent(static_cast<MiddleMouseButtonPressedEvent&>(aEvent));
		case EventType::MiddleMouseButtonReleasedEvent: return OnMiddleMouseButtonReleasedEvent(static_cast<MiddleMouseButtonReleasedEvent&>(aEvent));
		case EventType::MouseWheelScrolledEvent:		return OnMouseWheelScrolledEvent(static_cast<MouseWheelScrolledEvent&>(aEvent));
		case EventType::MouseBeginDragEvent:			return OnMouseBeginDragEvent(static_cast<MouseBeginDragEvent&>(aEvent));
		case EventType::MouseDragEvent:					return OnMouseDragEvent(static_cast<MouseDragEvent&>(aEvent));
		case EventType::MouseEndDragEvent:				return OnMouseEndDragEvent(static_cast<MouseEndDragEvent&>(aEvent));
		default: return false;
		}
	}

	void PanelBase::RebuildName() noexcept
	{
		m_Name = std::format("{}###{}_{}", GetDisplayName(), GetPersistKey(), m_Slot);
	}

	void PanelBase::Render() noexcept
	{
		const Vector2 defaultSize = GetDefaultSize();
		ImGui::SetNextWindowSize(ImVec2(defaultSize.x, defaultSize.y), ImGuiCond_FirstUseEver);

		for (const auto& [style, value] : m_Styles)
			ImGui::PushStyleVar(style, value);

		bool open = true;
		ImGui::Begin(m_Name.c_str(), &open, m_Flags);
		ImGui::PopStyleVar(m_Styles.size());

		if (!open)
			ModuleManager::LoadModuleChecked<UIModule>().RequestClose(this);
		
		SyncVisibility();
		SyncDocking();
		SyncContentRegion();
		SyncPosition();
		SyncSize();
		SyncFocus();
		SyncHover();
		SyncActivation();

		if (m_IsVisible)
		{
			PreRender();
			OnRender();

			if (m_pRoot)
			{
				const ImVec2 size = ImGui::GetContentRegionAvail();
				constexpr float safetyMargin = 5.0f;
				if (size.x > safetyMargin && size.y > safetyMargin)
				{
					m_pRoot->AssignSize(Vector2(size.x, size.y));
					m_pRoot->Render();
				}
			}
		
			PostRender();
			OnPostRender();
		}

		ImGui::End();
	}

	void PanelBase::SetPadding(const Vector2& padding)
	{
		m_Styles[ImGuiStyleVar_WindowPadding] = ImVec2(padding.x, padding.y);
	}

	const Vector2u& PanelBase::GetContentRegionAvail() const noexcept
	{
		return m_ContentRegionAvail;
	}

	const Vector2u& PanelBase::GetContentRegionMin() const noexcept
	{
		return m_ContentRegionMin;
	}

	const Vector2u& PanelBase::GetContentRegionMax() const noexcept
	{
		return m_ContentRegionMax;
	}

	FloatRect PanelBase::GetContentRegionInScreenSpace() const noexcept
	{
		FloatRect region;
		region.Top		= static_cast<float>(m_Position.y + m_ContentRegionMin.y);
		region.Left		= static_cast<float>(m_Position.x + m_ContentRegionMin.x);
		region.Right	= static_cast<float>(m_Position.x + m_ContentRegionMax.x);
		region.Bottom	= static_cast<float>(m_Position.y + m_ContentRegionMax.y);

		return region;
	}

	uint32 PanelBase::GetLastFrameFocused() const noexcept
	{
		return m_LastFrameFocused;
	}

	const Vector2u& PanelBase::GetPosition() const noexcept
	{
		return m_Position;
	}

	const Vector2u& PanelBase::GetSize() const noexcept
	{
		return m_Size;
	}

	const std::string& PanelBase::GetName() const noexcept
	{
		return m_Name;
	}

	void PanelBase::InitializeIdentity(uint32 aSlot) noexcept
	{
		m_Slot = aSlot;
		RebuildName();
	}

	bool PanelBase::IsDocked() const noexcept
	{
		return m_IsDocked;
	}

	bool PanelBase::IsFocused() const noexcept
	{
		return m_IsFocused;
	}

	bool PanelBase::IsHovered() const noexcept
	{
		return m_IsHovered;
	}

	bool PanelBase::IsVisible() const noexcept
	{
		return m_IsVisible;
	}

	void PanelBase::SetRoot(Ref<IBaseWidget> pRoot) noexcept
	{
		m_pRoot = pRoot;
	}

	void PanelBase::SyncActivation() noexcept
	{
		if (m_IsHovered && (Mouse::IsButtonPressed(RLS_Button::Left) || Mouse::IsButtonPressed(RLS_Button::Right)))
			OnActivated(this);
	}

	void PanelBase::SyncDocking() noexcept
	{
		const bool isDocked = ImGui::IsWindowDocked();

		if (m_IsDocked == isDocked)
			return;

		m_IsDocked = isDocked;

		if (m_IsDocked)
			OnDocked(this);
		else
			OnUndocked(this);
	}

	void PanelBase::SyncFocus() noexcept
	{
		const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		if (focused == m_IsFocused)
			return;
			
		m_IsFocused = focused;

		if (m_IsFocused)
		{
			m_LastFrameFocused = Time::GetFrameCount();
			OnGainedFocus(this);
		}
		else
			OnLostFocus(this);
	}

	void PanelBase::SyncHover() noexcept
	{
		const bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		if (hovered == m_IsHovered)
			return;
		
		m_IsHovered = hovered;
		
		if (m_IsHovered)
			OnMouseEnter(this);
		else
			OnMouseExit(this);
	}

	void PanelBase::SyncPosition() noexcept
	{
		const Vector2u position = Vector2u(static_cast<uint32>(ImGui::GetWindowPos().x), static_cast<uint32>(ImGui::GetWindowPos().y));
	
		if (m_Position == position)
			return;

		m_Position = position;
		OnMoved(this);
	}

	void PanelBase::SyncSize() noexcept
	{
		const Vector2u size = Vector2u(static_cast<uint32>(ImGui::GetWindowWidth()), static_cast<uint32>(ImGui::GetWindowHeight()));
		if (m_Size == size)
			return;

		m_Size = size;
		OnResized(this, m_Size);
	}

	void PanelBase::SyncContentRegion() noexcept
	{
		m_ContentRegionAvail = Vector2u(static_cast<uint32>(ImGui::GetContentRegionAvail().x), static_cast<uint32>(ImGui::GetContentRegionAvail().y));
		m_ContentRegionMin = Vector2u(static_cast<uint32>(ImGui::GetWindowContentRegionMin().x), static_cast<uint32>(ImGui::GetWindowContentRegionMin().y));
		m_ContentRegionMax = Vector2u(static_cast<uint32>(ImGui::GetWindowContentRegionMax().x), static_cast<uint32>(ImGui::GetWindowContentRegionMax().y));
	}

	void PanelBase::SyncVisibility() noexcept
	{
		const bool isVisible = !ImGui::IsWindowCollapsed();
		
		if (m_IsVisible == isVisible)
			return;

		m_IsVisible = isVisible;
	
		if (m_IsVisible)
			OnRestored(this);
		else
			OnCollapsed(this);
	}

}