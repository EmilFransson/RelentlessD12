#pragma once
#include <Relentless.h>

#include "UI/Views/Details/DetailNode.h"
#include "UI/Views/Details/TableRows/DetailPropertyRow.h"
#include "UI/Views/TreeView.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBoxEx.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/NumericEntryBox.h"
#include "UI/Widgets/Slider.h"
#include "UI/Widgets/SpinBox.h"

namespace Relentless
{
	template<typename DataType>
	class DetailPropertyRowBuilder;

	template<typename DataType>
	class NumericEntryBoxEditorBuilder;

	template<typename DataType>
	class SliderEditorBuilder;

	template<typename DataType>
	class SpinBoxEditorBuilder;

	enum class EEditorType : uint8 { Slider = 0u, SpinBox, NumericEntryBox };

	template<typename DataType>
	struct PropertyDetails
	{
		String Label;
		EEditorType EditorType = EEditorType::Slider;
		String Unit;
		String Tooltip;
		TVector2<DataType> Range;
		DataType Delta;
		bool Logarithmic		= false;
		bool ClampManualInput	= false;
		bool ClampOnZeroRange	= false;
		bool WrapAround			= false;
		bool SteppingEnabled	= false;
	};

	template<typename DataType>
	class EditorBuilder : public RefCounted<EditorBuilder<DataType>>
	{
	public:
		explicit EditorBuilder(DetailPropertyRowBuilder<DataType>& aParentBuilder) noexcept
			: m_ParentBuilder(aParentBuilder)
		{
		}

		virtual ~EditorBuilder() noexcept = default;

		void Build() noexcept;

		DetailPropertyRowBuilder<DataType>& Row() noexcept
		{
			return m_ParentBuilder;
		}
	protected:
		DetailPropertyRowBuilder<DataType>& m_ParentBuilder;
	};

	template<typename DataType>
	class DetailPropertyRowBuilder
	{
	public:
		explicit DetailPropertyRowBuilder(const char* aLabel, DetailNode* aDetailNode) noexcept
			 : m_pDetailNode(aDetailNode)
		{
			m_PropertyDetails.Label = aLabel;
			m_PropertyDetails.Range = TVector2<DataType>(std::numeric_limits<DataType>::lowest() / (DataType)2, std::numeric_limits<DataType>::max() / (DataType)2);
			m_PropertyDetails.Delta = (DataType)1;
		}

		virtual ~DetailPropertyRowBuilder() noexcept
		{
			if (!m_HasBuilt)
				Build();
		}

