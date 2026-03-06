#pragma once
#include <Relentless.h>
#include "ImGui/ImGuiIncludes.h"
#include "Tooltip.h"

namespace Relentless
{
	enum class EHorizontalAlignmentPolicy	: uint8 { Left = 0u, Center, Right };
	enum class EVerticalAlignmentPolicy		: uint8 { Top = 0u, Center, Bottom };
	enum class ESizePolicy					: uint8 { Auto = 0u, Fixed, Stretch };
	enum class ETextCommitType				: uint8 { OnEnter = 0u, OnUserMovedFocus, OnCleared };

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
		NO_DISCARD const FloatRect& GetMargin() const noexcept;
		NO_DISCARD EHorizontalAlignmentPolicy GetHorizontalAlignmentPolicy() const noexcept;
		NO_DISCARD ESizePolicy GetHorizontalSizePolicy() const noexcept;
		NO_DISCARD const FloatRect& GetPadding() const noexcept;
		NO_DISCARD ESizePolicy GetVerticalSizePolicy() const noexcept;
		NO_DISCARD EVerticalAlignmentPolicy GetVerticalAlignmentPolicy() const noexcept;

		NO_DISCARD bool HasAssignedSize() const noexcept;

		NO_DISCARD virtual bool IsContainer() const noexcept { return false; };
		NO_DISCARD bool IsEnabled() const noexcept;
		NO_DISCARD virtual bool IsHovered() const noexcept = 0;
		NO_DISCARD bool IsVisible() const noexcept;

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
	};

	template<class DerivedType>
	class IWidget : public IBaseWidget
	{
	public:
		IWidget() noexcept = default;
		virtual ~IWidget() noexcept = default;
		
		void AddFlags(int flags) noexcept
		{
			m_Flags |= flags;
		}

		void ForceKeyboardFocus() noexcept
		{
			m_ShouldForceKeyboardFocus = true;
		}

		[[nodiscard]] int GetFlags() const noexcept
		{
			return m_Flags;
		}

		NO_DISCARD virtual bool IsHovered() const noexcept override
		{
			return m_IsHovered;
		}

		[[nodiscard]] bool HasFlags(int flags) const noexcept
		{
			return (m_Flags & flags) == flags;
		}

		template<typename InstanceType>
		DerivedType* OnMouseEnter(InstanceType* instance, void(InstanceType::* method)(DerivedType*)) noexcept
		{
			m_OnMouseEnterCallback = [instance, method](DerivedType* pDerived) { return (instance->*method)(pDerived); };
			return static_cast<DerivedType*>(this);
		}

		template<typename T>
		DerivedType* OnMouseEnter(T&& callback) noexcept
		{
			m_OnMouseEnterCallback = Callback<void(DerivedType*)>(std::forward<T>(callback));
			return static_cast<DerivedType*>(this);
		}

		template<typename InstanceType>
		DerivedType* OnMouseExit(InstanceType* instance, void(InstanceType::* method)(DerivedType*)) noexcept
		{
			m_OnMouseExitCallback = [instance, method](DerivedType* pDerived) { return (instance->*method)(pDerived); };
			return static_cast<DerivedType*>(this);
		}

		template<typename T>
		DerivedType* OnMouseExit(T&& callback) noexcept
		{
			m_OnMouseExitCallback = Callback<void(DerivedType* pDerived)>(std::forward<T>(callback));
			return static_cast<DerivedType*>(this);
		}

		void RemoveFlags(int flags) noexcept
		{
			m_Flags &= ~flags;
		}

		virtual void Render() noexcept override
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

			OnRender();
			OnRenderEnd();

			OnPostRender();
			OnPostRenderEnd();

			if (!isEnabled)
				ImGui::EndDisabled();

			ShowTooltipIfApplicable();

			ImGui::PopID();
		}

		DerivedType* SetFlags(int someFlags) noexcept
		{
			m_Flags = someFlags;
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetTooltip(Ref<Tooltip> pTooltip) noexcept
		{
			m_pTooltip = pTooltip;
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetTooltipText(std::string_view text) noexcept
		{
			if (!m_pTooltip)
				m_pTooltip = new Tooltip(text);
			else
				m_pTooltip->SetText(text);
			
			return static_cast<DerivedType*>(this);
		}

		Broadcaster<void()> OnPreRenderEnd;
		Broadcaster<void()> OnRenderEnd;
		Broadcaster<void()> OnPostRenderEnd;

	protected:
		virtual void OnMouseEnter_private() noexcept
		{
			m_OnMouseEnterCallback.ExecuteIfSet(static_cast<DerivedType*>(this));
			m_IsHovered = true;
		}

		virtual void OnMouseExit_private() noexcept
		{
			m_OnMouseExitCallback.ExecuteIfSet(static_cast<DerivedType*>(this));
			m_IsHovered = false;
			m_ElapsedHoverTime = 0.0f;
		}

		virtual void OnPreRender() noexcept {};
		virtual void OnRender() noexcept = 0;
		virtual void OnPostRender() noexcept {};

		void ShowTooltipIfApplicable() noexcept
		{
			if (!m_pTooltip)
				return;

			if (m_IsHovered)
				m_ElapsedHoverTime += Time::GetDeltaTime();

			if (m_IsHovered && m_ElapsedHoverTime >= m_ShowTooltipThreshold)
				m_pTooltip->OnRender();
		}

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

	class WidgetStyle
	{
	public:
		void Apply() noexcept;
		void Discard() noexcept;

		NO_DISCARD ImFont* GetFont() const noexcept;
		NO_DISCARD Vector2 GetPadding() const noexcept;

		void SetFont(ImFont* pFont) noexcept;
		void SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept;
		void SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept;
		void SetStyleColor(ImGuiCol styleColor, ImVec4 value) noexcept;
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

		NO_DISCARD Vector2 GetPadding() const noexcept
		{
			return m_Style.GetPadding();
		}

		NO_DISCARD const WidgetStyle& GetStyle() const noexcept
		{
			return m_Style;
		}

		virtual void Render() noexcept override
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

			this->OnRender();
			this->OnRenderEnd();

			OnPostRender();
			this->OnPostRenderEnd();

			m_Style.Discard();

			if (!isEnabled)
				ImGui::EndDisabled();

			ImGui::PopID();

			this->ShowTooltipIfApplicable();
		}

		virtual DerivedType* SetActiveColor(const Color& color) noexcept
		{
			m_Style.SetStyleColor(ImGuiCol_FrameBgActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetAlpha(float alpha) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_Alpha, alpha);
			return static_cast<DerivedType*>(this);
		}

		virtual DerivedType* SetBackgroundColor(const Color& color) noexcept
		{
			m_Style.SetStyleColor(ImGuiCol_FrameBg, ImVec4(color.R(), color.G(), color.B(), color.A()));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetBorderColor(const Color& color) noexcept
		{
			m_Style.SetStyleColor(ImGuiCol_Border, ImVec4(color.R(), color.G(), color.B(), color.A()));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetBorderSize(float size) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_FrameBorderSize, size);
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetFont(ImFont* pFont) noexcept
		{
			m_Style.SetFont(pFont);
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetFrameRounding(float rounding) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_FrameRounding, rounding);
			return static_cast<DerivedType*>(this);
		}

		virtual DerivedType* SetHoverColor(const Color& color) noexcept
		{
			m_Style.SetStyleColor(ImGuiCol_FrameBgHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetInnerSpacing(const Vector2& innerSpacing) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(innerSpacing.x, innerSpacing.y));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetMargin(const IntRect& margin) noexcept
		{
			IBaseWidget::SetMargin(FloatRect(static_cast<float>(margin.Left), static_cast<float>(margin.Top), static_cast<float>(margin.Right), static_cast<float>(margin.Bottom)));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetPadding(const Vector2& padding) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padding.x, padding.y));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetSpacing(const Vector2& spacing) noexcept
		{
			m_Style.SetStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing.x, spacing.y));
			return static_cast<DerivedType*>(this);
		}

		DerivedType* SetTextColor(const Color& color) noexcept
		{
			m_Style.SetStyleColor(ImGuiCol_Text, ImVec4(color.R(), color.G(), color.B(), color.A()));
			return static_cast<DerivedType*>(this);
		}
	protected:
		virtual void OnPreRender() noexcept override {}
		virtual void OnPostRender() noexcept override {}
	protected:
		WidgetStyle m_Style;
	};

	template<typename DerivedType>
	class ICompoundWidget : public IStylableWidget<DerivedType>
	{
	public:
		ICompoundWidget() noexcept = default;
		virtual ~ICompoundWidget() noexcept = default;
	protected:
		virtual void OnPreRender() noexcept override {}
		virtual void OnPostRender() noexcept override {}
	};
}