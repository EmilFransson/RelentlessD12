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
		m_ContentRegionMin = Vector2u(ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMin().y);
		m_ContentRegionMax = Vector2u(ImGui::GetWindowContentRegionMax().x, ImGui::GetWindowContentRegionMax().y);

		m_Position = Vector2u(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		m_Size = Vector2u(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
		
		ImGuiWindow* window = ImGui::FindWindowByName(m_Name.c_str());
		m_IsVisible = window && !window->Hidden;

		if (m_IsVisible)
			OnRender();

		m_IsDocked = ImGui::IsWindowDocked();
		m_IsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
		m_IsHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

		ImGui::End();

		PostRender();
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
		region.Top		= m_Position.y + m_ContentRegionMin.y;
		region.Left		= m_Position.x + m_ContentRegionMin.x;
		region.Right	= m_Position.x + m_ContentRegionMax.x;
		region.Bottom	= m_Position.y + m_ContentRegionMax.y;

		return region;
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

}