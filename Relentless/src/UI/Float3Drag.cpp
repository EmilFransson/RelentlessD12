#include "Float3Drag.h"

#include "FloatDrag.h"
#include "HorizontalBox.h"

#include "Input/Mouse.h"

namespace Relentless
{
	Float3Drag::Float3Drag(float speed, float min, float max, const char* pFormat, int flags) noexcept
		: m_Speed{ speed }
		, m_Min{ min }
		, m_Max{ max }
		, m_Format{ pFormat }
	{
		using namespace std::placeholders;

		m_pFloatDragBox = new HorizontalBox();

		m_pFloatDragBox->Add(new FloatDrag(speed, min, max, pFormat, flags))
			->Value(std::bind(&Float3Drag::GetValue, this, 0))
			->OnValueChanged(std::bind(&Float3Drag::SetValue, this, _1, 0))
			->SetIndicatorColor(Colors::OffRed)
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pFloatDragBox->Add(new FloatDrag(speed, min, max, pFormat, flags))
			->Value(std::bind(&Float3Drag::GetValue, this, 1))
			->OnValueChanged(std::bind(&Float3Drag::SetValue, this, _1, 1))
			->SetIndicatorColor(Colors::OffGreen)
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pFloatDragBox->Add(new FloatDrag(speed, min, max, pFormat, flags))
			->Value(std::bind(&Float3Drag::GetValue, this, 2))
			->OnValueChanged(std::bind(&Float3Drag::SetValue, this, _1, 2))
			->SetIndicatorColor(Colors::OffBlue)
			->SetSizePolicy(ESizePolicy::Stretch);
	}

	float Float3Drag::CalcDesiredWidth() const noexcept
	{
		if (GetSizePolicy() == ESizePolicy::Stretch)
			return 0.0f;

		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;

		// Width of numeric value text (e.g. %.2f ° lux etc.)
		float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;

		// Optional color indicator width
		const float indicatorWidth = m_DrawColorIndicator ? 5.0f + 6.0f : 0.0f; // rect + spacing

		return (valueTextWidth + grabSize + indicatorWidth + 6.0f) * 3.0f; // extra spacing
	}

	float Float3Drag::GetValue(int componentIndex) const noexcept
	{
		const Vector3 values = m_ValueCallback();
		switch (componentIndex)
		{
		case 0: return values.x;
		case 1: return values.y;
		case 2: return values.z;
		}
	}

	void Float3Drag::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	//Could need another pass:
	void Float3Drag::OnRender() noexcept
	{
		m_pFloatDragBox->Render();
		return;

		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float totalSpacing = spacing * 2;
		const float availableWidth = ImGui::GetContentRegionAvail().x;
		const float stretchWidth = (availableWidth - totalSpacing) / 3;

		Vector3 value = m_ValueCallback();
		
		ImGui::SetNextItemWidth(stretchWidth);
		RenderComponent(value.x, 0);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(stretchWidth);
		RenderComponent(value.y, 1);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(stretchWidth);
		RenderComponent(value.z, 2);

		if (IsAnyUsed())
			m_OnChanged(value);
	}

	Float3Drag* Float3Drag::SetDrawColorIndicator(bool state) noexcept
	{
		m_DrawColorIndicator = state;
		return this;
	}

	Float3Drag* Float3Drag::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	Float3Drag* Float3Drag::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
		return this;
	}

	Float3Drag* Float3Drag::SetIndicatorColor(uint8 handleIndex, const Color& color) noexcept
	{
		SetDrawColorIndicator(true);
		m_IndicatorColors[handleIndex] = color;
		return this;
	}

	bool Float3Drag::IsAnyHovered() const noexcept
	{
		return std::any_of(m_IsHoveredAxis.begin(), m_IsHoveredAxis.end(), [](bool state) { return state == true; });
	}

	bool Float3Drag::IsAnyUsed() const noexcept
	{
		return std::any_of(m_IsUsingAxis.begin(), m_IsUsingAxis.end(), [](bool state) { return state == true; });
	}

	void Float3Drag::SetActive(bool state, uint8 componentIndex) noexcept
	{
		OnActiveChanged(state);
		m_IsActiveAxis[componentIndex] = state;

		if (m_IsActiveAxis[componentIndex])
		{
			Mouse::HideCursor();

			const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
			Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
			Mouse::OnRawMove.Connect(this, &Float3Drag::OnMouseRawMove);
		}
		else
		{
			Mouse::ShowCursor();
			Mouse::FreeCursor();
			Mouse::OnRawMove.Detach(this);
			m_Delta = Vector2i::Zero();
		}
	}

	void Float3Drag::SetValue(float value, int componentIndex) noexcept
	{
		Vector3 values = m_ValueCallback();
		switch (componentIndex)
		{
		case 0: values.x = value; break;
		case 1: values.y = value; break;
		case 2: values.z = value; break;
		}

		m_OnChanged(values);
	}

	void Float3Drag::RenderComponent(float& value, uint8 componentIndex) noexcept
	{
		const std::string componentLabel = std::to_string((long)this) + std::to_string(componentIndex);
		ImGui::PushID(componentLabel.c_str());

		if (m_IsActiveAxis[componentIndex])
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHoveredAxis[componentIndex])
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		auto curPos = ImGui::GetCursorScreenPos();

		m_IsUsingAxis[componentIndex] = ImGui::DragFloat("##DragFloat", &value, m_Speed, m_Min, m_Max, m_Format.c_str(), GetFlags());

		m_IsHoveredAxis[componentIndex] = ImGui::IsItemHovered();

		if (m_Delta != Vector2i::Zero() && m_IsHoveredAxis[componentIndex])
		{			
			value += (m_Speed * m_Delta.x);
			value = Math::Max(value, m_Min);
			value = Math::Min(value, m_Max);
			m_Delta = Vector2i::Zero();
			m_IsUsingAxis[componentIndex] = true;
		}

		if (m_IsHoveredAxis[componentIndex])
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);

		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActiveAxis[componentIndex])
			SetActive(true, componentIndex);
		else if (!isActive && m_IsActiveAxis[componentIndex])
			SetActive(false, componentIndex);

		const ImVec2 size = ImGui::GetItemRectSize();

		if (m_DrawColorIndicator)
		{
			const ImVec2 indicatorStartLocation = ImVec2(curPos.x + 3.0f, curPos.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 2.0f, min.y + size.y - 11.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColors[componentIndex].R(), m_IndicatorColors[componentIndex].G(), m_IndicatorColors[componentIndex].B(), m_IndicatorColors[componentIndex].A())), 3.0f);
		}

		ImGui::PopID();
	}

	void Float3Drag::OnMouseRawMove(const Vector2i& delta) noexcept
	{
		m_Delta = delta;
	}
}
