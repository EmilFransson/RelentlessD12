#pragma once
#include <Relentless.h>

#include "DetailLayoutBuilderHelpers.h"

#include "UI/Nodes/DetailNode.h"
#include "UI/Views/Details/TableRows/DetailPropertyRow.h"
#include "UI/Views/TreeView.h"

#include "UI/Widgets/CheckBox.h"
#include "UI/Widgets/ColorPicker.h"
#include "UI/Widgets/ComboBox.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/EditableTextBox.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/NumericEntryBox.h"
#include "UI/Widgets/Slider.h"
#include "UI/Widgets/SpinBox.h"

namespace Relentless
{
	template<typename DataType> class DetailPropertyRowBuilder;
	template<typename DataType> class NumericEntryBoxEditorBuilder;
	template<typename DataType> class SliderEditorBuilder;
	template<typename DataType> class SpinBoxEditorBuilder;
	template<typename DataType> class CheckBoxEditorBuilder;
	template<typename DataType> class ColorPickerEditorBuilder;
	template<typename DataType> class ComboBoxEditorBuilder;
	template<typename DataType> class TextBoxEditorBuilder;

	template<typename T>
	inline constexpr bool is_numeric_not_bool_v = std::is_arithmetic_v<std::remove_cv_t<T>> && !std::is_same_v<std::remove_cv_t<T>, bool>;

	template<typename T>
	inline constexpr bool is_vector_like_v = std::is_same_v<std::remove_cv_t<T>, Vector2> || std::is_same_v<std::remove_cv_t<T>, Vector3> || std::is_same_v<std::remove_cv_t<T>, Vector4>;

	enum class EEditorType : uint8 { CheckBox = 0u, ColorPicker, ComboBox, Label, NumericEntryBox, Slider, SpinBox, TextBox };

	struct PropertyDetails
	{
		std::vector<const char*> ComboBoxOptions;
		String Label;
		EEditorType EditorType = EEditorType::Slider;
		String Unit;
		String Tooltip;
		TVector2<double> Range;
		Vector2 FixedSize		= Vector2::Zero;
		float Delta				= 1.0f;
		int ComboBoxIndex		= 0;
		bool Logarithmic		= false;
		bool ClampManualInput	= false;
		bool ClampOnZeroRange	= false;
		bool WrapAround			= false;
		bool SteppingEnabled	= false;
		bool ColorIndicator		= false;

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
		{
		}

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
		{
		}

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

	template<typename DataType>
	class DetailPropertyRowBuilder
	{
	public:
		explicit DetailPropertyRowBuilder(const char* aLabel, DetailNode* aDetailNode) noexcept
			 : m_pDetailNode(aDetailNode)
		{
			m_NameSlot.Details.Label	= aLabel;
			m_ValueSlot.Details.Label	= aLabel;
			m_NameSlot.Details.Range	= TVector2<double>(std::numeric_limits<double>::lowest() / 2.0, std::numeric_limits<double>::max() / 2.0);
			m_ValueSlot.Details.Range	= TVector2<double>(std::numeric_limits<double>::lowest() / 2.0, std::numeric_limits<double>::max() / 2.0);
		}

		virtual ~DetailPropertyRowBuilder() noexcept
		{
			if (!m_HasBuilt)
				Build();
		}

