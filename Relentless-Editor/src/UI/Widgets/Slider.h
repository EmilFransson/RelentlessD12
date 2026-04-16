#pragma once
#include "IStylableWidget.h"
#include "UI/Meta/ImGuiHelpers.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	template<typename DataType>
	class Slider : public IStylableWidget<Slider<DataType>>
	{
	public:
		Slider() noexcept;
		virtual ~Slider() noexcept = default;

		Slider<DataType>* Bind(Ref<PropertyHandle<DataType>> aPropertyHandle) noexcept;

		template<typename T>
		Slider* OnValueChanged(T&& aCallback);

		template<typename InstanceType>
		Slider* OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType));

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		Slider* SetDelta(DataType aDelta) noexcept;
		Slider* SetHandleColor(const Color& aColor) noexcept;
		Slider* SetHandleSize(float aSize) noexcept;
		Slider* SetLogarithmic(bool aState) noexcept;
		Slider* SetMaxValue(DataType aValue) noexcept;
		Slider* SetMinValue(DataType aValue) noexcept;
		Slider* SetSuffix(StringView aSuffix) noexcept;

		template<typename T>
		Slider* Value(T&& aCallback);

		template<typename InstanceType>
		Slider* Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)());
	private:
		void OnRender() noexcept override;

		void SetActive(bool aState) noexcept;
	private:
		String m_Format;
		String m_FormatAndSuffix;
		String m_Suffix;
		Callback<DataType()> m_ValueCallback;
		Callback<void(DataType)> m_OnValueChanged;

		DataType m_Delta = (DataType)1;
		DataType m_MinValue = (DataType)0;
		DataType m_MaxValue = (DataType)0;

		bool m_IsActive = false;

		Ref<PropertyHandle<DataType>> m_pPropertyHandle = nullptr;
	};

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::Bind(Ref<PropertyHandle<DataType>> aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
	}

	template<typename DataType>
	Slider<DataType>::Slider() noexcept
	{
		m_Format = String(ImGuiHelpers::GetTypeFormat<DataType>());
		m_FormatAndSuffix = m_Format;
		m_MinValue = std::numeric_limits<DataType>::lowest() / (DataType)2;
		m_MaxValue = std::numeric_limits<DataType>::max() / (DataType)2;

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5f, 12.5f, 255.0f);
		this->SetBackgroundColor(defaultFrameColor);
		this->SetHoverColor(defaultFrameColor);
		this->SetActiveColor(defaultFrameColor);
		this->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		this->SetHandleColor(Colors::Normalize(90.0f, 90.0f, 90.0f, 255.0f));

		this->SetHandleSize(20.0f);
		this->SetFrameRounding(6.0f);
		this->SetBorderSize(2.0f);
	}

	template<typename DataType>
	void Slider<DataType>::OnRender() noexcept
	{
		if (m_IsActive)
			this->SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (this->m_IsHovered)
			this->SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			this->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

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
			isUsing = ImGui::SliderScalar("##Slider", dataType, &value, &m_MinValue, &m_MaxValue, m_FormatAndSuffix.c_str(), this->GetFlags());

			if (this->m_IsHovered || m_IsActive)
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
			this->SetActive(true);
		else if (!isActive && m_IsActive)
			this->SetActive(false);
	}

	template<typename DataType>
	template<typename T>
	Slider<DataType>* Slider<DataType>::OnValueChanged(T&& aCallback)
	{
		m_OnValueChanged = Callback<void(DataType)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	Slider<DataType>* Slider<DataType>::OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType))
	{
		m_OnValueChanged = [aInstance, aMethod](DataType aValue) { return (aInstance->*aMethod)(aValue); };
		return this;
	}

	template<typename DataType>
	Vector2 Slider<DataType>::ReportSize() const noexcept
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

		if (fixedWidth && fixedHeight)
			return size;

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
	Slider<DataType>* Slider<DataType>::SetDelta(DataType aDelta) noexcept
	{
		m_Delta = aDelta;
		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetHandleColor(const Color& aColor) noexcept
	{
		this->m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetHandleSize(float aSize) noexcept
	{
		this->m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, aSize);
		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetLogarithmic(bool aState) noexcept
	{
		if (aState)
			this->AddFlags(ImGuiSliderFlags_Logarithmic);
		else
			this->RemoveFlags(ImGuiSliderFlags_Logarithmic);

		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetMaxValue(DataType aValue) noexcept
	{
		m_MaxValue = Math::Min(aValue, std::numeric_limits<DataType>::max() / (DataType)2);
		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetMinValue(DataType aValue) noexcept
	{
		m_MinValue = Math::Max(aValue, std::numeric_limits<DataType>::lowest() / (DataType)2);
		return this;
	}

	template<typename DataType>
	Slider<DataType>* Slider<DataType>::SetSuffix(StringView aSuffix) noexcept
	{
		m_Suffix = aSuffix;
		m_FormatAndSuffix = m_Format + m_Suffix;
		return this;
	}

	template<typename DataType>
	template<typename T>
	Slider<DataType>* Slider<DataType>::Value(T&& aCallback)
	{
		m_ValueCallback = Callback<DataType()>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	Slider<DataType>* Slider<DataType>::Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)())
	{
		m_ValueCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
		return this;
	}

	template<typename DataType>
	void Slider<DataType>::SetActive(bool aState) noexcept
	{
		m_IsActive = aState;
	}
}