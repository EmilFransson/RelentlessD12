#include "WidgetShowcasePanel.h"

#include <UI/Widgets/Border.h>
#include <UI/Widgets/Button.h>
#include <UI/Widgets/CheckBox.h>
#include <UI/Widgets/ColorPicker.h>
#include <UI/Widgets/ComboBox.h>
#include <UI/Widgets/EditableTextBox.h>
#include <UI/Widgets/FloatDrag.h>
#include <UI/Widgets/FloatSlider.h>
#include <UI/Widgets/HorizontalBox.h>
#include <UI/Widgets/IntDrag.h>
#include <UI/Widgets/IntSlider.h>
#include <UI/Widgets/Label.h>
#include <UI/Widgets/SearchBar.h>
#include <UI/Widgets/VerticalBox.h>

namespace Relentless
{
	WidgetShowcasePanel::WidgetShowcasePanel() noexcept
		: PanelBase{ "Widget Showcase", ImGuiWindowFlags_None }
	{
		Ref<VerticalBox> pRootBox = new VerticalBox();

		{
			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Checkbox"));
				pHorizontal->AddWidget(new CheckBox())
					->Value([]() { return true; })
					->OnCheckStateChanged([](bool) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Button"));
				pHorizontal->AddWidget(new Button("Click Me!"));
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Float Dragger"));
				pHorizontal->AddWidget(new FloatDrag())
					->Value([]() { return 0.0f; })
					->OnValueChanged([](float) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Int Dragger"));
				pHorizontal->AddWidget(new IntDrag())
					->Value([]() { return 0; })
					->OnValueChanged([](int32) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Float Slider"));
				pHorizontal->AddWidget(new FloatSlider(-10.0f, 10.0f))
					->Value([]() { return 0.0f; })
					->OnValueChanged([](float) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Int Slider"));
				pHorizontal->AddWidget(new IntSlider(-10.0f, 10.0f))
					->Value([]() { return 0; })
					->OnValueChanged([](float) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Color Picker"));
				ColorPicker* pPicker = pHorizontal->AddWidget(new ColorPicker());
				pPicker
					->Value([]() { return Colors::White; })
					->OnValueChanged([](Color) {});
				
				pPicker->SetHorizontalSizePolicy(ESizePolicy::Fixed);
				pPicker->SetSize(Vector2(100.0f, 0.0f));
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());
				pHorizontal->SetHorizontalSizePolicy(ESizePolicy::Stretch);

				pHorizontal->AddWidget(new Label("Searchbar"));
				pHorizontal->AddWidget(new SearchBar("Search...", true))
					->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Combo box"));
				pHorizontal->AddWidget(new ComboBox())
					->AddSelectables({ "Jacke", "Kappe" })
					->OnSelectionChanged([](const ComboBox::SelectionInfo&) {});
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());

				pHorizontal->AddWidget(new Label("Border"));
				pHorizontal->AddWidget(new Border())
					->SetContent(new Label("Inside Border"))
					->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);
			}

			{
				HorizontalBox* pHorizontal = pRootBox->AddWidget(new HorizontalBox());
				pHorizontal->SetHorizontalSizePolicy(ESizePolicy::Stretch);

				pHorizontal->AddWidget(new Label("Editable Text Box"));
				pHorizontal->AddWidget(new EditableTextBox())
					->SetHorizontalSizePolicy(ESizePolicy::Stretch);
			}

		}

		SetRoot(pRootBox);
	}
}