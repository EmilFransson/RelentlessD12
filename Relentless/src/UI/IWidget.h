#pragma once
#include "Callback/Broadcaster.h"
#include "ImGui/ImguiLayer.h"

namespace Relentless
{
	enum class ESizePolicy
	{
		Fixed,       // Use exact size from CalcDesiredWidth()
		Stretch,     // Take all available space (or a share of it)
		Auto         // Use content size, but allow layout to override if needed
	};

	class IWidget : public RefCounted<IWidget>
	{
	public:
		IWidget(std::string_view id) noexcept;
		virtual ~IWidget() noexcept = default;
		
		void AddFlags(int flags) noexcept;
		void AddSearchTags(Span<String> searchTags) noexcept;

		template<typename T>
		T* As();

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept = 0;

		[[nodiscard]] int GetFlags() const noexcept;
		[[nodiscard]] const String& GetID() const noexcept;
		[[nodiscard]] ESizePolicy GetSizePolicy() const noexcept;

		[[nodiscard]] bool IsEnabled() const noexcept;
		[[nodiscard]] bool IsVisible() const noexcept;

		[[nodiscard]] bool HasFlags(int flags) const noexcept;
		[[nodiscard]] bool HasSearchTag(const String& searchTag) const noexcept;
		
		virtual void Render() noexcept;

		void SetFlags(int flags) noexcept;
		void SetID(std::string_view id) noexcept;
		void SetIsEnabled(bool state) noexcept;
		void SetIsVisible(bool state) noexcept;
		void SetSizePolicy(ESizePolicy sizePolicy) noexcept;
		virtual void SetWidthConstraint(float width) noexcept;
		
		Broadcaster<void(bool)> OnEnabledStateChanged;
		Broadcaster<void(bool)> OnVisibilityChanged;
		Broadcaster<void()> OnPreRenderEnd;
		Broadcaster<void()> OnRenderEnd;
		Broadcaster<void()> OnPostRenderEnd;
	protected:
		void DiscardAllStylesAndColors();

		virtual void OnPreRender() noexcept {};
		virtual void OnRender() noexcept = 0;
		virtual void OnPostRender() noexcept {};

	protected:
		String m_ID;
		float m_WidthConstraint = -1.0f;
	private:
		std::unordered_set<String> m_SearchTags;
		ESizePolicy m_SizePolicy = ESizePolicy::Auto;

		int m_Flags = 0;
		bool m_IsEnabled = true;
		bool m_IsVisible = true;
	};

	template<typename T>
	T* Relentless::IWidget::As()
	{
		return static_cast<T*>(this);
	}

	class WidgetStyle
	{
	public:
		void Apply() noexcept;
		void Discard() noexcept;

		void SetFont(ImFont* pFont) noexcept;
		void SetMargin(const IntRect& margin) noexcept;
		void SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept;
		void SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept;
		void SetStyleColor(ImGuiCol styleColor, ImVec4 value) noexcept;
	private:
		std::unordered_map<ImGuiStyleVar, ImVec2> m_Vars1;
		std::unordered_map<ImGuiStyleVar, float> m_Vars2;
		std::unordered_map<ImGuiCol, ImVec4> m_Cols;
		IntRect m_Margin;
		ImFont* m_pFont = nullptr;
	};

	class IStylableWidget : public IWidget
	{
	public:
		IStylableWidget(std::string_view id) noexcept;

		virtual void Render() noexcept override;

		virtual void SetActiveColor(const Color& color) noexcept;
		void SetAlpha(float alpha) noexcept;
		virtual void SetBackgroundColor(const Color& color) noexcept;
		void SetBorderColor(const Color& color) noexcept;
		void SetBorderSize(float size) noexcept;
		void SetFont(ImFont* pFont) noexcept;
		void SetFrameRounding(float rounding) noexcept;
		virtual void SetHoverColor(const Color& color) noexcept;
		void SetInnerSpacing(const Vector2& innerSpacing) noexcept;
		void SetMargin(const IntRect& margin) noexcept;
		void SetPadding(const Vector2& padding) noexcept;
		void SetSpacing(const Vector2& spacing) noexcept;
		void SetTextColor(const Color& color) noexcept;
	protected:
		virtual void OnPreRender() noexcept override {}
		virtual void OnPostRender() noexcept override {}
	protected:
		WidgetStyle m_Style;
	};
}