		void Build() noexcept
		{
			m_pDetailNode->OnRequestRow([nameSlot = std::move(m_NameSlot), valueSlot = std::move(m_ValueSlot), revertSlot = std::move(m_RevertSlot), revertCallback = std::move(m_OnRevertCallback), revertedCallback = std::move(m_OnRevertedCallback), propertyBaseHandle = m_pDetailNode->GetPropertyHandle()](const ItemInfo& aItemInfo)
				{
					Ref<PropertyHandle<DataType>> pPropertyHandle = Ref<PropertyHandle<DataType>>(static_cast<PropertyHandle<DataType>*>(propertyBaseHandle.Get()));

					Ref<DetailPropertyRow> pDetailPropertyRow = RLS_NEW DetailPropertyRow();
					pDetailPropertyRow->SetIndentation(aItemInfo.Depth);

					//Name content:
					{
						if (nameSlot.CustomWidgetCallback.IsSet())
						{
							Ref<IBaseWidget> pCustomWidget = nameSlot.CustomWidgetCallback();

							if (pCustomWidget->IsContainer())
								pDetailPropertyRow->SetNameContent(std::move(pCustomWidget));
							else
							{
								Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
								pBox->AddWidget(std::move(pCustomWidget));
								pDetailPropertyRow->SetNameContent(std::move(pBox));
							}
						}
						else
						{
							Ref<HorizontalBox> pNameBox = RLS_NEW HorizontalBox();

							switch (nameSlot.Details.EditorType)
							{
							case EEditorType::Label:
							{
								pNameBox->AddWidget(RLS_NEW::Relentless::Label(nameSlot.Details.Label))
									->SetVerticalAlignmentPolicy(nameSlot.Details.VerticalAlignmentPolicy)
									->SetHorizontalAlignmentPolicy(nameSlot.Details.HorizontalAlignmentPolicy)
									->SetVerticalSizePolicy(nameSlot.Details.VerticalSizePolicy)
									->SetHorizontalSizePolicy(nameSlot.Details.HorizontalSizePolicy);

								break;
							}
							case EEditorType::CheckBox:
							{
								Ref<PropertyHandle<bool>> pPropertyHandleBool = Ref<PropertyHandle<bool>>(static_cast<PropertyHandle<bool>*>(nameSlot.PropertyHandle.Get()));

								pNameBox->AddWidget(RLS_NEW ::Relentless::CheckBox())
									->Bind(pPropertyHandleBool)
									->SetVerticalAlignmentPolicy(nameSlot.Details.VerticalAlignmentPolicy)
									->SetHorizontalAlignmentPolicy(nameSlot.Details.HorizontalAlignmentPolicy)
									->SetVerticalSizePolicy(nameSlot.Details.VerticalSizePolicy)
									->SetHorizontalSizePolicy(nameSlot.Details.HorizontalSizePolicy);

								pNameBox->AddWidget(RLS_NEW ::Relentless::Label(nameSlot.Details.Label))
									->SetVerticalAlignmentPolicy(nameSlot.Details.VerticalAlignmentPolicy)
									->SetHorizontalAlignmentPolicy(nameSlot.Details.HorizontalAlignmentPolicy)
									->SetVerticalSizePolicy(nameSlot.Details.VerticalSizePolicy)
									->SetHorizontalSizePolicy(nameSlot.Details.HorizontalSizePolicy);

								break;
							}
							case EEditorType::ComboBox:
							{
								Ref<PropertyHandle<int>> pPropertyHandleInt = Ref<PropertyHandle<int>>(static_cast<PropertyHandle<int>*>(nameSlot.PropertyHandle.Get()));

								pNameBox->AddWidget(RLS_NEW ::Relentless::ComboBox())
									->Bind(pPropertyHandleInt)
									->AddSelectables(nameSlot.Details.ComboBoxOptions)
									->SetSelectedItem(nameSlot.Details.ComboBoxIndex)
									->SetVerticalAlignmentPolicy(nameSlot.Details.VerticalAlignmentPolicy)
									->SetHorizontalAlignmentPolicy(nameSlot.Details.HorizontalAlignmentPolicy)
									->SetVerticalSizePolicy(nameSlot.Details.VerticalSizePolicy)
									->SetHorizontalSizePolicy(nameSlot.Details.HorizontalSizePolicy);

								break;
							}
							default:
								break;
							}

							pDetailPropertyRow->SetNameContent(pNameBox);
						}
					}

					//Value content:
					{
						if (valueSlot.CustomWidgetCallback.IsSet())
						{
							Ref<IBaseWidget> pCustomWidget = valueSlot.CustomWidgetCallback();

							if (pCustomWidget->IsContainer())
								pDetailPropertyRow->SetValueContent(std::move(pCustomWidget));
							else
							{
								Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
								pBox->AddWidget(std::move(pCustomWidget));
								pDetailPropertyRow->SetValueContent(std::move(pBox));
							}
						}
						else
						{
							Ref<HorizontalBox> pValueBox = RLS_NEW HorizontalBox();

							if constexpr (std::is_same_v<DataType, Vector2>)
								CreateVector2Editor(pValueBox, pPropertyHandle, valueSlot.Details);
							else if constexpr (std::is_same_v<DataType, Vector3>)
								CreateVector3Editor(pValueBox, pPropertyHandle, valueSlot.Details);
							else if constexpr (std::is_same_v<DataType, Vector4>)
								CreateVector4Editor(pValueBox, pPropertyHandle, valueSlot.Details);
							else
							{
								switch (valueSlot.Details.EditorType)
								{
								case EEditorType::Slider:
								{
									if constexpr (is_numeric_not_bool_v<DataType>)
										pValueBox->AddWidget(CreateSlider<DataType>(pPropertyHandle, valueSlot.Details));
									break;
								}
								case EEditorType::SpinBox:
								{
									if constexpr (is_numeric_not_bool_v<DataType>)
										pValueBox->AddWidget(CreateSpinBox<DataType>(pPropertyHandle, valueSlot.Details))
										->SetIndicatorColor(Colors::Red);
									break;
								}
								case EEditorType::NumericEntryBox:
								{
									if constexpr (is_numeric_not_bool_v<DataType>)
										pValueBox->AddWidget(CreateNumericEntryBox<DataType>(pPropertyHandle, valueSlot.Details));
									break;
								}
								case EEditorType::CheckBox:
								{
									if constexpr (std::is_same_v<DataType, bool>)
									{
										pValueBox->AddWidget(RLS_NEW::Relentless::CheckBox())
											->Bind(pPropertyHandle)
											->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
									}

									break;
								}
								case EEditorType::ColorPicker:
								{
									if constexpr (std::is_same_v<DataType, Color>)
									{
										pValueBox->AddWidget(RLS_NEW::Relentless::ColorPicker())
											->Bind(pPropertyHandle)
											->SetHorizontalSizePolicy(ESizePolicy::Stretch)
											->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
									}

									break;
								}
								case EEditorType::ComboBox:
								{
									if constexpr (std::is_same_v<DataType, int>)
									{
										pValueBox->AddWidget(RLS_NEW::Relentless::ComboBox())
											->Bind(pPropertyHandle)
											->AddSelectables(valueSlot.Details.ComboBoxOptions)
											->SetSelectedItem(valueSlot.Details.ComboBoxIndex)
											->SetHorizontalSizePolicy(ESizePolicy::Stretch)
											->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
									}

									break;
								}
								case EEditorType::TextBox:
								{
									if constexpr (std::is_same_v<DataType, String>)
									{
										pValueBox->AddWidget(RLS_NEW::Relentless::EditableTextBox())
											->Bind(pPropertyHandle)
											->SetHorizontalSizePolicy(ESizePolicy::Stretch)
											->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
									}
									break;
								}
								default:
									break;
								}
							}
						
							pDetailPropertyRow->SetValueContent(pValueBox);
						}
					}

					{
						if (revertSlot.CustomWidgetCallback.IsSet())
						{
							Ref<IBaseWidget> pCustomWidget = revertSlot.CustomWidgetCallback();

							if (pCustomWidget->IsContainer())
								pDetailPropertyRow->SetResetContent(std::move(pCustomWidget));
							else
							{
								Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
								pBox->AddWidget(std::move(pCustomWidget));
								pDetailPropertyRow->SetResetContent(std::move(pBox));
							}
						}
						else
						{
							Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();

							Button* pButton = pRevertBox->AddWidget(RLS_NEW Button(ICON_FA_ARROW_ROTATE_LEFT));
							pButton->SetBackgroundColor(Colors::Transparent);
							pButton->SetBorderColor(Colors::Transparent);
							pButton->SetHoverColor(Colors::Transparent);
							pButton->SetActiveColor(Colors::Transparent);
							pButton->SetTextColor(pPropertyHandle->DiffersFromDefault() ? Color(1.0f, 1.0f, 1.0f, 0.5f) : Colors::Transparent);
							pButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
							pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

							if (pPropertyHandle->CanResetToDefault())
							{
								pButton->OnMouseEnter([propertyHandle = pPropertyHandle.Get()](Button* aButton)
									{
										if (!propertyHandle->DiffersFromDefault())
											return;

										aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
									});

								pButton->OnMouseExit([propertyHandle = pPropertyHandle.Get()](Button* aButton)
									{
										if (!propertyHandle->DiffersFromDefault())
											return;

										aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
									});

								pButton->OnClicked([propertyHandle = pPropertyHandle.Get(), preCallback = std::move(revertCallback), postCallback = std::move(revertedCallback)]()
									{
										preCallback.ExecuteIfSet();
										propertyHandle->ResetToDefault();
										postCallback.ExecuteIfSet();
									});

								const CallbackID valueChangedCallbackID = pPropertyHandle->OnValueChanged.Connect([pButton, propertyHandle = pPropertyHandle.Get()](const DataType&)
									{
										const bool differs = propertyHandle->DiffersFromDefault();
										pButton->SetTextColor(differs ? (pButton->IsHovered() ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 0.5f)) : Colors::Transparent);
									});

								pDetailPropertyRow->OnDestroy.Connect([valueChangedCallbackID, propertyHandle = pPropertyHandle.Get()]()
									{
										propertyHandle->OnValueChanged.Detach(valueChangedCallbackID);
									});
							}

							pDetailPropertyRow->SetResetContent(pRevertBox);
						}
					}

					return pDetailPropertyRow;
				});

