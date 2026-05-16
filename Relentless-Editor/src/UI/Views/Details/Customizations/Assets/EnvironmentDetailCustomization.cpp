#include "EnvironmentDetailCustomization.h"

#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	void EnvironmentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EnvironmentDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>();
		
		if (detailsContext.Environment->OnPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
		{
			detailsContext.Environment->OnPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);
			m_OnAssetPropertyChangedCallbackID = INVALID_CALLBACK_ID;
		}

		m_OnAssetPropertyChangedCallbackID = detailsContext.Environment->OnPropertyChanged.Connect([this, &aDetailLayoutBuilder]
		(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty)
			{
				if (m_SuspendRefresh)
					return;

				aDetailLayoutBuilder.ForceRefreshDetails();
			});

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CLOUD_SUN "  Environment");
		AssetData* pAssetData = ModuleManager::LoadModuleChecked<AssetRegistryModule>().FindAsset(detailsContext.Environment->GetEnvironmentMapHandle().Uuid);

		//Source Type:
		{
			Ref<PropertyHandle<int>> pHandle = RLS_NEW PropertyHandle<int>(
				[&detailsContext]() { return static_cast<int>(detailsContext.Environment->GetSourceType()); },
				[this, &detailsContext](const int& aValue)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Environment->SetSourceType(static_cast<EEnvironmentSourceType>(aValue));
				},
				0);

			categoryBuilder.AddProperty<int>("Source Type", pHandle)
				.NameSlot().Label("Source Type")
				.ValueSlot().ComboBox().Options({ "Cubemap", "Solid Color"});
		}

		//Environment Map:
		{
			categoryBuilder.AddAssetProperty("Environment Map", *pAssetData)
				.AcceptableAssetTypes({ TextureCube::StaticType() })
				.OnAssetsDropped([&detailsContext](Span<const AssetData> someAssetDatas)
					{
						const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
						detailsContext.Environment->SetEnvironmentMapHandle(assetHandle);
					})
				.NameSlot().Label("Environment Map")
				.ValueSlot().AssetThumbnail();
		}

		//Color:
		{
			Ref<PropertyHandle<Color>> pHandle = RLS_NEW PropertyHandle<Color>(
				[&detailsContext]() { return detailsContext.Environment->GetSolidColor(); },
				[this, &detailsContext](const Color& aColor)
				{
					ScopedSuspend scopedSuspend(m_SuspendRefresh);
					detailsContext.Environment->SetSolidColor(aColor);
				},
				Colors::Black);

			categoryBuilder.AddProperty<Color>("Solid Color", pHandle)
				.NameSlot().Label("Solid Color")
				.ValueSlot().ColorPicker();
		}
		
		//Intensity:
		{
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
	}

	void EnvironmentDetailCustomization::OnDestroy(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EnvironmentDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>();

		if (detailsContext.Environment->OnPropertyChanged.IsConnected(m_OnAssetPropertyChangedCallbackID))
			detailsContext.Environment->OnPropertyChanged.Detach(m_OnAssetPropertyChangedCallbackID);

		m_OnAssetPropertyChangedCallbackID = INVALID_CALLBACK_ID;
	}

	bool EnvironmentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		return aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>().EnvironmentHandle != AssetHandle::INVALID;
	}
}