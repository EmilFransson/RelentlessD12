#include "Panel.h"

namespace Relentless
{

	PanelBase::PanelBase(const char* pName, ImGuiWindowFlags flags) noexcept
	: m_Name{ pName }, m_Flags{ flags }
	{}

	PanelBase::~PanelBase() noexcept{ }

	IPanel::~IPanel() noexcept { }

	bool IPanel::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::KeyPressedEvent:				return OnKeyPressedEvent(EVENT(KeyPressedEvent));
		case EventType::LeftMouseButtonPressedEvent:	return OnLeftMouseButtonPressedEvent(EVENT(LeftMouseButtonPressedEvent));
		case EventType::LeftMouseButtonReleasedEvent:	return OnLeftMouseButtonReleasedEvent(EVENT(LeftMouseButtonReleasedEvent));
		case EventType::RightMouseButtonPressedEvent:	return OnRightMouseButtonPressedEvent(EVENT(RightMouseButtonPressedEvent));
		case EventType::RightMouseButtonReleasedEvent:	return OnRightMouseButtonReleasedEvent(EVENT(RightMouseButtonReleasedEvent));
		case EventType::MiddleMouseButtonPressedEvent:	return OnMiddleMouseButtonPressedEvent(EVENT(MiddleMouseButtonPressedEvent));
		case EventType::MiddleMouseButtonReleasedEvent: return OnMiddleMouseButtonReleasedEvent(EVENT(MiddleMouseButtonReleasedEvent));
		case EventType::MouseWheelScrolledEvent:		return OnMouseWheelScrolledEvent(EVENT(MouseWheelScrolledEvent));
		default: return false;
		}
	}

	void PanelBase::Render() noexcept
	{
		PreRender();
		
		bool open = true;
		
		for (const auto& [style, value] : m_Styles)
			ImGui::PushStyleVar(style, value);

		ImGui::Begin(m_Name.c_str(), &open, m_Flags);

		ImGui::PopStyleVar(m_Styles.size());

		m_ContentRegionAvail = Vector2u((uint32)ImGui::GetContentRegionAvail().x, (uint32)ImGui::GetContentRegionAvail().y);
		m_ContentRegionMin = Vector2u((uint32)ImGui::GetWindowContentRegionMin().x, (uint32)ImGui::GetWindowContentRegionMin().y);
		m_ContentRegionMax = Vector2u((uint32)ImGui::GetWindowContentRegionMax().x, (uint32)ImGui::GetWindowContentRegionMax().y);

		m_Position = Vector2u((uint32)ImGui::GetWindowPos().x, (uint32)ImGui::GetWindowPos().y);

		const Vector2u size = Vector2u((uint32)ImGui::GetWindowWidth(), (uint32)ImGui::GetWindowHeight());
		if (m_Size != size)
		{
			m_Size = size;
			OnResized(this, m_Size);
		}
		
		ImGuiWindow* window = ImGui::FindWindowByName(m_Name.c_str());
		m_IsVisible = window && !window->Hidden;

		if (m_IsVisible)
		{
			OnRender();

			if (m_pRoot)
				m_pRoot->Render();
		}

		m_IsDocked = ImGui::IsWindowDocked();

		const bool focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		if (focused != m_IsFocused)
		{
			m_IsFocused = focused;

			if (m_IsFocused)
			{
				m_LastFrameFocused = Time::GetFrameCount();
				OnGainedFocus(this);
			}
			else
				OnLostFocus(this);
		}

		m_IsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

		PostRender();
		OnPostRender();

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

}