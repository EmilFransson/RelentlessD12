#pragma once
#include <Relentless.h>

#include "Property/EntityPropertyHandle.h"

#include "Subsystem/EngineContentSubsystem.h"

namespace Relentless::DetailHelpers
{
	template<typename ComponentType, typename TResolve>
	void MakeAssetTargetImpl(auto aPropertyGenerator, IDetailLayoutBuilder& aLayoutBuilder,StringView aName,
		std::vector<entity>& aEntities, EntityManager& aManager,
		Span<TypeIndex> someValidTypes, TResolve aResolve,
		auto aGetter, auto aSetter, auto aRemover)
	{
		if (aEntities.empty())
			return;

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();

		const AssetHandle heuristicHandle = std::invoke(aGetter, aResolve(aManager.Get<ComponentType>(aEntities.front())));
		const bool allSameAndValid = heuristicHandle.IsValid() &&
			std::ranges::all_of(aEntities, [&](const entity aEntity)
				{
					return std::invoke(aGetter, aResolve(aManager.Get<ComponentType>(aEntity))) == heuristicHandle;
				});

		AssetData* pAssetData = allSameAndValid
			? assetRegistry.FindAsset(heuristicHandle.Uuid)
			: assetRegistry.FindAsset(pEngineContentSubsystem->GetAssetHandle(EEngineAsset::NoneThumbnail).Uuid);

		auto* pEntities = &aEntities;
		auto* pManager = &aManager;

			aPropertyGenerator(aName.data(), *pAssetData)
			.AcceptableAssetTypes(someValidTypes)
			.OnAssetsDropped([pEntities, pManager, &aLayoutBuilder, aResolve, aSetter](Span<const AssetData> someAssetDatas)
				{
					const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
					for (const entity e : *pEntities)
						std::invoke(aSetter, aResolve(pManager->Get<ComponentType>(e)), assetHandle);

					Application::Get().SubmitToMainThread([&aLayoutBuilder]()
						{
							aLayoutBuilder.GetDetailsView()->Rebuild<EntityDetailsContext>();
						});
				})
			.NameSlot().Label(aName)
			.ValueSlot().AssetThumbnail()
			.Tooltip(allSameAndValid ? pAssetData->PackagePath.string() + pAssetData->Name : "").Row()
			.RevertSlot().Widget([pEntities, pManager, &aLayoutBuilder, allSameAndValid, aResolve, aRemover]()
				{
					Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();
					pRevertBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });

					if (allSameAndValid)
					{
						Button* pButton = pRevertBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
						pButton->SetTextColor(Colors::TextInactive);
						pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
						pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
						pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
						pButton->OnClicked([pEntities, pManager, &aLayoutBuilder, aResolve, aRemover]()
							{
								for (const entity e : *pEntities)
									std::invoke(aRemover, aResolve(pManager->Get<ComponentType>(e)));

								Application::Get().SubmitToMainThread([&aLayoutBuilder]()
									{
										aLayoutBuilder.GetDetailsView()->Rebuild<EntityDetailsContext>();
									});
							});
					}
					return pRevertBox;
				});
	}

	template<typename ComponentType, typename TProjection>
	struct SubHandleFactory
	{
		std::vector<entity>& Entities;
		EntityManager& EntityManager;
		TProjection Projection;

		template<typename T>
		NO_DISCARD Ref<EntityPropertyHandle<T, ComponentType>> Make(auto aGetter, auto aSetter, T aDefault) noexcept
		{
			return RLS_NEW EntityPropertyHandle<T, ComponentType>
				(
				EntityManager, Entities,
				[proj = Projection, aGetter](const ComponentType& aComponent) -> T { return (proj(aComponent).*aGetter)(); },
				[proj = Projection, aSetter](entity, ComponentType& aComponent, const T& aValue) { (proj(aComponent).*aSetter)(aValue); },
				aDefault
				);
		}

		void MakeAssetTarget(IDetailCategoryBuilder& aBuilder, StringView aName, Span<TypeIndex> someValidTypes, auto aGetter, auto aSetter, auto aRemover)
		{
			MakeAssetTargetImpl<ComponentType>([&aBuilder](const char* aName, const AssetData& aAssetData) { return aBuilder.AddAssetProperty(aName, aAssetData);  },
				aBuilder.GetLayoutBuilder(), aName, Entities, EntityManager, someValidTypes, Projection, aGetter, aSetter, aRemover);
		}
	};

	template<typename ComponentType>
	struct EntityHandleFactory
	{
		std::vector<entity>& Entities;
		EntityManager& EntityManager;

		template<typename T>
		NO_DISCARD Ref<EntityPropertyHandle<T, ComponentType>> Make(auto aGetter, auto aSetter, T aDefault) noexcept
		{
			return RLS_NEW EntityPropertyHandle<T, ComponentType>
				(
					EntityManager, 
					Entities, 
					[aGetter](const ComponentType& aComponent) { return (aComponent.*aGetter)(); },
					[aSetter](entity, ComponentType& aComponent, const T& aValue) { return (aComponent.*aSetter)(aValue); },
					aDefault
					);
		}

		void MakeAssetTarget(IDetailCategoryBuilder& aBuilder, StringView aName, Span<TypeIndex> someValidTypes, auto aGetter, auto aSetter, auto aRemover)
		{
				MakeAssetTargetImpl<ComponentType>([&aBuilder](const char* aName, const AssetData& aAssetData) { return aBuilder.AddAssetProperty(aName, aAssetData); },
				aBuilder.GetLayoutBuilder(),
				aName, Entities, EntityManager, someValidTypes,
				[](auto& aComponent) -> auto& { return aComponent; },
				aGetter, aSetter, aRemover);
		}

		void MakeAssetTarget(IDetailGroupBuilder& aBuilder, StringView aName, Span<TypeIndex> someValidTypes, auto aGetter, auto aSetter, auto aRemover)
		{
			MakeAssetTargetImpl<ComponentType>([&aBuilder](const char* aName, const AssetData& aAssetData) { return aBuilder.AddAssetProperty(aName, aAssetData); },
				aBuilder.GetCategoryBuilder().GetLayoutBuilder(), aName, Entities, EntityManager, someValidTypes,
				[](auto& aComponent) -> auto& { return aComponent; },
				aGetter, aSetter, aRemover);
		}

		//void MakeAssetTarget(IDetailCategoryBuilder& aBuilder, StringView aName, Span<TypeIndex> someValidTypes, auto aGetter, auto aSetter, auto aRemover)
		//{
		//	AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		//	EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();
		//
		//	const AssetHandle heuristicHandle = (EntityManager.Get<ComponentType>(Entities.front()).*aGetter)();
		//	const bool allSameAndValid = heuristicHandle.IsValid() && std::ranges::all_of(Entities, [this, &heuristicHandle, aGetter](const entity aEntity)
		//		{
		//			return (EntityManager.Get<ComponentType>(aEntity).*aGetter)() == heuristicHandle;
		//		});
		//
		//	AssetData* pAssetData = allSameAndValid ? assetRegistry.FindAsset(heuristicHandle.Uuid) : assetRegistry.FindAsset(pEngineContentSubsystem->GetAssetHandle(EEngineAsset::NoneThumbnail).Uuid);
		//	auto pEntityManager = &EntityManager;
		//	auto pEntities = &Entities;
		//
		//	aBuilder.AddAssetProperty(aName.data(), *pAssetData)
		//		.AcceptableAssetTypes(someValidTypes)
		//		.OnAssetsDropped([pEntities, pEntityManager, &aBuilder, aSetter](Span<const AssetData> someAssetDatas)
		//			{
		//				const AssetHandle assetHandle = AssetManager::LoadAsset(someAssetDatas[0]);
		//				std::ranges::for_each(*pEntities, [pEntityManager, &assetHandle, aSetter](const entity aEntity)
		//					{
		//						(pEntityManager->Get<ComponentType>(aEntity).*aSetter)(assetHandle);
		//					});
		//
		//				Application::Get().SubmitToMainThread([&aBuilder]()
		//					{
		//						aBuilder.GetLayoutBuilder().GetDetailsView()->Rebuild<EntityDetailsContext>();
		//					});
		//			})
		//		.NameSlot().Label(aName)
		//		.ValueSlot().AssetThumbnail().Tooltip(allSameAndValid ? pAssetData->PackagePath.string() + pAssetData->Name : "").Row()
		//		.RevertSlot().Widget([pEntities, pEntityManager, &aBuilder, allSameAndValid, aRemover]()
		//			{
		//				Ref<HorizontalBox> pRevertBox = RLS_NEW HorizontalBox();
		//				pRevertBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
		//
		//				if (allSameAndValid)
		//				{
		//					Button* pButton = pRevertBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
		//					pButton->SetTextColor(Colors::TextInactive);
		//					pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		//
		//					pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		//					pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
		//					pButton->OnClicked([pEntities, pEntityManager, &aBuilder, aRemover]()
		//						{
		//							std::ranges::for_each(*pEntities, [pEntityManager, aRemover](const entity aEntity)
		//								{
		//									(pEntityManager->Get<ComponentType>(aEntity).*aRemover)();
		//								});
		//
		//							Application::Get().SubmitToMainThread([&aBuilder]()
		//								{
		//									aBuilder.GetLayoutBuilder().GetDetailsView()->Rebuild<EntityDetailsContext>();
		//								});
		//						});
		//				}
		//
		//				return pRevertBox;
		//			});
		//}

		template<typename T, typename TGetter, typename TSetter>
		NO_DISCARD Ref<EntityPropertyHandle<T, ComponentType>> MakeCustom(TGetter&& aGetter, TSetter&& aSetter, T aDefault = {}) noexcept
		{
			return RLS_NEW EntityPropertyHandle<T, ComponentType>(
				EntityManager, Entities,
				std::forward<TGetter>(aGetter),
				std::forward<TSetter>(aSetter),
				aDefault);
		}

		template<typename TProjection>
		auto MakeSubFactory(TProjection aProjection) noexcept
		{
			return SubHandleFactory<ComponentType, TProjection>{ .Entities = Entities, .EntityManager = EntityManager, .Projection = aProjection };
		}
	};


}