#pragma once
#include <Relentless.h> 

#include "IWidget.h"

#include "UI/Meta/ImGuiHelpers.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	template<typename DataType>
	class SpinBox : public IStylableWidget<SpinBox<DataType>>
	{
	public:
		SpinBox() noexcept;
		virtual ~SpinBox() noexcept;

		SpinBox* Bind(PropertyHandle<DataType>* aPropertyHandle) noexcept;

		template<typename T>
		SpinBox* OnValueChanged(T&& aCallback);

		template<typename InstanceType>
		SpinBox* OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType));

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		SpinBox* SetDelta(DataType aDelta) noexcept;
		SpinBox* SetDrawColorIndicator(bool aState) noexcept;
		SpinBox* SetIndicatorColor(const Color& aColor) noexcept;
		SpinBox* SetMaxValue(DataType aValue) noexcept;
		SpinBox* SetMinValue(DataType aValue) noexcept;
		SpinBox* SetSuffix(StringView aSuffix) noexcept;
		SpinBox* SetWrapAround(bool aState) noexcept;

		template<typename T>
		SpinBox* Value(T&& aCallback);

		template<typename InstanceType>
		SpinBox* Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)());
	private:
		void OnMouseRawMove(const Vector2i& aDelta) noexcept;
		void OnRender() noexcept override;

		void SetActive(bool aState) noexcept;
	private:
		String m_Format;
		String m_FormatAndSuffix;
		String m_Suffix;
		Callback<DataType()> m_ValueCallback;
		Callback<void(DataType)> m_OnValueChanged;

		Color m_IndicatorColor = Colors::White;
		Vector2i m_MouseDelta = Vector2i::Zero();

		DataType m_Delta = (DataType)1;
		DataType m_MinValue = (DataType)0;
		DataType m_MaxValue = (DataType)0;
		bool m_IsActive = false;
		bool m_DrawColorIndicator = false;

		PropertyHandle<DataType>* m_pPropertyHandle = nullptr;
	};

	template<typename DataType>
	SpinBox<DataType>::SpinBox() noexcept
	{
		m_Format = String(ImGuiHelpers::GetTypeFormat<DataType>());
		m_FormatAndSuffix = m_Format;
		m_MinValue = std::numeric_limits<DataType>::lowest() / (DataType)2;
		m_MaxValue = std::numeric_limits<DataType>::max() / (DataType)2;

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5, 12.5f, 255.0f);
		this->SetBackgroundColor(defaultFrameColor);
		this->SetHoverColor(defaultFrameColor);
		this->SetActiveColor(defaultFrameColor);
		this->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		this->SetFrameRounding(6.0f);
		this->SetBorderSize(2.0f);
		this->SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	template<typename DataType>
	SpinBox<DataType>::~SpinBox() noexcept
	{
		SetActive(false);
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::Bind(PropertyHandle<DataType>* aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
	}

	template<typename DataType>
	template<typename T>
	SpinBox<DataType>* SpinBox<DataType>::OnValueChanged(T&& aCallback)
	{
		m_OnValueChanged = Callback<void(DataType)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	SpinBox<DataType>* SpinBox<DataType>::OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType))
	{
		m_OnValueChanged = [aInstance, aMethod](DataType aValue) { return (aInstance->*aMethod)(aValue); };
		return this;
	}

	template<typename DataType>
	void SpinBox<DataType>::OnRender() noexcept
	{
		if (m_IsActive)
			this->SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (this->m_IsHovered)
			this->SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			this->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		auto cursorPosPreDraw = ImGui::GetCursorScreenPos();

		DataType value = (DataType)0;
		EPropertyAccessResult accessResult = EPropertyAccessResult::Fail;

		if (m_pPropertyHandle)
			accessResult = m_pPropertyHandle->GetValue(value);
		if (accessResult == EPropertyAccessResult::Fail && m_ValueCallback.IsSet())
			value = m_ValueCallback();

		constexpr ImGuiDataType dataType = ImGuiHelpers::ToImGuiDataType<DataType>();
		bool isUsing = false;
		
		if (accessResult == EPropertyAccessResult::MixedValues)
		{
			constexpr ImGuiInputTextFlags inputflags = ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_CharsNoBlank;
			isUsing = ImGui::InputScalar("##NumericEntryBox", dataType, &value, nullptr, nullptr, "Mixed", inputflags);
		}
		else
		{
			isUsing = ImGui::DragScalar("##SpinBox", dataType, &value, (float)m_Delta, &m_MinValue, &m_MaxValue, m_FormatAndSuffix.c_str(), this->GetFlags());

			if (m_MouseDelta != Vector2i::Zero())
			{
				value += (m_Delta * (DataType)m_MouseDelta.x);
				value = Math::Max(value, m_MinValue);
				value = Math::Min(value, m_MaxValue);
				m_MouseDelta = Vector2i::Zero();
				isUsing = true;
			}

			if (this->m_IsHovered)
				ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);
		}

		if (isUsing)
		{
			if (m_pPropertyHandle)
				m_pPropertyHandle->SetValue(value);
			else
				m_OnValueChanged.ExecuteIfSet(value);
		}

		this->m_IsHovered = ImGui::IsItemHovered();
		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			SetActive(true);
		else if (!isActive && m_IsActive)
			SetActive(false);

		if (accessResult != EPropertyAccessResult::MixedValues && m_DrawColorIndicator)
		{
			const ImVec2 size = ImGui::GetItemRectSize();
			const ImVec2 indicatorStartLocation = ImVec2(cursorPosPreDraw.x + 3.0f, cursorPosPreDraw.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 2.0f, min.y + size.y - 11.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColor.R(), m_IndicatorColor.G(), m_IndicatorColor.B(), m_IndicatorColor.A())), 3.0f);
		}
	}

	template<typename DataType>
	Vector2 SpinBox<DataType>::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;
		const ESizePolicy horizontalSizePolicy = this->GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = this->GetVerticalSizePolicy();
		const bool fixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool fixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		if (fixedWidth)
			size.x = this->GetFixedWidth();
		if (fixedHeight)
			size.y = this->GetFixedHeight();

		ImFont* pFont = this->GetStyle().GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = this->GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;

		if (!fixedWidth)
			size.x = 200.0f;
		if (!fixedHeight)
			size.y = frameHeight;

		if (pFont)
			ImGui::PopFont();

		return size;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetDelta(DataType aDelta) noexcept
	{
		m_Delta = aDelta;
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetDrawColorIndicator(bool aState) noexcept
	{
		m_DrawColorIndicator = aState;
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetIndicatorColor(const Color& aColor) noexcept
	{
		m_IndicatorColor = aColor;
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetMaxValue(DataType aValue) noexcept
	{
		m_MaxValue = Math::Min(aValue, std::numeric_limits<DataType>::max() / (DataType)2);
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetMinValue(DataType aValue) noexcept
	{
		m_MinValue = Math::Max(aValue, std::numeric_limits<DataType>::lowest() / (DataType)2);
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetSuffix(StringView aSuffix) noexcept
	{
		m_Suffix = aSuffix;
		m_FormatAndSuffix = m_Format + m_Suffix;
		return this;
	}

	template<typename DataType>
	SpinBox<DataType>* SpinBox<DataType>::SetWrapAround(bool aState) noexcept
	{
		if (aState)
			this->AddFlags(ImGuiSliderFlags_WrapAround);
		else
			this->RemoveFlags(ImGuiSliderFlags_WrapAround);

		return this;
	}

	template<typename DataType>
	template<typename T>
	SpinBox<DataType>* SpinBox<DataType>::Value(T&& aCallback)
	{
		m_ValueCallback = Callback<DataType()>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	SpinBox<DataType>* SpinBox<DataType>::Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)())
	{
		m_ValueCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
		return this;
	}

	template<typename DataType>
	void SpinBox<DataType>::SetActive(bool aState) noexcept
	{
		if (m_IsActive == aState)
			return;

		m_IsActive = aState;

		if (m_IsActive)
		{
			Mouse::HideCursor();

			const Vector2 cursorScreenPosition = Vector2(static_cast<float>(Mouse::GetCursorScreenPosition().x), static_cast<float>(Mouse::GetCursorScreenPosition().y));
			Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
			Mouse::OnRawMove.Connect(this, &SpinBox<DataType>::OnMouseRawMove);
		}
		else
		{
			Mouse::ShowCursor();
			Mouse::FreeCursor();
			Mouse::OnRawMove.Detach(this);
			m_MouseDelta = Vector2i::Zero();
		}
	}

	template<typename DataType>
	void SpinBox<DataType>::OnMouseRawMove(const Vector2i& aDelta) noexcept
	{
		m_MouseDelta = aDelta;
	}
}