			m_HasBuilt = true;
		}

		NameSlotBuilder<DataType> NameSlot() noexcept
		{
			return NameSlotBuilder<DataType>(*this, m_NameSlot);
		}

		DetailPropertyRowBuilder<DataType>& OnRevertToDefault(Callback<void()> aCallback) noexcept
		{
			m_OnRevertCallback = std::move(aCallback);
			return *this;
		}
		DetailPropertyRowBuilder<DataType>& OnRevertedToDefault(Callback<void()> aCallback) noexcept
		{
			m_OnRevertedCallback = std::move(aCallback);
			return *this;
		}

		ValueSlotBuilder<DataType> ValueSlot() noexcept
		{
			return ValueSlotBuilder<DataType>(*this, m_ValueSlot);
		}

		ValueSlotBuilder<DataType> RevertSlot() noexcept
		{
			return ValueSlotBuilder<DataType>(*this, m_RevertSlot);
		}

	private:
		template<typename T>
		NO_DISCARD static Ref<::Relentless::NumericEntryBox<T>> CreateNumericEntryBox(Ref<PropertyHandle<T>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<::Relentless::NumericEntryBox<T>> pNumericEntryBox = RLS_NEW ::Relentless::NumericEntryBox<T>();
			pNumericEntryBox->Bind(aPropertyHandle);
			pNumericEntryBox->SetMinValue(static_cast<T>(aPropertyDetails.Range.x));
			pNumericEntryBox->SetMaxValue(static_cast<T>(aPropertyDetails.Range.y));
			pNumericEntryBox->SetDelta(static_cast<T>(aPropertyDetails.Delta));
			pNumericEntryBox->SetSteppingEnabled(aPropertyDetails.SteppingEnabled);
			pNumericEntryBox->SetSuffix(aPropertyDetails.Unit);
			pNumericEntryBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pNumericEntryBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

			return pNumericEntryBox;
		}

