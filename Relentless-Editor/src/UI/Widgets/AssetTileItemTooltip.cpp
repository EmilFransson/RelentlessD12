#include "AssetTileItemTooltip.h"

#include "Core/Editor.h"

#include "HorizontalBox.h"

#include "Label.h"

#include "Separator.h"
#include "Subsystem/AssetDefinitionRegistry.h"

#include "VerticalBox.h"

namespace Relentless
{
	AssetTileItemTooltip::AssetTileItemTooltip(const AssetData& aAssetData) noexcept
		: Tooltip("")
	{
		AssetDefinitionRegistry& assetDefinitionRegistry = *Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		const Ref<IAssetDefinition>& pAssetDefinition = assetDefinitionRegistry.GetDefinitionForAsset(aAssetData);

		m_pSlot = RLS_NEW VerticalBox();
		m_pSlot->SetBackgroundColor(Colors::ContextMenuColorDefault);
		m_pSlot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pSlot->SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_pSlot->AddWidget(RLS_NEW Label(aAssetData.Name));
		
		HorizontalBox* pTypeBox = m_pSlot->AddWidget(RLS_NEW HorizontalBox());
		pTypeBox->AddWidget(RLS_NEW Label(pAssetDefinition->GetAssetIcon()))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		pTypeBox->AddWidget(RLS_NEW Label(pAssetDefinition->GetAssetDisplayName()))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pSlot->AddWidget(RLS_NEW Separator())
			->SetActiveColor(Colors::TextInactive);

		HorizontalBox* pPathBox = m_pSlot->AddWidget(RLS_NEW HorizontalBox());
		pPathBox->AddWidget(RLS_NEW Label("Path:"))
			->SetTextColor(Colors::TextInactive)
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		pPathBox->AddWidget(RLS_NEW Label(aAssetData.PackagePath.string()))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
	}

	void AssetTileItemTooltip::OnRender() noexcept
	{
		ImGui::PushID((const void*)this);

		constexpr Color backgroundColor = Colors::ContextMenuColorDefault;
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(backgroundColor.R(), backgroundColor.G(), backgroundColor.G(), backgroundColor.A()));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 6));
		ImGui::BeginTooltip();

		m_pSlot->AssignSize(m_pSlot->ReportSize());
		m_pSlot->Render();

		ImGui::EndTooltip();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		ImGui::PopID();
	}
}