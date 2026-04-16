#pragma once
#include "UI/Widgets/IBaseWidget.h"

namespace Relentless
{
	template<typename DataType> class DetailPropertyRowBuilder;
	template<typename DataType> class NumericEntryBoxEditorBuilder;
	template<typename DataType> class SliderEditorBuilder;
	template<typename DataType> class SpinBoxEditorBuilder;
	template<typename DataType> class AssetThumbnailEditorBuilder;
	template<typename DataType> class CheckBoxEditorBuilder;
	template<typename DataType> class ColorPickerEditorBuilder;
	template<typename DataType> class ComboBoxEditorBuilder;
	template<typename DataType> class TextBoxEditorBuilder;

	template<typename T>
	inline constexpr bool is_numeric_not_bool_v = std::is_arithmetic_v<std::remove_cv_t<T>> && !std::is_same_v<std::remove_cv_t<T>, bool>;

	template<typename T>
	inline constexpr bool is_vector_like_v = std::is_same_v<std::remove_cv_t<T>, Vector2> || std::is_same_v<std::remove_cv_t<T>, Vector3> || std::is_same_v<std::remove_cv_t<T>, Vector4>;

	enum class EEditorType : uint8 { AssetThumbnail = 0, CheckBox, ColorPicker, ComboBox, Label, NumericEntryBox, Slider, SpinBox, TextBox };

	struct PropertyDetails
	{
		std::vector<const char*> ComboBoxOptions;
		String Label;
		EEditorType EditorType = EEditorType::Slider;
		String Unit;
		String Tooltip;
		TVector2<double> Range;
		Vector2 FixedSize = Vector2::Zero;
		float Delta = 1.0f;
		int ComboBoxIndex = 0;
		bool Enabled = true;
		bool Logarithmic = false;
		bool ClampManualInput = false;
		bool ClampOnZeroRange = false;
		bool WrapAround = false;
		bool SteppingEnabled = false;
		bool ColorIndicator = false;

		ESizePolicy HorizontalSizePolicy = ESizePolicy::Auto;
		ESizePolicy VerticalSizePolicy = ESizePolicy::Auto;
		EHorizontalAlignmentPolicy HorizontalAlignmentPolicy = EHorizontalAlignmentPolicy::Left;
		EVerticalAlignmentPolicy VerticalAlignmentPolicy = EVerticalAlignmentPolicy::Center;
	};

	struct SlotContent
	{
		PropertyDetails Details;
		Callback<Ref<IBaseWidget>()> CustomWidgetCallback;
		Ref<IPropertyHandleBase> PropertyHandle = nullptr;
	};

	template<typename DataType>
	class SlotBuilder
	{
	public:
		SlotBuilder(DetailPropertyRowBuilder<DataType>& aParent, SlotContent& aSlotContent) noexcept
			:m_SlotContent(aSlotContent),
			m_Parent(aParent)
		{}

		CheckBoxEditorBuilder<DataType> CheckBox() noexcept;
		ComboBoxEditorBuilder<DataType> ComboBox() noexcept;

		DetailPropertyRowBuilder<DataType>& Row() noexcept
		{
			return m_Parent;
		}

		DetailPropertyRowBuilder<DataType>& Widget(Callback<Ref<IBaseWidget>()> aCallback) noexcept
		{
			m_SlotContent.CustomWidgetCallback = std::move(aCallback);
			return Row();
		}

	protected:
		template<typename D, typename T> friend class ArithmeticEditorBuilder;
		template<typename D, typename T> friend class EditorBuilder;
		template<typename T> friend class NumericEntryBoxEditorBuilder;
		template<typename T> friend class SliderEditorBuilder;
		template<typename T> friend class SpinBoxEditorBuilder;
		template<typename T> friend class CheckBoxEditorBuilder;
		template<typename T> friend class ColorPickerEditorBuilder;
		template<typename T> friend class ComboBoxEditorBuilder;
		template<typename T> friend class TextBoxEditorBuilder;

		void SetClampManualInput(bool aState) noexcept
		{
			m_SlotContent.Details.ClampManualInput = aState;
		}

		void SetClampOnZeroRange(bool aState) noexcept
		{
			m_SlotContent.Details.ClampOnZeroRange = aState;
		}

		void EnableColorIndicator(bool aState) noexcept
		{
			m_SlotContent.Details.ColorIndicator = aState;
		}

		void SetComboBoxOptions(const std::vector<const char*>& someOptions) noexcept
		{
			m_SlotContent.Details.ComboBoxOptions = someOptions;
		}