		template<typename T>
		NO_DISCARD static Ref<::Relentless::Slider<T>> CreateSlider(Ref<PropertyHandle<T>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<::Relentless::Slider<T>> pSlider = RLS_NEW ::Relentless::Slider<T>();
			pSlider->Bind(aPropertyHandle);
			pSlider->SetMinValue(static_cast<T>(aPropertyDetails.Range.x));
			pSlider->SetMaxValue(static_cast<T>(aPropertyDetails.Range.y));
			pSlider->SetDelta(static_cast<T>(aPropertyDetails.Delta));
			pSlider->SetSuffix(aPropertyDetails.Unit);
			pSlider->SetLogarithmic(aPropertyDetails.Logarithmic);
			pSlider->SetVerticalAlignmentPolicy(aPropertyDetails.VerticalAlignmentPolicy);
			pSlider->SetHorizontalAlignmentPolicy(aPropertyDetails.HorizontalAlignmentPolicy);
			pSlider->SetVerticalSizePolicy(aPropertyDetails.VerticalSizePolicy);
			pSlider->SetHorizontalSizePolicy(aPropertyDetails.HorizontalSizePolicy);
			
			return pSlider;
		}

		template<typename T>
		NO_DISCARD static Ref<::Relentless::SpinBox<T>> CreateSpinBox(Ref<PropertyHandle<T>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<::Relentless::SpinBox<T>> pSpinBox = RLS_NEW ::Relentless::SpinBox<T>();
			pSpinBox->Bind(aPropertyHandle);
			pSpinBox->SetMinValue(static_cast<T>(aPropertyDetails.Range.x));
			pSpinBox->SetMaxValue(static_cast<T>(aPropertyDetails.Range.y));
			pSpinBox->SetDelta(static_cast<T>(aPropertyDetails.Delta));
			pSpinBox->SetDrawColorIndicator(aPropertyDetails.ColorIndicator);
			pSpinBox->SetSuffix(aPropertyDetails.Unit);
			pSpinBox->SetWrapAround(aPropertyDetails.WrapAround);
			pSpinBox->SetVerticalAlignmentPolicy(aPropertyDetails.VerticalAlignmentPolicy);
			pSpinBox->SetHorizontalAlignmentPolicy(aPropertyDetails.HorizontalAlignmentPolicy);
			pSpinBox->SetVerticalSizePolicy(aPropertyDetails.VerticalSizePolicy);
			pSpinBox->SetHorizontalSizePolicy(aPropertyDetails.HorizontalSizePolicy);

			return pSpinBox;
		}

