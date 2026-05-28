#include "ContentBrowserPanel.h"
#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Module/UIModule.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "UI/Views/Assets/AssetView.h"
#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	ContentBrowserPanel::ContentBrowserPanel() noexcept
		: PanelBase("Content Browser", ImGuiWindowFlags_None)
	{
		SetPadding(Vector2(2.0f, 0.0f));
		
		Ref<VerticalBox> pRoot = RLS_NEW VerticalBox();
		pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pRoot->SetPadding(FloatRect(10.0f, 5.0f, 10.0f, 5.0f));

		pRoot->AddWidget(BuildToolbar());
		m_pAssetsView = pRoot->AddWidget(RLS_NEW AssetView());
		
		m_pAssetsView->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pAssetsView->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pAssetsView->SetMargin(FloatRect::WithTop(5.0f));

		SetRoot(pRoot);
	}

	ContentBrowserPanel::~ContentBrowserPanel() noexcept = default;

	String ContentBrowserPanel::GetDisplayName() const noexcept
	{
		return "Content Browser";
	}

	String ContentBrowserPanel::GetPersistKey() const noexcept
	{
		return "Content Browser";
	}

	Ref<Button> ContentBrowserPanel::BuildAddAssetButton() noexcept
	{
		const String label = std::format("{} Add", ICON_FA_PLUS);
		Ref<Button> pButton = RLS_NEW Button(label);
		pButton->SetFont(UI::Fonts::Get("Medium"));
		pButton->OnClicked(this, &ContentBrowserPanel::OnAddAssetButtonClicked);
		pButton->SetPadding(Vector2(12.0f, 6.0f));

		return pButton;
	}

	Ref<HorizontalBox> ContentBrowserPanel::BuildToolbar() noexcept
	{
		Ref<HorizontalBox> pToolbarBox = RLS_NEW HorizontalBox();
		pToolbarBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pToolbarBox->AddWidget(BuildAddAssetButton());

		return pToolbarBox;
	}

	void ContentBrowserPanel::OnAddAssetButtonClicked() noexcept
	{
		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		if (!pAssetDefinitionRegistry)
			return;

		ContextMenuBuilder builder;

		builder.AddSection("Core")
				.Font(UI::Fonts::Get("Small"))
				.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
				.TextColor(Colors::TextInactive)
				.Thickness(0.5f);

		for (auto& pDefinition : pAssetDefinitionRegistry->GetAllAssetDefinitions()
			| std::views::filter([](const auto& aDefinition) { return aDefinition->SupportsCreateNew(); }))
		{
			builder.AddItem(pDefinition->GetAssetDisplayName())
					.OnClicked([pDefinition]()
					{
					});
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}
}
