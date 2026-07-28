#pragma once
#include <Relentless.h>

#include "Property/EntityPropertyHandle.h"

namespace Relentless::DetailHelpers
{
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