		static void CreateVector2Editor(Ref<HorizontalBox> aHorizontalBox, Ref<PropertyHandle<DataType>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<PropertyHandle<float>> pPropertyHandleX = DetailLayoutBuilderHelpers::MakeVector2ComponentHandle(aPropertyHandle, 0u);
			Ref<PropertyHandle<float>> pPropertyHandleY = DetailLayoutBuilderHelpers::MakeVector2ComponentHandle(aPropertyHandle, 1u);

			switch (aPropertyDetails.EditorType)
			{
			case EEditorType::Slider:
			{
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleY, aPropertyDetails));
				break;
			}
			case EEditorType::SpinBox:
			{
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleX, aPropertyDetails))
					->SetIndicatorColor(Colors::Red);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleY, aPropertyDetails))
					->SetIndicatorColor(Colors::Green);
				break;
			}
			case EEditorType::NumericEntryBox:
			{
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleY, aPropertyDetails));
				break;
			}
			default:
				break;
			}
		}

		static void CreateVector3Editor(Ref<HorizontalBox> aHorizontalBox, Ref<PropertyHandle<DataType>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<PropertyHandle<float>> pPropertyHandleX = DetailLayoutBuilderHelpers::MakeVector3ComponentHandle(aPropertyHandle, 0u);
			Ref<PropertyHandle<float>> pPropertyHandleY = DetailLayoutBuilderHelpers::MakeVector3ComponentHandle(aPropertyHandle, 1u);
			Ref<PropertyHandle<float>> pPropertyHandleZ = DetailLayoutBuilderHelpers::MakeVector3ComponentHandle(aPropertyHandle, 2u);

			switch (aPropertyDetails.EditorType)
			{
			case EEditorType::Slider:
			{
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleY, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleZ, aPropertyDetails));
				break;
			}
			case EEditorType::SpinBox:
			{
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleX, aPropertyDetails))
					->SetIndicatorColor(Colors::Red);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleY, aPropertyDetails))
					->SetIndicatorColor(Colors::Green);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleZ, aPropertyDetails))
					->SetIndicatorColor(Colors::Blue);
				break;
			}
			case EEditorType::NumericEntryBox:
			{
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleY, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleZ, aPropertyDetails));
				break;
			}
			default:
				break;
			}
		}

		static void CreateVector4Editor(Ref<HorizontalBox> aHorizontalBox, Ref<PropertyHandle<DataType>> aPropertyHandle, const PropertyDetails& aPropertyDetails) noexcept
		{
			Ref<PropertyHandle<float>> pPropertyHandleX = DetailLayoutBuilderHelpers::MakeVector4ComponentHandle(aPropertyHandle, 0u);
			Ref<PropertyHandle<float>> pPropertyHandleY = DetailLayoutBuilderHelpers::MakeVector4ComponentHandle(aPropertyHandle, 1u);
			Ref<PropertyHandle<float>> pPropertyHandleZ = DetailLayoutBuilderHelpers::MakeVector4ComponentHandle(aPropertyHandle, 2u);
			Ref<PropertyHandle<float>> pPropertyHandleW = DetailLayoutBuilderHelpers::MakeVector4ComponentHandle(aPropertyHandle, 3u);

			switch (aPropertyDetails.EditorType)
			{
			case EEditorType::Slider:
			{
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleY, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleZ, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateSlider<float>(pPropertyHandleW, aPropertyDetails));
				break;
			}
			case EEditorType::SpinBox:
			{
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleX, aPropertyDetails))
					->SetIndicatorColor(Colors::Red);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleY, aPropertyDetails))
					->SetIndicatorColor(Colors::Green);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleZ, aPropertyDetails))
					->SetIndicatorColor(Colors::Blue);
				aHorizontalBox->AddWidget(CreateSpinBox<float>(pPropertyHandleW, aPropertyDetails))
					->SetIndicatorColor(Colors::Yellow);
				break;
			}
			case EEditorType::NumericEntryBox:
			{
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleX, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleY, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleZ, aPropertyDetails));
				aHorizontalBox->AddWidget(CreateNumericEntryBox<float>(pPropertyHandleW, aPropertyDetails));
				break;
			}
			default:
				break;
			}
		}

	private:
		SlotContent m_NameSlot;
		SlotContent m_ValueSlot;
		SlotContent m_RevertSlot;
		Callback<void()> m_OnRevertCallback;
		Callback<void()> m_OnRevertedCallback;

		DetailNode* m_pDetailNode = nullptr;
		bool m_HasBuilt = false;
	};

	template<typename Derived, typename DataType>
	class EditorBuilder : public RefCounted<EditorBuilder<Derived, DataType>>
	{
	public:
		explicit EditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: m_ParentBuilder(aParentBuilder)
		{
		}

		virtual ~EditorBuilder() noexcept = default;

		void Build() noexcept;

		Derived& Bind(Ref<IPropertyHandleBase> aPropertyHandle) noexcept
		{
			m_ParentBuilder.SetPropertyHandle(aPropertyHandle);
			return *static_cast<Derived*>(this);
		}

		Derived& FixedHeight(float aHeight) noexcept
		{
			m_ParentBuilder.SetVerticalSizePolicy(ESizePolicy::Fixed);
			m_ParentBuilder.SetFixedHeight(aHeight);
			return *static_cast<Derived*>(this);
		}

		Derived& FixedWidth(float aWidth) noexcept
		{
			m_ParentBuilder.SetHorizontalSizePolicy(ESizePolicy::Fixed);
			m_ParentBuilder.SetFixedWidth(aWidth);
			return *static_cast<Derived*>(this);
		}

		Derived& StretchHeight() noexcept
		{
			m_ParentBuilder.SetVerticalSizePolicy(ESizePolicy::Stretch);
			return *static_cast<Derived*>(this);
		}

		Derived& StretchWidth() noexcept
		{
			m_ParentBuilder.SetHorizontalSizePolicy(ESizePolicy::Stretch);
			return *static_cast<Derived*>(this);
		}

		Derived& Tooltip(StringView aTooltip) noexcept
		{
			m_ParentBuilder.SetTooltip(aTooltip);
			return *static_cast<Derived*>(this);
		}

		Derived& Unit(StringView aUnit) noexcept
		{
			m_ParentBuilder.SetUnit(aUnit);
			return *static_cast<Derived*>(this);
		}

		DetailPropertyRowBuilder<DataType>& Row() noexcept
		{
			return m_ParentBuilder.Row();
		}
	protected:
		SlotBuilder<DataType>& m_ParentBuilder;
	};

	template<typename Derived, typename DataType>
	void EditorBuilder<Derived, DataType>::Build() noexcept
	{
		m_ParentBuilder.Build();
	}

	//Specializers:

	template<typename DataType>
	class CheckBoxEditorBuilder : public EditorBuilder<CheckBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit CheckBoxEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<CheckBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}
	};

	template<typename DataType>
	class ColorPickerEditorBuilder : public EditorBuilder<ColorPickerEditorBuilder<DataType>, DataType>
	{
	public:
		explicit ColorPickerEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<ColorPickerEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}
	};

	template<typename DataType>
	class ComboBoxEditorBuilder : public EditorBuilder<ComboBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit ComboBoxEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<ComboBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}

		ComboBoxEditorBuilder<DataType>& Options(Span<const char*> someOptions) noexcept
		{
			this->m_ParentBuilder.SetComboBoxOptions(someOptions.Copy());
			return *this;
		}

		ComboBoxEditorBuilder<DataType>& Selected(int aIndex) noexcept
		{
			this->m_ParentBuilder.SetComboBoxIndex(aIndex);
			return *this;
		}
	};

	template<typename DataType>
	class TextBoxEditorBuilder : public EditorBuilder<TextBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit TextBoxEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<TextBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}
	};

	template<typename DataType>
	class NumericEntryBoxEditorBuilder : public EditorBuilder<NumericEntryBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit NumericEntryBoxEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<NumericEntryBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}

		NumericEntryBoxEditorBuilder<DataType>& EnableStepping(bool aState) noexcept
		{
			this->m_ParentBuilder.SetSteppingEnabled(aState);
			return *this;
		}

		template<typename T>
		NumericEntryBoxEditorBuilder<DataType>& Range(T aMin, T aMax) noexcept
		{
			this->m_ParentBuilder.SetRange(TVector2<double>(static_cast<double>(aMin), static_cast<double>(aMax)));
			return *this;
		}
	};

	template<typename Derived, typename DataType>
	class ArithmeticEditorBuilder : public EditorBuilder<ArithmeticEditorBuilder<Derived, DataType>, DataType>
	{
	public:
		explicit ArithmeticEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<ArithmeticEditorBuilder<Derived, DataType>, DataType>(aParentBuilder)
		{
		}

		virtual ~ArithmeticEditorBuilder() noexcept = default;

		Derived& ClampAll() noexcept
		{
			this->m_ParentBuilder.SetClampManualInput(true);
			this->m_ParentBuilder.SetClampOnZeroRange(true);
			return static_cast<Derived&>(*this);
		}

		Derived& ClampManualInput() noexcept
		{
			this->m_ParentBuilder.SetClampManualInput(true);
			return static_cast<Derived&>(*this);
		}

		Derived& ClampOnZeroRange() noexcept
		{
			this->m_ParentBuilder.SetClampOnZeroRange(true);
			return static_cast<Derived&>(*this);
		}

		template<typename T>
		Derived& Range(T aMin, T aMax) noexcept
		{
			this->m_ParentBuilder.SetRange(TVector2<double>(static_cast<double>(aMin), static_cast<double>(aMax)));
			return static_cast<Derived&>(*this);
		}

		Derived& Logarithmic(bool aState) noexcept
		{
			this->m_ParentBuilder.SetLogarithmic(aState);
			return static_cast<Derived&>(*this);
		}
	};

	template<typename DataType>
	class SliderEditorBuilder : public ArithmeticEditorBuilder<SliderEditorBuilder<DataType>, DataType>
	{
	public:
		explicit SliderEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: ArithmeticEditorBuilder<SliderEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}
	};

	template<typename DataType>
	class SpinBoxEditorBuilder : public ArithmeticEditorBuilder<SpinBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit SpinBoxEditorBuilder(SlotBuilder<DataType>& aParentBuilder) noexcept
			: ArithmeticEditorBuilder<SpinBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}

		SpinBoxEditorBuilder<DataType>& Delta(float aDelta) noexcept
		{
			this->m_ParentBuilder.SetDelta(aDelta);
			return *this;
		}

		SpinBoxEditorBuilder<DataType>& WrapAround(bool aState) noexcept
		{
			this->m_ParentBuilder.SetWrapAround(aState);
			return *this;
		}

		SpinBoxEditorBuilder<DataType>& EnableColorIndicator(bool aState) noexcept
		{
			this->m_ParentBuilder.EnableColorIndicator(aState);
			return *this;
		}
	};

	template<typename DataType>
	CheckBoxEditorBuilder<DataType> SlotBuilder<DataType>::CheckBox() noexcept
	{
		this->SetEditorType(EEditorType::CheckBox);
		return CheckBoxEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	ColorPickerEditorBuilder<DataType> ValueSlotBuilder<DataType>::ColorPicker() noexcept
		requires (std::is_same_v<DataType, Color>)
	{
		this->SetEditorType(EEditorType::ColorPicker);
		return ColorPickerEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	ComboBoxEditorBuilder<DataType> SlotBuilder<DataType>::ComboBox() noexcept
	{
		this->SetEditorType(EEditorType::ComboBox);
		return ComboBoxEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	NumericEntryBoxEditorBuilder<DataType> ValueSlotBuilder<DataType>::NumericEntryBox() noexcept
		requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>)
	{
		this->SetEditorType(EEditorType::NumericEntryBox);
		return NumericEntryBoxEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	SliderEditorBuilder<DataType> ValueSlotBuilder<DataType>::Slider() noexcept
		requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>)
	{
		this->SetEditorType(EEditorType::Slider);
		return SliderEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	SpinBoxEditorBuilder<DataType> ValueSlotBuilder<DataType>::SpinBox() noexcept
		requires (is_numeric_not_bool_v<DataType> || is_vector_like_v<DataType>)
	{
		this->SetEditorType(EEditorType::SpinBox);
		return SpinBoxEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	TextBoxEditorBuilder<DataType> ValueSlotBuilder<DataType>::TextBox() noexcept
		requires (std::is_same_v<std::remove_cv_t<DataType>, String>)
	{
		this->SetEditorType(EEditorType::TextBox);
		return TextBoxEditorBuilder<DataType>(*this);
	}
}