		void Build() noexcept
		{
			m_pDetailNode->OnRequestRow([propertyDetails = m_PropertyDetails, propertyBaseHandle = m_pDetailNode->GetPropertyHandle().Get()](const ItemInfo& aItemInfo)
				{
					PropertyHandle<DataType>* pPropertyhandle = static_cast<PropertyHandle<DataType>*>(propertyBaseHandle);

					Ref<DetailPropertyRow2> pDetailPropertyRow = RLS_NEW DetailPropertyRow2();
					pDetailPropertyRow->SetIndentation(aItemInfo.Depth);

					//Name content:
					{
						Ref<HorizontalBoxEx> pNameBox = RLS_NEW HorizontalBoxEx();

						pNameBox->AddWidget(RLS_NEW Label(propertyDetails.Label))
							->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

						pDetailPropertyRow->SetNameContent(pNameBox);
					}

					//Value content:
					{
						Ref<HorizontalBoxEx> pValueBox = RLS_NEW HorizontalBoxEx();

						switch (propertyDetails.EditorType)
						{
						case EEditorType::Slider:
						{
							pValueBox->AddWidget(RLS_NEW ::Relentless::Slider<DataType>())
								->Bind(pPropertyhandle)
								->SetMinValue(propertyDetails.Range.x)
								->SetMaxValue(propertyDetails.Range.y)
								->SetDelta(propertyDetails.Delta)
								->SetSuffix(propertyDetails.Unit)
								->SetLogarithmic(propertyDetails.Logarithmic)
								->SetHorizontalSizePolicy(ESizePolicy::Stretch)
								->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
							break;
						}
						case EEditorType::SpinBox:
						{
							pValueBox->AddWidget(RLS_NEW ::Relentless::SpinBox<DataType>())
								->Bind(pPropertyhandle)
								->SetMinValue(propertyDetails.Range.x)
								->SetMaxValue(propertyDetails.Range.y)
								->SetDelta(propertyDetails.Delta)
								->SetSuffix(propertyDetails.Unit)
								->SetWrapAround(propertyDetails.WrapAround)
								->SetHorizontalSizePolicy(ESizePolicy::Stretch)
								->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
							break;
						}
						case EEditorType::NumericEntryBox:
						{
							pValueBox->AddWidget(RLS_NEW ::Relentless::NumericEntryBox<DataType>())
								->Bind(pPropertyhandle)
								->SetMinValue(propertyDetails.Range.x)
								->SetMaxValue(propertyDetails.Range.y)
								->SetDelta(propertyDetails.Delta)
								->SetSteppingEnabled(propertyDetails.SteppingEnabled)
								->SetSuffix(propertyDetails.Unit)
								->SetHorizontalSizePolicy(ESizePolicy::Stretch)
								->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
							break;
						}
						default:
							break;
						}

						pDetailPropertyRow->SetValueContent(pValueBox);
					}

					{
						Ref<HorizontalBoxEx> pRevertBox = RLS_NEW HorizontalBoxEx();

						Button* pButton = pRevertBox->AddWidget(RLS_NEW Button(ICON_FA_ARROW_ROTATE_LEFT));
						pButton->SetBackgroundColor(Colors::Transparent);
						pButton->SetBorderColor(Colors::Transparent);
						pButton->SetHoverColor(Colors::Transparent);
						pButton->SetActiveColor(Colors::Transparent);
						pButton->SetTextColor(pPropertyhandle->DiffersFromDefault() ? Color(1.0f, 1.0f, 1.0f, 0.5f) : Colors::Transparent);
						pButton->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
						pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

						if (pPropertyhandle->CanResetToDefault())
						{
							pButton->OnMouseEnter([pPropertyhandle](Button* aButton)
								{
									if (!pPropertyhandle->DiffersFromDefault())
										return;

									aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
								});

							pButton->OnMouseExit([pPropertyhandle](Button* aButton)
								{
									if (!pPropertyhandle->DiffersFromDefault())
										return;

									aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
								});

							pButton->OnClicked([pPropertyhandle]() { pPropertyhandle->ResetToDefault(); });

							const CallbackID valueChangedCallbackID = pPropertyhandle->OnValueChanged.Connect([pButton, pPropertyhandle](const DataType&)
								{
									const bool differs = pPropertyhandle->DiffersFromDefault();
									pButton->SetTextColor(differs ? (pButton->IsHovered() ? Color(1.0f, 1.0f, 1.0f, 1.0f) : Color(1.0f, 1.0f, 1.0f, 0.5f)) : Colors::Transparent);
								});

							pDetailPropertyRow->OnDestroy.Connect([valueChangedCallbackID, pPropertyhandle]()
								{
									pPropertyhandle->OnValueChanged.Detach(valueChangedCallbackID);
								});
						}

						pDetailPropertyRow->SetResetContent(pRevertBox);
					}

					return pDetailPropertyRow;
				});

			m_HasBuilt = true;
		}

		NumericEntryBoxEditorBuilder<DataType> NumericEntryBox() noexcept;

		SliderEditorBuilder<DataType> Slider() noexcept;
		SpinBoxEditorBuilder<DataType> SpinBox() noexcept;

		DetailPropertyRowBuilder<DataType>& Tooltip(StringView aTooltip) noexcept
		{
			m_PropertyDetails.Tooltip = aTooltip;
			return *this;
		}

		DetailPropertyRowBuilder<DataType>& Unit(StringView aUnit) noexcept
		{
			m_PropertyDetails.Unit = aUnit;
			return *this;
		}

	private:
		template<typename Derived, typename T>
		friend class ArithmeticEditorBuilder;

		template<typename T>
		friend class NumericEntryBoxEditorBuilder;

		template<typename T>
		friend class SliderEditorBuilder;

		template<typename T>
		friend class SpinBoxEditorBuilder;

