#pragma once
#include "Callback/Broadcaster.h"
#include "ImGui/ImguiLayer.h"

namespace Relentless
{
	class IWidget : public RefCounted<IWidget>
	{
	public:
		IWidget(std::string_view id) noexcept;
		virtual ~IWidget() noexcept = default;
		
		void AddFlags(int flags) noexcept;
		void AddSearchTags(Span<String> searchTags) noexcept;

		[[nodiscard]] int GetFlags() const noexcept;

		[[nodiscard]] bool IsEnabled() const noexcept;

		[[nodiscard]] bool HasFlags(int flags) const noexcept;
		[[nodiscard]] bool HasSearchTag(const String& searchTag) const noexcept;
		
		virtual void Render() noexcept;

		void SetFlags(int flags) noexcept;
		void SetIsEnabled(bool state) noexcept;
		virtual void SetWidthConstraint(float width) noexcept;
		
		Broadcaster<void(bool)> OnEnabledStateChanged;
	protected:
		void DiscardAllStylesAndColors();

		virtual void OnPreRender() noexcept {};
		virtual void OnRender() noexcept = 0;
		virtual void OnPostRender() noexcept {};

		void SetStyleColors(Span<std::pair<ImGuiCol, ImVec4>> colors) noexcept;
		void SetStyleColors(Span<std::pair<ImGuiCol, uint32>> colors) noexcept;

		void SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept;
		void SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept;
		void SetStyleVars(Span<std::pair<ImGuiStyleVar, ImVec2>> styles) noexcept;
		void SetStyleVars(Span<std::pair<ImGuiStyleVar, float>> styles) noexcept;

	protected:
		String m_ID;
		float m_WidthConstraint = -1.0f;
	private:
		std::unordered_set<String> m_SearchTags;

		int m_Flags = 0;

		uint32 m_NumStyleVars = 0u;
		uint32 m_NumStyleColors = 0u;

		bool m_IsEnabled = true;
	};
}