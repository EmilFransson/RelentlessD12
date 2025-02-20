#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IPanel
	{
	public:
		virtual void Render() noexcept = 0;
	private:

	};

	class PanelBase : public IPanel
	{
	public:
		PanelBase(const char* pName, ImGuiWindowFlags flags) noexcept;
		virtual void Render() noexcept override final;

		[[nodiscard]] const Vector2u& GetContentRegionAvail() const noexcept;
		[[nodiscard]] const Vector2u& GetPosition() const noexcept;
		[[nodiscard]] const Vector2u& GetSize() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] bool IsDocked() const noexcept;
		[[nodiscard]] bool IsFocused() const noexcept;
		[[nodiscard]] bool IsHovered() const noexcept;
	protected:
		virtual void PreRender() noexcept {}
		virtual void OnRender() noexcept = 0;
		virtual void PostRender() noexcept {}

	private:
		std::string m_Name{};
		ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;

		Vector2u m_ContentRegionAvail;
		Vector2u m_Position;
		Vector2u m_Size;

		bool m_IsDocked = false;
		bool m_IsFocused = false;
		bool m_IsHovered = false;
	};
}