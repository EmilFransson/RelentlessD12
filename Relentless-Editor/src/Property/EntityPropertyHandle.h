#pragma once
#include "MultiPropertyHandle.h"

namespace Relentless
{
	template<typename DataType, typename ComponentType>
	class EntityPropertyHandle : public MultiPropertyHandle<DataType>
	{
	public:
		using MemberPtr = DataType ComponentType::*;
		using ComponentGetter = Callback<DataType(const ComponentType&)>;
		using ComponentSetter = Callback<void(ComponentType&, const DataType&)>;

		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember) noexcept;
		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember, Callback<DataType()> aDefaultGetter) noexcept;
		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember, const DataType& aDefaultValue) noexcept;

		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter) noexcept;
		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter, Callback<DataType()> aDefaultGetter) noexcept;
		EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter, const DataType& aDefaultValue) noexcept;
	};

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember) noexcept
		: MultiPropertyHandle<DataType>
		(
		[&someEntities, &aManager, aMember](uint32 aIndex) -> DataType
		{
			const entity e = someEntities[aIndex];
			const ComponentType& component = aManager.Get<ComponentType>(e);
			return component.*aMember;
		},
		[&someEntities, &aManager, aMember](const DataType& aNewValue, uint32 aIndex) noexcept
		{
			const entity e = someEntities[aIndex];
			ComponentType& component = aManager.Get<ComponentType>(e);
			component.*aMember = aNewValue;
		},
		static_cast<uint32>(someEntities.size())
		)
	{
	}

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember, Callback<DataType()> aDefaultGetter) noexcept
		: MultiPropertyHandle<DataType>
		(
			[&someEntities, &aManager, aMember](uint32 aIndex) -> DataType
			{
				const entity e = someEntities[aIndex];
				const ComponentType& component = aManager.Get<ComponentType>(e);
				return component.*aMember;
			},
			[&someEntities, &aManager, aMember](const DataType& aNewValue, uint32 aIndex) noexcept
			{
				const entity e = someEntities[aIndex];
				ComponentType& component = aManager.Get<ComponentType>(e);
				component.*aMember = aNewValue;
			},
			static_cast<uint32>(someEntities.size()),
			std::move(aDefaultGetter)
		)
	{
	}

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, MemberPtr aMember, const DataType& aDefaultValue) noexcept
		: MultiPropertyHandle<DataType>
		(
			[&someEntities, &aManager, aMember](uint32 aIndex) -> DataType
			{
				const entity e = someEntities[aIndex];
				const ComponentType& component = aManager.Get<ComponentType>(e);
				return component.*aMember;
			},
			[&someEntities, &aManager, aMember](const DataType& aNewValue, uint32 aIndex) noexcept
			{
				const entity e = someEntities[aIndex];
				ComponentType& component = aManager.Get<ComponentType>(e);
				component.*aMember = aNewValue;
			},
			static_cast<uint32>(someEntities.size()),
			aDefaultValue
		)
	{
	}

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter) noexcept
		: MultiPropertyHandle<DataType>
		(
			[&someEntities, &aManager, getter = std::move(aGetter)](uint32 aIndex) -> DataType
			{
				const entity e = someEntities[aIndex];
				const ComponentType& component = aManager.Get<ComponentType>(e);
				return getter(component);
			},
			[&someEntities, &aManager, setter = std::move(aSetter)](const DataType& aNewValue, uint32 aIndex) noexcept
			{
				const entity e = someEntities[aIndex];
				ComponentType& component = aManager.Get<ComponentType>(e);
				setter(component, aNewValue);
			},
			static_cast<uint32>(someEntities.size())
		)
	{
	}

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter, Callback<DataType()> aDefaultGetter) noexcept
		: MultiPropertyHandle<DataType>
		(
			[&someEntities, &aManager, getter = std::move(aGetter)](uint32 aIndex) -> DataType
			{
				const entity e = someEntities[aIndex];
				const ComponentType& component = aManager.Get<ComponentType>(e);
				return getter(component);
			},
			[&someEntities, &aManager, setter = std::move(aSetter)](const DataType& aNewValue, uint32 aIndex) noexcept
			{
				const entity e = someEntities[aIndex];
				ComponentType& component = aManager.Get<ComponentType>(e);
				setter(component, aNewValue);
			},
			static_cast<uint32>(someEntities.size()),
			std::move(aDefaultGetter))
	{
	}

	template<typename DataType, typename ComponentType>
	EntityPropertyHandle<DataType, ComponentType>::EntityPropertyHandle(EntityManager& aManager, std::vector<entity>& someEntities, ComponentGetter aGetter, ComponentSetter aSetter, const DataType& aDefaultValue) noexcept
		: MultiPropertyHandle<DataType>
		(
			[&someEntities, &aManager, getter = std::move(aGetter)](uint32 aIndex) -> DataType
			{
				const entity e = someEntities[aIndex];
				const ComponentType& component = aManager.Get<ComponentType>(e);
				return getter(component);
			},
			[&someEntities, &aManager, setter = std::move(aSetter)](const DataType& aNewValue, uint32 aIndex) noexcept
			{
				const entity e = someEntities[aIndex];
				ComponentType& component = aManager.Get<ComponentType>(e);
				setter(component, aNewValue);
			},
			static_cast<uint32>(someEntities.size()),
			aDefaultValue
		)
	{
	}
}