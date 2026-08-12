#include "TransformGizmoViewportExtension.h"

#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Module/UIModule.h"

#include "Panels/ViewportPanel.h"

#include "Subsystem/SelectionSubsystem.h"

#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Canvas.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Separator.h"

namespace Relentless
{
	namespace SnapValues
	{
		inline constexpr float Translation[] = { 0.01f, 0.05f, 0.1f, 0.5f, 1.0f, 5.0f, 10.0f, 50.0f, 100.0f };
		inline constexpr float Rotation[] = { 5.0f, 10.0f, 15.0f, 30.0f, 45.0f, 60.0f, 90.0f, 120.0f };
		inline constexpr float Scale[] = { 0.03125f, 0.0625f, 0.1f, 0.125f, 0.25f, 0.5f, 1.0f, 10.0f };
	}

	void TransformGizmoViewportExtension::OnRegistered(const ViewportPanel& aViewportPanel)
	{
		m_pViewportPanel = &aViewportPanel;
	}

	void TransformGizmoViewportExtension::ExtendToolbar(ViewportToolbarSlots& aSlots)
	{
		auto CreateTransformGizmoButton = [this, &aSlots]
		(ETransformGizmoType aTransformGizmoType, StringView aIcon, StringView aTooltip, Callback<void()>&& aCallback) -> Button*
			{
				Button* pButton = aSlots.Left->AddWidget(Button::CreateTransparent(aIcon));
				pButton->OnClicked([callback = std::forward<Callback<void()>>(aCallback)]() { callback(); });
				pButton->OnMouseEnter([this, aTransformGizmoType](Button* aButton)
					{
						if (m_TransformGizmoController.GetActiveTransformType() == aTransformGizmoType)
							return;
						aButton->SetTextColor(Colors::TextDefault);
					});
				pButton->OnMouseExit([this, aTransformGizmoType](Button* aButton)
					{
						if (m_TransformGizmoController.GetActiveTransformType() == aTransformGizmoType)
							return;
						aButton->SetTextColor(Colors::TextInactive);
					});
				pButton->SetTextColor(m_TransformGizmoController.GetActiveTransformType() == aTransformGizmoType ? Colors::SoftOrange : Colors::TextInactive);
				pButton->SetTooltipText(aTooltip);
				pButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
				pButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
				pButton->SetSize(Vector2(25.0f, 25.0f));

				return pButton;
			};

		m_pTransformGizmoSelectButton = CreateTransformGizmoButton(ETransformGizmoType::None, ICON_FA_ARROW_POINTER, "Select objects (Q)", Callback<void()>::Bind(this, &TransformGizmoViewportExtension::OnTransformGizmoSelectModeSelected));
		m_pTransformGizmoTranslateButton = CreateTransformGizmoButton(ETransformGizmoType::Translate, ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, "Select and translate objects (W)", Callback<void()>::Bind(this, &TransformGizmoViewportExtension::OnTransformGizmoTranslateModeSelected));
		m_pTransformGizmoRotateButton = CreateTransformGizmoButton(ETransformGizmoType::Rotate, ICON_FA_ARROW_ROTATE_RIGHT, "Select and rotate objects (E)", Callback<void()>::Bind(this, &TransformGizmoViewportExtension::OnTransformGizmoRotateModeSelected));
		m_pTransformGizmoScaleButton = CreateTransformGizmoButton(ETransformGizmoType::Scale, ICON_FA_VECTOR_SQUARE, "Select and scale objects (R)", Callback<void()>::Bind(this, &TransformGizmoViewportExtension::OnTransformGizmoScaleModeSelected));

		aSlots.Left->AddWidget(RLS_NEW Separator(false))
			->SetThickness(1.0f)
			->SetHorizontalSizePolicy(ESizePolicy::Fixed);

		m_pTransformGizmoModeButton = aSlots.Left->AddWidget(Button::CreateTransparent(m_TransformGizmoController.GetActiveMode() == ETransformGizmoMode::World ? ICON_FA_GLOBE : ICON_FA_ARROWS_TO_DOT));
		m_pTransformGizmoModeButton->OnClicked(this, &TransformGizmoViewportExtension::OnTransformGizmoModeToggle);
		m_pTransformGizmoModeButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		m_pTransformGizmoModeButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
		m_pTransformGizmoModeButton->SetTooltipText("World scale coordinates (T - toggle)");
		m_pTransformGizmoModeButton->SetTextColor(Colors::TextInactive);
		m_pTransformGizmoModeButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoModeButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoModeButton->SetSize(Vector2(25.0f, 25.0f));

		HorizontalBox* pGizmoMovementBox = aSlots.Left->AddWidget(RLS_NEW HorizontalBox());
		pGizmoMovementBox->SetMargin(FloatRect::WithLeft(10.0f));
		pGizmoMovementBox->SetSpacing(10.0f);

		HorizontalBox* pTranslationGroupBox = pGizmoMovementBox->AddWidget(RLS_NEW HorizontalBox());

		m_pTransformGizmoTranslationMovementModeButton = pTranslationGroupBox->AddWidget(Button::CreateTransparent(ICON_FA_TABLE_CELLS));
		m_pTransformGizmoTranslationMovementModeButton->OnClicked(this, &TransformGizmoViewportExtension::OnToggleGizmoTranslationMovementModeButtonClicked);
		m_pTransformGizmoTranslationMovementModeButton->SetTextColor(m_TransformGizmoController.GetActiveTranslationMovementMode() == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoTranslationMovementModeButton->SetTooltipText("Toggles whether entities snap to grid when translated. Use sub menu to choose grid size.");
		m_pTransformGizmoTranslationMovementModeButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoTranslationMovementModeButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoTranslationMovementModeButton->SetSize(Vector2(25.0f, 25.0f));

		m_pTransformGizmoTranslationSnapValuesButton = pTranslationGroupBox->AddWidget(Button::CreateTransparent("0.1m" ICON_FA_CHEVRON_DOWN));
		m_pTransformGizmoTranslationSnapValuesButton->OnClicked(this, &TransformGizmoViewportExtension::OnTranslationSnapValuesButtonClicked);
		m_pTransformGizmoTranslationSnapValuesButton->SetFont(UI::Fonts::Get("FA_SMALL"));
		m_pTransformGizmoTranslationSnapValuesButton->SetTextColor(Colors::TextInactive);
		m_pTransformGizmoTranslationSnapValuesButton->SetTooltipText("Toggles whether entities snap to grid when translated. Use sub menu to choose grid size.");
		m_pTransformGizmoTranslationSnapValuesButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoTranslationSnapValuesButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoTranslationSnapValuesButton->SetSize(Vector2(45.0f, 25.0f));

		HorizontalBox* pRotationGroupBox = pGizmoMovementBox->AddWidget(RLS_NEW HorizontalBox());

		m_pTransformGizmoRotationMovementModeButton = pRotationGroupBox->AddWidget(Button::CreateTransparent(ICON_FA_ROTATE));
		m_pTransformGizmoRotationMovementModeButton->OnClicked([this]()
			{
				m_TransformGizmoController.ToggleRotationMovementMode();
				const ETransformGizmoMovementMode activeRotationMovementMode = m_TransformGizmoController.GetActiveRotationMovementMode();
				m_pTransformGizmoRotationMovementModeButton->SetTextColor(activeRotationMovementMode == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
			});
		m_pTransformGizmoRotationMovementModeButton->SetTextColor(m_TransformGizmoController.GetActiveRotationMovementMode() == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoRotationMovementModeButton->SetTooltipText("Toggles whether entities snap to preset angles when rotated. Use sub menu to choose the angle increment.");
		m_pTransformGizmoRotationMovementModeButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoRotationMovementModeButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoRotationMovementModeButton->SetSize(Vector2(25.0f, 25.0f));

		m_pTransformGizmoRotationSnapValuesButton = pRotationGroupBox->AddWidget(Button::CreateTransparent("10°" ICON_FA_CHEVRON_DOWN));
		m_pTransformGizmoRotationSnapValuesButton->OnClicked(this, &TransformGizmoViewportExtension::OnRotationSnapValuesButtonClicked);
		m_pTransformGizmoRotationSnapValuesButton->SetFont(UI::Fonts::Get("FA_SMALL"));
		m_pTransformGizmoRotationSnapValuesButton->SetTextColor(Colors::TextInactive);
		m_pTransformGizmoRotationSnapValuesButton->SetTooltipText("Toggles whether entities snap to preset angles when rotated. Use sub menu to choose the angle increment.");
		m_pTransformGizmoRotationSnapValuesButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoRotationSnapValuesButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoRotationSnapValuesButton->SetSize(Vector2(45.0f, 25.0f));

		HorizontalBox* pScaleGroupBox = pGizmoMovementBox->AddWidget(RLS_NEW HorizontalBox());

		m_pTransformGizmoScaleMovementModeButton = pScaleGroupBox->AddWidget(Button::CreateTransparent(ICON_FA_MAXIMIZE));
		m_pTransformGizmoScaleMovementModeButton->OnMouseEnter([pScaleGroupBox](Button*) { pScaleGroupBox->SetBackgroundColor(Color(0.3f, 0.305f, 0.31f, 0.4f)); });
		m_pTransformGizmoScaleMovementModeButton->OnMouseExit([pScaleGroupBox](Button*) { pScaleGroupBox->SetBackgroundColor(Colors::Transparent); });
		m_pTransformGizmoScaleMovementModeButton->OnClicked([this]()
			{
				m_TransformGizmoController.ToggleScaleMovementMode();
				const ETransformGizmoMovementMode activeScaleMovementMode = m_TransformGizmoController.GetActiveScaleMovementMode();
				m_pTransformGizmoScaleMovementModeButton->SetTextColor(activeScaleMovementMode == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
			});
		m_pTransformGizmoScaleMovementModeButton->SetTextColor(m_TransformGizmoController.GetActiveScaleMovementMode() == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoScaleMovementModeButton->SetTooltipText("Toggles whether entities resize by a preset ratio when scaled. Use sub menu to choose the scaling increment.");
		m_pTransformGizmoScaleMovementModeButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoScaleMovementModeButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoScaleMovementModeButton->SetSize(Vector2(25.0f, 25.0f));

		m_pTransformGizmoScaleSnapValuesButton = pScaleGroupBox->AddWidget(Button::CreateTransparent("0.25" ICON_FA_CHEVRON_DOWN));
		m_pTransformGizmoScaleSnapValuesButton->OnClicked(this, &TransformGizmoViewportExtension::OnScaleSnapValuesButtonClicked);
		m_pTransformGizmoScaleSnapValuesButton->OnMouseEnter([pScaleGroupBox](Button*) { pScaleGroupBox->SetBackgroundColor(Color(0.3f, 0.305f, 0.31f, 0.4f)); });
		m_pTransformGizmoScaleSnapValuesButton->OnMouseExit([pScaleGroupBox](Button*) { pScaleGroupBox->SetBackgroundColor(Colors::Transparent); });
		m_pTransformGizmoScaleSnapValuesButton->SetFont(UI::Fonts::Get("FA_SMALL"));
		m_pTransformGizmoScaleSnapValuesButton->SetTextColor(Colors::TextInactive);
		m_pTransformGizmoScaleSnapValuesButton->SetTooltipText("Toggles whether entities resize by a preset ratio when scaled. Use sub menu to choose the scaling increment.");
		m_pTransformGizmoScaleSnapValuesButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoScaleSnapValuesButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
		m_pTransformGizmoScaleSnapValuesButton->SetSize(Vector2(45.0f, 25.0f));
	}

	bool TransformGizmoViewportExtension::HandleInput(const ViewportInputEvent& aInputEvent)
	{
		switch (aInputEvent.Type)
		{
		case EViewportInputType::KeyPressed:
		{
			if (aInputEvent.PointerInfo.IsAnyButtonDown())
				return false;

			switch (aInputEvent.Key)
			{
			case RLS_Key::Q: OnTransformGizmoSelectModeSelected();    return true;
			case RLS_Key::W: OnTransformGizmoTranslateModeSelected(); return true;
			case RLS_Key::E: OnTransformGizmoRotateModeSelected();    return true;
			case RLS_Key::R: OnTransformGizmoScaleModeSelected();     return true;
			case RLS_Key::T: OnTransformGizmoModeToggle();            return true;
			default: return false;
			}
		}
		case EViewportInputType::MouseButtonPressed:
		{
			if (aInputEvent.Button != RLS_Button::Left)
				return false;

			if (!HasSelection())
				return false;

			if (m_TransformGizmoController.GetCurrentInteractionState() != ETransformGizmoInteractionState::Hovering)
				return false;

			if (EnumHasAnyFlags(aInputEvent.KeyboardModifiers, KeyboardModifierMask::Alt))
			{
				switch (m_TransformGizmoController.GetActiveTransformType())
				{
				case ETransformGizmoType::Translate:
				case ETransformGizmoType::Rotate:
					Editor::Get()->OnViewportEntityDuplicationRequest();
					break;
				default:
					break;
				}
			}

			return true;
		}
		case EViewportInputType::MouseDragBegin:
		{
			if (aInputEvent.Button != RLS_Button::Left)
				return false;

			return HasSelection() && m_TransformGizmoController.GetCurrentInteractionState() == ETransformGizmoInteractionState::Hovering;
		}
		case EViewportInputType::MouseButtonReleased:
		{
			if (aInputEvent.Button != RLS_Button::Left)
				return false;

			return HasSelection() && m_TransformGizmoController.IsInteracting();
		}
		default:
			return false;
		}
	}

	void TransformGizmoViewportExtension::OnCanvasRenderEnd()
	{
		auto pEditor = Editor::Get();

		std::vector<entity> participatingEntities = pEditor->GetTransformSelection();
		if (participatingEntities.empty())
			return;

		Canvas* pCanvas = m_pViewportPanel->GetCanvas();
		if (!pCanvas)
			return;

		const ViewportClient& client = m_pViewportPanel->GetClient();

		const TransformGizmoControllerContext transformContext
		{
			.Entities = std::move(participatingEntities),
			.WorldToView = client.GetCamera().GetViewTransform().WorldToView,
			.ViewToClip = client.GetCamera().GetViewTransform().ViewToClip,
			.Rect = pCanvas->GetScreenRect(),
			.pScene = pEditor->GetActiveScene(),
		};

		m_TransformGizmoController.Execute(transformContext);
	}

	bool TransformGizmoViewportExtension::WantsExclusiveInput() const noexcept
	{
		return m_TransformGizmoController.GetCurrentInteractionState() == ETransformGizmoInteractionState::Using;
	}

	void TransformGizmoViewportExtension::BuildSnapMenu(Span<const float> someValues, Callback<float()>&& aGetter, Callback<void(float)>&& aSetter, StringView aSectionName, StringView aBaseTooltip, StringView aOptionalSuffix, Button* aLabelButton) noexcept
	{
		auto FormatSnapValue = [aOptionalSuffix](float aValue) -> String
			{
				String result = std::format("{:.5f}", aValue);
				const size_t lastNonZero = result.find_last_not_of('0');
				result.erase(lastNonZero + (result[lastNonZero] == '.' ? 0 : 1));
				result.append(aOptionalSuffix);
				return result;
			};

		ContextMenuBuilder builder;

		builder.AddSection(aSectionName)
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		for (float value : someValues)
		{
			const String valueString = FormatSnapValue(value);

			builder.AddRadioButton(valueString)
				.Font(UI::Fonts::Get("Small"))
				.Tooltip(std::format("{} {}", aBaseTooltip, value))
				.Value([value, getter = aGetter]() { return Math::AreValuesClose(getter(), value); })
				.OnValueChanged([value, setter = aSetter, aLabelButton, valueString](bool)
					{
						setter(value);
						aLabelButton->SetText(std::format("{} {}", valueString, ICON_FA_CHEVRON_DOWN));
					});
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	bool TransformGizmoViewportExtension::HasSelection() const noexcept
	{
		return Editor::Get()->GetSubsystem<SelectionSubsystem>()->GetSelectedEntityCount() > 0u;
	}

	void TransformGizmoViewportExtension::OnToggleGizmoTranslationMovementModeButtonClicked()
	{
		m_TransformGizmoController.ToggleTranslationMovementMode();
		const ETransformGizmoMovementMode activeTranslationMovementMode = m_TransformGizmoController.GetActiveTranslationMovementMode();
		m_pTransformGizmoTranslationMovementModeButton->SetTextColor(activeTranslationMovementMode == ETransformGizmoMovementMode::Snap ? Colors::SoftOrange : Colors::TextInactive);
	}

	void TransformGizmoViewportExtension::OnTransformGizmoModeToggle()
	{
		m_TransformGizmoController.ToggleActiveMode();
		const ETransformGizmoMode activeMode = m_TransformGizmoController.GetActiveMode();

		m_pTransformGizmoModeButton->SetText(activeMode == ETransformGizmoMode::World ? ICON_FA_GLOBE : ICON_FA_ARROWS_TO_DOT);
		m_pTransformGizmoModeButton->SetTooltipText(activeMode == ETransformGizmoMode::World ? "World space coordinates (T - toggle)" : "Local space coordinates (T - toggle)");
	}

	void TransformGizmoViewportExtension::OnTransformGizmoSelectModeSelected()
	{
		m_TransformGizmoController.SetActiveType(ETransformGizmoType::None);
		m_pTransformGizmoSelectButton->SetTextColor(Colors::SoftOrange);
		m_pTransformGizmoTranslateButton->SetTextColor(m_pTransformGizmoTranslateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoRotateButton->SetTextColor(m_pTransformGizmoRotateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoScaleButton->SetTextColor(m_pTransformGizmoScaleButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
	}

	void TransformGizmoViewportExtension::OnTransformGizmoTranslateModeSelected()
	{
		m_TransformGizmoController.SetActiveType(ETransformGizmoType::Translate);
		m_pTransformGizmoTranslateButton->SetTextColor(Colors::SoftOrange);
		m_pTransformGizmoSelectButton->SetTextColor(m_pTransformGizmoSelectButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoRotateButton->SetTextColor(m_pTransformGizmoRotateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoScaleButton->SetTextColor(m_pTransformGizmoScaleButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
	}

	void TransformGizmoViewportExtension::OnTransformGizmoRotateModeSelected()
	{
		m_TransformGizmoController.SetActiveType(ETransformGizmoType::Rotate);
		m_pTransformGizmoRotateButton->SetTextColor(Colors::SoftOrange);
		m_pTransformGizmoTranslateButton->SetTextColor(m_pTransformGizmoTranslateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoSelectButton->SetTextColor(m_pTransformGizmoSelectButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoScaleButton->SetTextColor(m_pTransformGizmoScaleButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
	}

	void TransformGizmoViewportExtension::OnTransformGizmoScaleModeSelected()
	{
		m_TransformGizmoController.SetActiveType(ETransformGizmoType::Scale);
		m_pTransformGizmoScaleButton->SetTextColor(Colors::SoftOrange);
		m_pTransformGizmoRotateButton->SetTextColor(m_pTransformGizmoRotateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoTranslateButton->SetTextColor(m_pTransformGizmoTranslateButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
		m_pTransformGizmoSelectButton->SetTextColor(m_pTransformGizmoSelectButton->IsHovered() ? Colors::SoftOrange : Colors::TextInactive);
	}

	void TransformGizmoViewportExtension::OnTranslationSnapValuesButtonClicked()
	{
		BuildSnapMenu(SnapValues::Translation,
			[this]() { return m_TransformGizmoController.GetTranslationSnapValue(); },
			[this](float aValue) { m_TransformGizmoController.SetTranslationSnapValue(aValue); },
			"Snap Values",
			"Snaps translation values to increments of",
			"m",
			m_pTransformGizmoTranslationSnapValuesButton);
	}

	void TransformGizmoViewportExtension::OnRotationSnapValuesButtonClicked()
	{
		BuildSnapMenu(SnapValues::Rotation,
			[this]() { return m_TransformGizmoController.GetRotationSnapValue(); },
			[this](float aValue) { m_TransformGizmoController.SetRotationSnapValue(aValue); },
			"Rotation Increment",
			"Sets rotation grid angle to",
			"°",
			m_pTransformGizmoRotationSnapValuesButton);
	}

	void TransformGizmoViewportExtension::OnScaleSnapValuesButtonClicked()
	{
		BuildSnapMenu(SnapValues::Scale,
			[this]() { return m_TransformGizmoController.GetScaleSnapValue(); },
			[this](float aValue) { m_TransformGizmoController.SetScaleSnapValue(aValue); },
			"Snap Values",
			"Snaps scale values to increments of",
			"",
			m_pTransformGizmoScaleSnapValuesButton);
	}
}
