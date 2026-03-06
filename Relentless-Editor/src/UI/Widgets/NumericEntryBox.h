#include "IWidget.h"
#include "UI/Meta/ImGuiHelpers.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	template<typename DataType>
	class NumericEntryBox : public IStylableWidget<NumericEntryBox<DataType>>
	{
	public:
		NumericEntryBox() noexcept;
		virtual ~NumericEntryBox() noexcept = default;

		NumericEntryBox* Bind(Ref<PropertyHandle<DataType>> aPropertyHandle) noexcept;

		template<typename T>
		NumericEntryBox* OnValueChanged(T&& aCallback);

		template<typename InstanceType>
		NumericEntryBox* OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType));

		template<typename T>
		NumericEntryBox* OnValueCommitted(T&& aCallback);

		template<typename InstanceType>
		NumericEntryBox* OnValueCommitted(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType, ETextCommitType));

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		NumericEntryBox* SetButtonActiveColor(const Color& aColor) noexcept;
		NumericEntryBox* SetButtonBackgroundColor(const Color& aColor) noexcept;
		NumericEntryBox* SetButtonHoverColor(const Color& aColor) noexcept;
		NumericEntryBox* SetDelta(DataType aDelta) noexcept;
		NumericEntryBox* SetMaxValue(DataType aValue) noexcept;
		NumericEntryBox* SetMinValue(DataType aValue) noexcept;
		NumericEntryBox* SetSteppingEnabled(bool aState) noexcept;
		NumericEntryBox* SetSuffix(StringView aSuffix) noexcept;
		NumericEntryBox* SetDisplayText(StringView aSuffix) noexcept;

		template<typename T>
		NumericEntryBox* Value(T&& aCallback);

		template<typename InstanceType>
		NumericEntryBox* Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)());

	private:
		void OnRender() noexcept override;
		
		void SetActive(bool aState) noexcept;
	private:
		String m_Format;
		String m_DisplayText;
		String m_Suffix;
		Callback<DataType()> m_ValueCallback;
		Callback<void(DataType)> m_OnValueChanged;
		Callback<void(DataType, ETextCommitType)> m_OnValueCommitted;

		DataType m_Delta = (DataType)1;
		DataType m_MinValue = (DataType)0;
		DataType m_MaxValue = (DataType)0;

		bool m_IsActive = false;
		bool m_StepButtonsEnabled = true;

		Ref<PropertyHandle<DataType>> m_pPropertyHandle = nullptr;
	};

	template<typename DataType>
	NumericEntryBox<DataType>::NumericEntryBox() noexcept
	{
		m_Format = String(ImGuiHelpers::GetTypeFormat<DataType>());
		m_DisplayText = m_Format;
		m_MinValue = std::numeric_limits<DataType>::lowest() / (DataType)2;
		m_MaxValue = std::numeric_limits<DataType>::max() / (DataType)2;

		this->AddFlags(ImGuiInputTextFlags_CharsNoBlank);
		this->AddFlags(ImGuiInputTextFlags_CharsScientific);

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5, 12.5f, 255.0f);
		this->SetBackgroundColor(defaultFrameColor);
		this->SetHoverColor(defaultFrameColor);
		this->SetActiveColor(defaultFrameColor);
		this->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		SetButtonBackgroundColor(defaultFrameColor);

		this->SetFrameRounding(6.0f);
		this->SetBorderSize(2.0f);
		this->SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::Bind(Ref<PropertyHandle<DataType>> aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
	}

	template<typename DataType>
	Vector2 NumericEntryBox<DataType>::ReportSize() const noexcept
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
	void NumericEntryBox<DataType>::SetActive(bool aState) noexcept
	{
		m_IsActive = aState;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetButtonActiveColor(const Color& aColor) noexcept
	{
		this->m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetButtonBackgroundColor(const Color& aColor) noexcept
	{
		this->m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetButtonHoverColor(const Color& aColor) noexcept
	{
		this->m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetDelta(DataType aDelta) noexcept
	{
		m_Delta = aDelta;
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetDisplayText(StringView aDisplayText) noexcept
	{
		m_DisplayText = String(aDisplayText);
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetMaxValue(DataType aValue) noexcept
	{
		m_MaxValue = Math::Min(aValue, std::numeric_limits<DataType>::max() / (DataType)2);
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetMinValue(DataType aValue) noexcept
	{
		m_MinValue = Math::Max(aValue, std::numeric_limits<DataType>::lowest() / (DataType)2);
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetSteppingEnabled(bool aState) noexcept
	{
		m_StepButtonsEnabled = aState;
		return this;
	}

	template<typename DataType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::SetSuffix(StringView aSuffix) noexcept
	{
		m_Suffix = aSuffix;
		m_DisplayText = m_Format + m_Suffix;
		return this;
	}

	template<typename DataType>
	template<typename T>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::OnValueChanged(T&& aCallback)
	{
		m_OnValueChanged = Callback<void(DataType)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType))
	{
		m_OnValueChanged = [aInstance, aMethod](DataType aValue) { return (aInstance->*aMethod)(aValue); };
		return this;
	}

	template<typename DataType>
	template<typename T>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::OnValueCommitted(T&& aCallback)
	{
		m_OnValueCommitted = Callback<void(DataType, ETextCommitType)>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::OnValueCommitted(InstanceType* aInstance, void(InstanceType::*aMethod)(DataType, ETextCommitType))
	{
		m_OnValueCommitted = [aInstance, aMethod](DataType aValue, ETextCommitType aCommitType) { return (aInstance->*aMethod)(aValue, aCommitType); };
		return this;
	}

	template<typename DataType>
	template<typename T>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::Value(T&& aCallback)
	{
		m_ValueCallback = Callback<DataType()>(std::forward<T>(aCallback));
		return this;
	}

	template<typename DataType>
	template<typename InstanceType>
	NumericEntryBox<DataType>* NumericEntryBox<DataType>::Value(InstanceType* aInstance, DataType(InstanceType::*aMethod)())
	{
		m_ValueCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
		return this;
	}

	template<typename DataType>
	void NumericEntryBox<DataType>::OnRender() noexcept
	{
		DataType value = (DataType)0;
		EPropertyAccessResult accessResult = EPropertyAccessResult::Fail;

		if (m_pPropertyHandle)
			accessResult = m_pPropertyHandle->GetValue(value);
		if (accessResult == EPropertyAccessResult::Fail && m_ValueCallback.IsSet())
			value = m_ValueCallback();

		constexpr ImGuiDataType dataType = ImGuiHelpers::ToImGuiDataType<DataType>();

		if (ImGui::InputScalar("##NumericEntryBox", dataType, &value, m_StepButtonsEnabled ? &m_Delta : nullptr, nullptr, accessResult == EPropertyAccessResult::MixedValues ? "Mixed" : m_DisplayText.c_str(), this->GetFlags()))
		{
			if (value >= m_MinValue && value <= m_MaxValue)
			{
				if (m_pPropertyHandle)
					m_pPropertyHandle->SetValue(value);
				else
					m_OnValueChanged.ExecuteIfSet(value);
			}
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (value >= m_MinValue && value <= m_MaxValue)
			{
				if (m_pPropertyHandle)
					m_pPropertyHandle->SetValue(value);
				else
				{
					const ETextCommitType type = Keyboard::IsKeyPressed(RLS_Key::Enter) ? ETextCommitType::OnEnter : ETextCommitType::OnUserMovedFocus;
					m_OnValueCommitted.ExecuteIfSet(value, type);
				}
			}
		}

		this->m_IsHovered = ImGui::IsItemHovered();
		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			this->SetActive(true);
		else if (!isActive && m_IsActive)
			this->SetActive(false);
	}
}