		void SetClampManualInput(bool aState) noexcept
		{
			m_PropertyDetails.ClampManualInput = aState;
		}

		void SetClampOnZeroRange(bool aState) noexcept
		{
			m_PropertyDetails.ClampOnZeroRange = aState;
		}

		void SetEditorType(EEditorType aType) noexcept
		{
			m_PropertyDetails.EditorType = aType;
		}

		void SetLogarithmic(bool aState) noexcept
		{
			m_PropertyDetails.Logarithmic = aState;
		}

		void SetRange(const TVector2<DataType>& aRange) noexcept
		{
			m_PropertyDetails.Range = aRange;
		}

		void SetSteppingEnabled(bool aState) noexcept
		{
			m_PropertyDetails.SteppingEnabled = aState;
		}

		void SetWrapAround(bool aState) noexcept
		{
			m_PropertyDetails.WrapAround = aState;
		}
		
	private:
		DetailNode* m_pDetailNode = nullptr;
		PropertyDetails<DataType> m_PropertyDetails;
		bool m_HasBuilt = false;
	};


	template<typename DataType>
	void EditorBuilder<DataType>::Build() noexcept
	{
		m_ParentBuilder.Build();
	}

	template<typename Derived, typename DataType>
	class ArithmeticEditorBuilder : public EditorBuilder<DataType>
	{
	public:
		explicit ArithmeticEditorBuilder(DetailPropertyRowBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<DataType>(aParentBuilder)
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

		Derived& Range(DataType aMin, DataType aMax) noexcept
		{
			this->m_ParentBuilder.SetRange(TVector2<DataType>(aMin, aMax));
			return static_cast<Derived&>(*this);
		}

		Derived& Logarithmic() noexcept
		{
			this->m_ParentBuilder.SetLogarithmic(true);
			return static_cast<Derived&>(*this);
		}
	};

	template<typename DataType>
	class NumericEntryBoxEditorBuilder : public EditorBuilder<DataType>
	{
	public:
		explicit NumericEntryBoxEditorBuilder(DetailPropertyRowBuilder<DataType>& aParentBuilder) noexcept
			: EditorBuilder<DataType>(aParentBuilder)
		{
		}

		NumericEntryBoxEditorBuilder<DataType>& EnableStepping() noexcept
		{
			this->m_ParentBuilder.SetSteppingEnabled(true);
			return *this;
		}
	};

	template<typename DataType>
	class SliderEditorBuilder : public ArithmeticEditorBuilder<SliderEditorBuilder<DataType>, DataType>
	{
	public:
		explicit SliderEditorBuilder(DetailPropertyRowBuilder<DataType>& aParentBuilder) noexcept
			: ArithmeticEditorBuilder<SliderEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}
	};

	template<typename DataType>
	class SpinBoxEditorBuilder : public ArithmeticEditorBuilder<SpinBoxEditorBuilder<DataType>, DataType>
	{
	public:
		explicit SpinBoxEditorBuilder(DetailPropertyRowBuilder<DataType>& aParentBuilder) noexcept
			: ArithmeticEditorBuilder<SpinBoxEditorBuilder<DataType>, DataType>(aParentBuilder)
		{
		}

		SpinBoxEditorBuilder<DataType>& WrapAround() noexcept
		{
			this->m_ParentBuilder.SetWrapAround(true);
			return *this;
		}
	};

	//Specializers:
	
	template<typename DataType>
	NumericEntryBoxEditorBuilder<DataType> DetailPropertyRowBuilder<DataType>::NumericEntryBox() noexcept
	{
		SetEditorType(EEditorType::NumericEntryBox);
		return NumericEntryBoxEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	SliderEditorBuilder<DataType> DetailPropertyRowBuilder<DataType>::Slider() noexcept
	{
		SetEditorType(EEditorType::Slider);
		return SliderEditorBuilder<DataType>(*this);
	}

	template<typename DataType>
	SpinBoxEditorBuilder<DataType> DetailPropertyRowBuilder<DataType>::SpinBox() noexcept
	{
		SetEditorType(EEditorType::SpinBox);
		return SpinBoxEditorBuilder<DataType>(*this);
	}
}