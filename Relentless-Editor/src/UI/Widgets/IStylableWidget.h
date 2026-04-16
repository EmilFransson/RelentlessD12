#pragma once
#include "IWidget.h"

namespace Relentless
{
	class WidgetStyle
	{
	public:
		void Apply() noexcept;
		void Discard() noexcept;

		NO_DISCARD ImFont* GetFont() const noexcept;
		NO_DISCARD Vector2 GetPadding() const noexcept;

		void SetFont(ImFont* aFont) noexcept;
		void SetStyleVar(ImGuiStyleVar aStyleVar, ImVec2 aValue) noexcept;
		void SetStyleVar(ImGuiStyleVar aStyleVar, float aValue) noexcept;
		void SetStyleColor(ImGuiCol aStyleColor, ImVec4 aValue) noexcept;
	private:
		std::unordered_map<ImGuiStyleVar, ImVec2> m_Vars1;
		std::unordered_map<ImGuiStyleVar, float> m_Vars2;
		std::unordered_map<ImGuiCol, ImVec4> m_Cols;
		ImFont* m_pFont = nullptr;
	};

	template<typename DerivedType>
	class IStylableWidget : public IWidget<DerivedType>
	{
	public:
		IStylableWidget() noexcept = default;
		virtual ~IStylableWidget() noexcept override = default;

		NO_DISCARD Vector2 GetPadding() const noexcept;
		NO_DISCARD const WidgetStyle& GetStyle() const noexcept;

		virtual void Render() noexcept override;

		virtual DerivedType* SetActiveColor(const Color& aColor) noexcept;
		DerivedType* SetAlpha(float aAlpha) noexcept;
		virtual DerivedType* SetBackgroundColor(const Color& aColor) noexcept;
		DerivedType* SetBorderColor(const Color& aColor) noexcept;
		DerivedType* SetBorderSize(float aSize) noexcept;
		DerivedType* SetFont(ImFont* aFont) noexcept;
		DerivedType* SetFrameRounding(float aRounding) noexcept;
		virtual DerivedType* SetHoverColor(const Color& aColor) noexcept;
		DerivedType* SetInnerSpacing(const Vector2& aInnerSpacing) noexcept;
		DerivedType* SetMargin(const IntRect& aMargin) noexcept;
		DerivedType* SetPadding(const Vector2& aPadding) noexcept;
		DerivedType* SetSpacing(const Vector2& aSpacing) noexcept;
		DerivedType* SetTextColor(const Color& aColor) noexcept;
	protected:
		virtual void OnPreRender() noexcept override {}
		virtual void OnPostRender() noexcept override {}
	protected:
		WidgetStyle m_Style;
	};

	template<typename DerivedType>
	Vector2 IStylableWidget<DerivedType>::GetPadding() const noexcept
	{
		return m_Style.GetPadding();
	}

	template<typename DerivedType>
	const WidgetStyle& IStylableWidget<DerivedType>::GetStyle() const noexcept
	{
		return m_Style;
	}

	template<typename DerivedType>
	void IStylableWidget<DerivedType>::Render() noexcept
	{
		if (!this->IsVisible())
			return;

		ImGui::PushID(this);

		const bool isEnabled = this->IsEnabled();

		if (!isEnabled)
			ImGui::BeginDisabled();

		m_Style.Apply();

		OnPreRender();
		this->OnPreRenderEnd();

		if (this->m_ShouldForceKeyboardFocus)
		{
			ImGui::SetKeyboardFocusHere();
			this->m_ShouldForceKeyboardFocus = false;
		}

		ImGui::BeginGroup();
		this->OnRender();
		ImGui::EndGroup();

		this->HandleDragDrop();
		this->ResolveGeometry();
		this->ResolveHoverState();
		this->ResolveMouseStates();

		this->OnRenderEnd();

		OnPostRender();
		this->OnPostRenderEnd();

		m_Style.Discard();

		if (!isEnabled)
			ImGui::EndDisabled();

		ImGui::PopID();

		this->ShowTooltipIfApplicable();
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetActiveColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBgActive, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetAlpha(float aAlpha) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_Alpha, aAlpha);
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetBackgroundColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBg, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetBorderColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Border, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetBorderSize(float aSize) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FrameBorderSize, aSize);
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetFont(ImFont* aFont) noexcept
	{
		m_Style.SetFont(aFont);
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetFrameRounding(float aRounding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FrameRounding, aRounding);
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetHoverColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_FrameBgHovered, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetInnerSpacing(const Vector2& aInnerSpacing) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(aInnerSpacing.x, aInnerSpacing.y));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetMargin(const IntRect& aMargin) noexcept
	{
		IBaseWidget::SetMargin(FloatRect(static_cast<float>(aMargin.Left), static_cast<float>(aMargin.Top), static_cast<float>(aMargin.Right), static_cast<float>(aMargin.Bottom)));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetPadding(const Vector2& aPadding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_FramePadding, ImVec2(aPadding.x, aPadding.y));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetSpacing(const Vector2& aSpacing) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(aSpacing.x, aSpacing.y));
		return static_cast<DerivedType*>(this);
	}

	template<typename DerivedType>
	DerivedType* IStylableWidget<DerivedType>::SetTextColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Text, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return static_cast<DerivedType*>(this);
	}

}