		void SetComboBoxIndex(int aIndex) noexcept
		{
			m_SlotContent.Details.ComboBoxIndex = aIndex;
		}

		void SetDelta(float aDelta) noexcept
		{
			m_SlotContent.Details.Delta = aDelta;
		}

		void SetEditorType(EEditorType aType) noexcept
		{
			m_SlotContent.Details.EditorType = aType;
		}

		void SetEnabled(bool aEnable) noexcept
		{
			m_SlotContent.Details.Enabled = aEnable;
		}

		void SetFixedHeight(float aHeight) noexcept
		{
			m_SlotContent.Details.FixedSize.y = aHeight;
		}

		void SetFixedWidth(float aWidth) noexcept
		{
			m_SlotContent.Details.FixedSize.x = aWidth;
		}

		void SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy aAlignmentPolicy) noexcept
		{
			m_SlotContent.Details.HorizontalAlignmentPolicy = aAlignmentPolicy;
		}

		void SetHorizontalSizePolicy(ESizePolicy aSizePolicy) noexcept
		{
			m_SlotContent.Details.HorizontalSizePolicy = aSizePolicy;
		}

		void SetLogarithmic(bool aState) noexcept
		{
			m_SlotContent.Details.Logarithmic = aState;
		}

		void SetPropertyHandle(Ref<IPropertyHandleBase> aPropertyHandle) noexcept
		{
			m_SlotContent.PropertyHandle = aPropertyHandle;
		}

		void SetRange(const TVector2<double>& aRange) noexcept
		{
			m_SlotContent.Details.Range = TVector2<double>(static_cast<double>(aRange.x), static_cast<double>(aRange.y));
		}

		void SetSteppingEnabled(bool aState) noexcept
		{
			m_SlotContent.Details.SteppingEnabled = aState;
		}

		void SetTooltip(StringView aTooltip) noexcept
		{
			m_SlotContent.Details.Tooltip = aTooltip;
		}

		void SetUnit(StringView aUnit) noexcept
		{
			m_SlotContent.Details.Unit = aUnit;
		}

		void SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy aAlignmentPolicy) noexcept
		{
			m_SlotContent.Details.VerticalAlignmentPolicy = aAlignmentPolicy;
		}

		void SetVerticalSizePolicy(ESizePolicy aSizePolicy) noexcept
		{
			m_SlotContent.Details.VerticalSizePolicy = aSizePolicy;
		}

		void SetWrapAround(bool aState) noexcept
		{
			m_SlotContent.Details.WrapAround = aState;
		}

	protected:
		SlotContent& m_SlotContent;
		DetailPropertyRowBuilder<DataType>& m_Parent;
	};

	template<typename DataType>
	class NameSlotBuilder : public SlotBuilder<DataType>
	{
	public:
		explicit NameSlotBuilder(DetailPropertyRowBuilder<DataType>& aParent, SlotContent& aSlotContent) noexcept
			: SlotBuilder<DataType>(aParent, aSlotContent)
		{}

		DetailPropertyRowBuilder<DataType>& Label(StringView aLabel) noexcept
		{
			this->m_SlotContent.Details.EditorType = EEditorType::Label;
			this->m_SlotContent.Details.Label = aLabel;
			return this->m_Parent;
		}
	};

	template<typename DataType>
	class ValueSlotBuilder : public SlotBuilder<DataType>
	{
	public:
		explicit ValueSlotBuilder(DetailPropertyRowBuilder<DataType>& aParent, SlotContent& aSlotContent) noexcept
			: SlotBuilder<DataType>(aParent, aSlotContent)
		{
			this->m_SlotContent.Details.HorizontalSizePolicy = ESizePolicy::Stretch;
		}

		AssetThumbnailEditorBuilder<DataType> AssetThumbnail() noexcept
			requires (std::is_same_v<std::remove_cv_t<DataType>, AssetData>);

		ColorPickerEditorBuilder<DataType> ColorPicker() noexcept
			requires (std::is_same_v<DataType, Color>);

		NumericEntryBoxEditorBuilder<DataType> NumericEntryBox() noexcept
			requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>);

		SliderEditorBuilder<DataType> Slider() noexcept
			requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>);

		SpinBoxEditorBuilder<DataType> SpinBox() noexcept
			requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>);

		TextBoxEditorBuilder<DataType> TextBox() noexcept
			requires (std::is_same_v<std::remove_cv_t<DataType>, String>);
	};
}