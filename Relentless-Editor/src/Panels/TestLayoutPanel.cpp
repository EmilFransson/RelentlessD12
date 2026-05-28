#include "TestLayoutPanel.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/ColorPicker.h"
#include "UI/Widgets/ComboBox.h"
#include "UI/Widgets/CheckBox.h"
#include "UI/Widgets/EditableTextBox.h"
#include "UI/Widgets/FloatEntryBox.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/NumericEntryBox.h"
#include "UI/Widgets/SearchBar.h"
#include "UI/Widgets/Slider.h"
#include "UI/Widgets/Spacer.h"
#include "UI/Widgets/SpinBox.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	TestLayoutPanel::TestLayoutPanel() noexcept
		:PanelBase("Test Layout")
	{
		SetPadding(Vector2(5.0f, 5.0f));
		Ref<VerticalBox> pRoot = new VerticalBox();
		pRoot->SetScrollBarsVisible(false);
		pRoot->SetMouseScrollingEnabled(false);

		HorizontalBox* pTopBox = pRoot->AddWidget(new HorizontalBox());
		pTopBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pTopBox->SetScrollBarsVisible(false);
		pTopBox->SetMouseScrollingEnabled(false);

		//Top Left:
		{
			HorizontalBox* pTopLeftBox = pTopBox->AddWidget(new HorizontalBox());
			pTopLeftBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pTopLeftBox->SetSpacing(5.0f);
			pTopLeftBox->SetMargin(FloatRect(3.0f, 3.0f, 3.0f, 3.0f));

			CheckBox* pCheckBox = pTopLeftBox->AddWidget(new CheckBox());
			pCheckBox->SetPadding(Vector2(20.0f, 20.0f));

			SearchBar* pSearchBar = pTopLeftBox->AddWidget(new SearchBar("Searching...", true));
			pSearchBar->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pSearchBar->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Bottom);

			ComboBox* pComboBox = pTopLeftBox->AddWidget(new ComboBox());
			pComboBox->AddSelectables({ "Robin", "Emil", "Jacke", "Kappe" });
			pComboBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		}
		//Top Right:
		{
			HorizontalBox* pTopRightBox = pTopBox->AddWidget(new HorizontalBox());
			pTopRightBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pTopRightBox->SetMargin(FloatRect(3.0f, 3.0f, 3.0f, 3.0f));

			ColorPicker* pPicker = pTopRightBox->AddWidget(new ColorPicker());
			pPicker->SetVerticalSizePolicy(ESizePolicy::Stretch);
			pPicker->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pPicker->SetSize(Vector2(50.0f, -1.0f));
		}

		HorizontalBox* pBottomBox = pRoot->AddWidget(new HorizontalBox());
		pBottomBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pBottomBox->SetScrollBarsVisible(false);
		pBottomBox->SetMouseScrollingEnabled(false);

		//Bottom Left:
		{
			HorizontalBox* pBottomLeftBox = pBottomBox->AddWidget(new HorizontalBox());
			pBottomLeftBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pBottomLeftBox->SetMargin(FloatRect(3.0f, 3.0f, 3.0f, 3.0f));
			pBottomLeftBox->SetScrollBarsVisible(true);
			pBottomLeftBox->SetMouseScrollingEnabled(true);

			ComboBox* pComboBox = pBottomLeftBox->AddWidget(new ComboBox());
			pComboBox->AddSelectables({ "Robin", "Emil", "Jacke", "Kappe" });
			pComboBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pComboBox->SetSize(Vector2(150.0f, 40.0f));

			pBottomLeftBox->AddWidget(new EditableTextBox());
		}
		//Bottom Right:
		{
			static float value = 0;

			VerticalBox* pBottomRightBox = pBottomBox->AddWidget(new VerticalBox());
			pBottomRightBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pBottomRightBox->SetMargin(FloatRect(3.0f, 3.0f, 3.0f, 3.0f));
			pBottomRightBox->SetScrollBarsVisible(true);
			pBottomRightBox->SetMouseScrollingEnabled(true);

			pBottomRightBox->AddWidget(new Label("Label"));

			SpinBox<float>* pFloatSpinBox = pBottomRightBox->AddWidget(new SpinBox<float>());
			pFloatSpinBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			pFloatSpinBox->SetDrawColorIndicator(true);
			pFloatSpinBox->SetDelta(0.1f);
			pFloatSpinBox->Value([]() { return value; });
			pFloatSpinBox->OnValueChanged([](float aValue) { value = aValue; });

			Slider<float>* pFloatSlider = pBottomRightBox->AddWidget(new Slider<float>());
			pFloatSlider->SetMinValue(-10.0f);
			pFloatSlider->SetMaxValue(10.0f);
			pFloatSlider->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);

			pBottomRightBox->AddWidget(new SpinBox<int>());
			Slider<int>* pIntSlider = pBottomRightBox->AddWidget(new Slider<int>());
			pIntSlider->SetMinValue(-20);
			pIntSlider->SetMaxValue(30);

			NumericEntryBox<float>* pNumericEntrybox = pBottomRightBox->AddWidget(new NumericEntryBox<float>());
			pNumericEntrybox->Value([]() { return value; });
			pNumericEntrybox->OnValueChanged([](float aValue) { value = aValue; });
			pNumericEntrybox->SetSuffix(" %%");
			pNumericEntrybox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);
		}

		SetRoot(pRoot);
	}

	String TestLayoutPanel::GetDisplayName() const noexcept
	{
		return "Test Layout";
	}

	String TestLayoutPanel::GetPersistKey() const noexcept
	{
		return "Test Layout";
	}
}