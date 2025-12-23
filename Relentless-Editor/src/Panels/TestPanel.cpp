#include "TestPanel.h"

namespace Relentless
{
	TestPanel::TestPanel(const char* aName, ImGuiWindowFlags aFlags)
		: PanelBase{ aName, aFlags }
	{
		Ref<VerticalBoxEx> pRootBox = new VerticalBoxEx();

		{
			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Checkbox"));
				pHorizontal->AddWidget(new CheckBox())
					->Value([]() { return true; })
					->OnCheckStateChanged([](bool isChecked) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Button"));
				pHorizontal->AddWidget(new Button("Click Me!"));
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Float Dragger"));
				pHorizontal->AddWidget(new FloatDrag())
					->Value([]() { return 0.0f; })
					->OnValueChanged([](float value) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Int Dragger"));
				pHorizontal->AddWidget(new IntDrag())
					->Value([]() { return 0; })
					->OnValueChanged([](int32 value) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Float Slider"));
				pHorizontal->AddWidget(new FloatSlider(-10.0f, 10.0f))
					->Value([]() { return 0.0f; })
					->OnValueChanged([](float value) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Int Slider"));
				pHorizontal->AddWidget(new IntSlider(-10.0f, 10.0f))
					->Value([]() { return 0; })
					->OnValueChanged([](float value) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Color Picker"));
				pHorizontal->AddWidget(new ColorPicker(Vector2(100.0f, 0.0f)))
					->Value([]() { return Colors::White; })
					->OnValueChanged([](Color color) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());
				pHorizontal->SetSizePolicy(ESizePolicy::Stretch);

				pHorizontal->AddWidget(new Label("Searchbar"));
				pHorizontal->AddWidget(new SearchBar("Search...", true))
					->SetSizePolicy(ESizePolicy::Stretch);
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Combo box"));
				pHorizontal->AddWidget(new ComboBox())
					->AddSelectables({ "Jacke", "Kappe" })
					->OnSelectionChanged([](const ComboBox::SelectionInfo& aInfo) {});
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());

				pHorizontal->AddWidget(new Label("Border"));
				pHorizontal->AddWidget(new Border())
					->SetContent(new Label("Inside Border"))
					->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);
			}

			{
				HorizontalBoxEx* pHorizontal = pRootBox->AddWidget(new HorizontalBoxEx());
				pHorizontal->SetSizePolicy(ESizePolicy::Stretch);

				pHorizontal->AddWidget(new Label("Editable Text Box"));
				pHorizontal->AddWidget(new EditableTextBox({100.0f, 0.0f}))
					->SetSizePolicy(ESizePolicy::Stretch);
			}

		}

		SetRoot(pRootBox);
	}
}