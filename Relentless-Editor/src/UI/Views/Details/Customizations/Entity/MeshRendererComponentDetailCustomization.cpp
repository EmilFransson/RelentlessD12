#include "MeshRendererComponentDetailCustomization.h"

#include "Core/Editor.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/Context/EntityDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/DetailHelpers.h"

namespace Relentless
{
	static Ref<HorizontalBox> OnBuildLightingChannelsRequested(EntityDetailsContext& aContext) noexcept
	{
		auto AllHaveChannelsSet = [&aContext](ELightChannel aLightChannel) -> bool
			{
				return std::ranges::all_of(aContext.Entities, [&aContext, aLightChannel](entity aEntity)
					{
						const MeshRendererComponent& meshRendererComponent = aContext.EntityManager->Get<MeshRendererComponent>(aEntity);
						return meshRendererComponent.HasLightChannelsEnabled(aLightChannel);
					});
			};

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
		pBox->SetSpacing(5.0f);

		for (uint32 i = 0; i <= 5; ++i)
		{
			const ELightChannel lightChannel = static_cast<ELightChannel>(1u << i);

			Button* pButton = pBox->AddWidget(RLS_NEW Button(std::format("{}", i)));
			pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
			pButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
			pButton->SetSize({ 20.0f, 20.0f });
			pButton->OnClicked([&aContext, lightChannel, AllHaveChannelsSet]()
				{
					const bool setChannel = !AllHaveChannelsSet(lightChannel);

					std::ranges::for_each(aContext.Entities, [&aContext, lightChannel, setChannel](entity aEntity)
						{
							MeshRendererComponent& meshRendererComponent = aContext.EntityManager->Get<MeshRendererComponent>(aEntity);
							meshRendererComponent.SetLightChannelEnabled(lightChannel, setChannel);
						});
				});

			pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::White); });
			pButton->OnMouseExit([lightChannel, AllHaveChannelsSet](Button* aButton)
				{
					if (AllHaveChannelsSet(lightChannel))
						return;

					aButton->SetTextColor(Colors::Gray);
				});

			const bool allSet = AllHaveChannelsSet(lightChannel);
			pButton->SetBackgroundColor(allSet ? Colors::Blue : Colors::Black);
			pButton->SetHoverColor(allSet ? Colors::Blue : Colors::Black);
			pButton->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
			pButton->SetTextColor(allSet ? Colors::White : Colors::Gray);
		}

		return pBox;
	}

	static Ref<HorizontalBox> OnBuildLightingChannelsRevertButtonRequested(EntityDetailsContext& aContext) noexcept
	{
		auto AllHaveOnlyChannel1Set = [&aContext]() -> bool
			{
				return std::ranges::all_of(aContext.Entities, [&aContext](entity aEntity)
					{
						const MeshRendererComponent& meshRendererComponent = aContext.EntityManager->Get<MeshRendererComponent>(aEntity);
						return meshRendererComponent.GetLightChannels() == ELightChannel::Channel1;
					});
			};

		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
		pBox->SetSpacing(5.0f);

		Button* pButton = pBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
		pButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
		pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pButton->SetIsVisible(!AllHaveOnlyChannel1Set());

		pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 1.0f)); });
		pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Color(1.0f, 1.0f, 1.0f, 0.5f)); });
		pButton->OnClicked([&aContext]()
			{
				std::ranges::for_each(aContext.Entities, [&aContext](const entity aEntity)
					{
						MeshRendererComponent& meshRendererComponent = aContext.EntityManager->Get<MeshRendererComponent>(aEntity);
						meshRendererComponent.SetLightChannelEnabled(ELightChannel::All, false);
						meshRendererComponent.SetLightChannelEnabled(ELightChannel::Channel1, true);
					});
			});

		return pBox;
	}

	void MeshRendererComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		SetupConnections();
		
		using MRC = MeshRendererComponent;
		
		EntityDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		DetailHelpers::EntityHandleFactory<MRC> handleFactory({ .Entities = detailsContext.Entities, .EntityManager = *detailsContext.EntityManager });

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PALETTE "  Mesh Renderer");
		categoryBuilder.AddHeaderAction("Remove", [this]() { RemoveFromInspected(); });

		handleFactory.MakeAssetTarget(categoryBuilder, "Material", { Material::StaticType() }, &MRC::GetMaterialHandle, &MRC::SetMaterial, &MRC::RemoveMaterial);

		auto pCastShadowsHandle = handleFactory.Make(&MRC::IsCastingShadows, &MRC::SetCastShadows, true);
		categoryBuilder.AddProperty<bool>("Cast Shadows", pCastShadowsHandle)
			.NameSlot().Label("Cast Shadows")
			.ValueSlot().CheckBox();

		categoryBuilder.AddProperty<bool>("Lighting Channels", nullptr)
			.NameSlot().Label("Lighting Channels")
			.ValueSlot().Widget([&detailsContext]() { return OnBuildLightingChannelsRequested(detailsContext); })
			.RevertSlot().Widget([&detailsContext]() { return OnBuildLightingChannelsRevertButtonRequested(detailsContext); });
	}

	void MeshRendererComponentDetailCustomization::SetupConnections() noexcept
	{
		m_OnMeshRendererComponentPropertyChangedConnection = ScopedConnection(CoreObjectBroadcasters::OnEntityComponentPropertyChanged,
			[this](entity aEntity, TypeIndex aComponentType, MAYBE_UNUSED IComponent* aComponent, uint64 aProperty)
			{
				if (aComponentType != MeshRendererComponent::StaticType())
					return;
				if (!IsEntityInspected(aEntity))
					return;
				if (aProperty == "m_MaterialHandle"_h || aProperty == "m_LightChannels"_h)
				{
					if (IDetailLayoutBuilder* pLayoutBuilder = GetDetailLayoutBuilder())
						pLayoutBuilder->ForceRefreshDetails();
				}
			});
	}
}