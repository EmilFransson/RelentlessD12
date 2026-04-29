#include "EnvironmentDetailCustomization.h"

#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	EnvironmentDetailCustomization::~EnvironmentDetailCustomization() noexcept
	{
		if (CoreObjectBroadcasters::OnAssetPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnAssetPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);
	}

	void EnvironmentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EnvironmentDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>();
		
		if (CoreObjectBroadcasters::OnAssetPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
			CoreObjectBroadcasters::OnAssetPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);

		m_OnAssetPropertyChangedCallbackID = CoreObjectBroadcasters::OnAssetPropertyChanged.Connect([this, &aDetailLayoutBuilder, &detailsContext]
		(IAsset* aAsset, MAYBE_UNUSED TypeIndex aAssetType, MAYBE_UNUSED uint64 aProperty)
			{
				if (m_SuspendRefresh)
					return;

				if (aAsset == detailsContext.Environment)
					aDetailLayoutBuilder.ForceRefreshDetails();
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CLOUD_SUN "  Environment");
		AssetData* pAssetData = ModuleManager::LoadModuleChecked<AssetRegistryModule>().FindAsset(detailsContext.Environment->GetEnvironmentMapHandle().Uuid);

		categoryBuilder.AddAssetProperty("Environment Map", *pAssetData)
			.AcceptableAssetTypes({ TextureCube::StaticType() })
			.OnAssetsDropped([&detailsContext](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					detailsContext.Environment->SetEnvironmentMapHandle(assetHandle); 
				})
			.NameSlot().Label("Environment Map")
			.ValueSlot().AssetThumbnail();

		Ref<PropertyHandle<float>> pIntensityHandle = RLS_NEW PropertyHandle<float>(
			[&detailsContext]() { return detailsContext.Environment->GetIntensity(); },
			[this, &detailsContext](const float& aValue) 
			{ 
				ScopedSuspend scopedSuspend(m_SuspendRefresh);
				detailsContext.Environment->SetIntensity(aValue); 
			},
			1.0f);

		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().Slider().Range(0.0f, 1.0f);
	}

	bool EnvironmentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		return aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>().Environment != nullptr;
	}
}