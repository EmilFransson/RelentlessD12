#include "IntDrag.h"

namespace Relentless
{
	IntDrag::IntDrag(float aSpeed, int32 aMin, int32 aMax, const char* aFormat, int someFlags) noexcept
		: m_Min{ aMin }
		, m_Max{ aMax }
		, m_Speed{ aSpeed }
	{
		if (aFormat == nullptr || aFormat[0] == '\0')
			m_Format = "%.2f";
		else
			m_Format = aFormat;

		SetFlags(someFlags);

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5, 12.5f, 255.0f);
		SetBackgroundColor(defaultFrameColor);
		SetHoverColor(defaultFrameColor);
		SetActiveColor(defaultFrameColor);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetHandleColor(Color(0.35f, 0.35f, 0.35f, 255.0f));

		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float IntDrag::CalcDesiredWidth() const noexcept
	{
		if (GetSizePolicy() == ESizePolicy::Stretch)
			return 0.0f;

		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;

		const float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;
		const float indicatorWidth = m_DrawColorIndicator ? 5.0f + 6.0f : 0.0f; // rect + spacing

		return valueTextWidth + grabSize + indicatorWidth + 6.0f; // extra spacing
	}

	void IntDrag::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void IntDrag::OnRender() noexcept
	{
		if (m_IsActive)
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		auto cursorPosPreDraw = ImGui::GetCursorScreenPos();

		int32 value = m_ValueCallback();
		m_IsUsing = ImGui::DragInt("##DragInt", &value, m_Speed, m_Min, m_Max, m_Format.c_str(), GetFlags());

		auto cursorPosPostDraw = ImGui::GetCursorScreenPos();

		if (m_Delta != Vector2i::Zero())
		{
			value += (m_Speed * m_Delta.x);
			value = Math::Max(value, m_Min);
			value = Math::Min(value, m_Max);
			m_Delta = Vector2i::Zero();
			m_IsUsing = true;
		}

		if (m_IsUsing)
			m_OnChanged(value);

		m_IsHovered = ImGui::IsItemHovered();

		if (m_IsHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);

		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			SetActive(true);
		else if (!isActive && m_IsActive)
			SetActive(false);

		const ImVec2 size = ImGui::GetItemRectSize();

		if (m_DrawColorIndicator)
		{
			const ImVec2 indicatorStartLocation = ImVec2(cursorPosPreDraw.x + 3.0f, cursorPosPreDraw.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 2.0f, min.y + size.y - 11.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColor.R(), m_IndicatorColor.G(), m_IndicatorColor.B(), m_IndicatorColor.A())), 3.0f);

			ImGui::SetCursorScreenPos(cursorPosPostDraw);
		}
	}

	Vector2 IntDrag::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;

		char valueBuffer[64];
		const float value = m_ValueCallback.IsSet() ? m_ValueCallback() : 0.0f;
		ImFormatString(valueBuffer, sizeof(valueBuffer), m_Format.c_str(), value);

		const float rawWidth = ImGui::CalcTextSize(valueBuffer).x;
		float width = rawWidth + padding.x;
		width = Math::Max(width, 100.0f);

		if (pFont)
			ImGui::PopFont();

		return { width, frameHeight };
	}

	void IntDrag::SetDrawColorIndicator(bool state) noexcept
	{
		m_DrawColorIndicator = state;
	}

	void IntDrag::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IntDrag::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
	}

	IntDrag* IntDrag::SetIndicatorColor(const Color& color) noexcept
	{
		SetDrawColorIndicator(true);
		m_IndicatorColor = color;
		return this;
	}

	void IntDrag::SetActive(bool state) noexcept
	{
		OnActiveChanged(state);
		m_IsActive = state;

		if (m_IsActive)
		{
			Mouse::HideCursor();

			const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
			Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
			Mouse::OnRawMove.Connect(this, &IntDrag::OnMouseRawMove);
		}
		else
		{
			Mouse::ShowCursor();
			Mouse::FreeCursor();
			Mouse::OnRawMove.Detach(this);
			m_Delta = Vector2i::Zero();
		}
	}

	void IntDrag::OnMouseRawMove(const Vector2i& delta) noexcept
	{
		m_Delta = delta;
	}
}
