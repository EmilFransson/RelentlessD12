#include "MeshRendererComponentDetailCustomization.h"

#include "Core/Editor.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"

namespace Relentless
{
	MeshRendererComponentDetailCustomization::~MeshRendererComponentDetailCustomization() noexcept
	{
		if (CoreObjectBroadcasters::OnEntityComponentPropertyChanged.IsConnected(m_OnMeshRendererComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Detach(m_OnMeshRendererComponentPropertyChangedCallbackID);
	}

	void MeshRendererComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		if (CoreObjectBroadcasters::OnEntityComponentPropertyChanged.IsConnected(m_OnMeshRendererComponentPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Detach(m_OnMeshRendererComponentPropertyChangedCallbackID);

		m_OnMeshRendererComponentPropertyChangedCallbackID = CoreObjectBroadcasters::OnEntityComponentPropertyChanged.Connect([&aDetailLayoutBuilder]
		(MAYBE_UNUSED entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != MeshRendererComponent::StaticType())
					return;

				if (aProperty == "m_MaterialHandle"_h)
					aDetailLayoutBuilder.ForceRefreshDetails();
			});

		EntityDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PALETTE "  Mesh Renderer");

		const AssetHandle heuristicHandle = detailsContext.EntityManager->Get<MeshRendererComponent>(detailsContext.Entities.front()).GetMaterialHandle();

		const bool allSameAndValid = heuristicHandle.IsValid() && std::ranges::all_of(detailsContext.Entities, [&detailsContext, &heuristicHandle](const entity aEntity)
			{
				return detailsContext.EntityManager->Get<MeshRendererComponent>(aEntity).GetMaterialHandle() == heuristicHandle;
			});

		AssetData* pAssetData = nullptr;  
		bool isNone = false;
		
		if (allSameAndValid)
			pAssetData = assetRegistry.FindAsset(heuristicHandle.Uuid);

		if (!pAssetData)
		{
			pAssetData = assetRegistry.FindAsset(pEngineContentSubsystem->GetNoneTexture2DHandle().Uuid);
			isNone = true;
		}

		RLS_ASSERT(pAssetData, "[MeshRendererComponentDetailCustomization::CustomizeDetails]: Asset data is invalid.");	

		categoryBuilder.AddAssetProperty("Material", *pAssetData)
			.AcceptableAssetTypes({ Material::StaticType() })
			.OnAssetsDropped([&detailsContext, &aDetailLayoutBuilder](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					std::ranges::for_each(detailsContext.Entities, [&detailsContext, &assetHandle](const entity aEntity)
						{
							detailsContext.EntityManager->Get<MeshRendererComponent>(aEntity).SetMaterial(assetHandle);
						});

					Application::Get().SubmitToMainThread([&aDetailLayoutBuilder]()
						{
							aDetailLayoutBuilder.GetDetailsView()->Rebuild<EntityDetailsContext>();
						});
				})
			.NameSlot().Label("Material")
			.ValueSlot().AssetThumbnail().Row()
			.RevertSlot().Widget([&detailsContext, &aDetailLayoutBuilder, isNone]()
				{
					Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();
					pRevertBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });

					if (!isNone)
					{
						Button* pButton = pRevertBox->AddWidget(RLS_NEW Button(ICON_FA_ARROW_ROTATE_LEFT));
						pButton->SetBackgroundColor(Colors::Transparent);
						pButton->SetBorderColor(Colors::Transparent);
						pButton->SetHoverColor(Colors::Transparent);
						pButton->SetActiveColor(Colors::Transparent);
						pButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
						pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

						pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
						pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f)); });
						pButton->OnClicked([&detailsContext, &aDetailLayoutBuilder]()
							{
								std::ranges::for_each(detailsContext.Entities, [&detailsContext](const entity aEntity)
									{  
										detailsContext.EntityManager->Get<MeshRendererComponent>(aEntity).RemoveMaterial(); 
									});

								Application::Get().SubmitToMainThread([&aDetailLayoutBuilder]()
									{
										aDetailLayoutBuilder.GetDetailsView()->Rebuild<EntityDetailsContext>();
									});
							});
					}

					return pRevertBox;
				});
	}

	bool MeshRendererComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		if (!std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<MeshRendererComponent>(aEntity); }))
			return false;

		return true;
	}
}