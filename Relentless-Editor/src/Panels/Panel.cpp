#include "Panel.h"

namespace Relentless
{

	PanelBase::PanelBase(const char* pName, ImGuiWindowFlags flags) noexcept
		: m_Name{pName}, m_Flags{flags}
	{}

	void PanelBase::Render() noexcept
	{
		PreRender();
		
		bool open = true;
		ImGui::Begin(m_Name.c_str(), &open, m_Flags);

		m_ContentRegionAvail = Vector2u(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
		m_Position = Vector2u(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		m_Size = Vector2u(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

		m_IsDocked = ImGui::IsWindowDocked();
		m_IsFocused = ImGui::IsWindowFocused();
		m_IsHovered = ImGui::IsWindowHovered();

		OnRender();

		ImGui::End();

		PostRender();
	}

	const Vector2u& PanelBase::GetContentRegionAvail() const noexcept
	{
		return m_ContentRegionAvail;
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

}