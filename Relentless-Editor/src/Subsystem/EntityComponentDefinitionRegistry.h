#pragma once
#include "ECS/EntityComponentDefinition.h"

namespace Relentless
{
	class EntityComponentDefinitionRegistry : public ISubsystem
	{
	public:
		NO_DISCARD std::vector<Ref<IEntityComponentDefinition>> GetAllComponentDefinitions() const noexcept;

		NO_DISCARD bool Exists(TypeIndex aType) const noexcept;

		template<typename ComponentType>
		NO_DISCARD Ref<IEntityComponentDefinition> GetDefinition() const noexcept;
		NO_DISCARD Ref<IEntityComponentDefinition> GetDefinition(TypeIndex aType) const noexcept;

		NO_DISCARD virtual bool OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;

		template<typename ComponentType>
		void Register(const EntityComponentDescriptor& aDescriptor) noexcept;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		template<typename ComponentType>
		void Unregister() noexcept;
	private:
		std::unordered_map<TypeIndex, Ref<IEntityComponentDefinition>> m_ComponentDefinitions;
	};

	template<typename ComponentType>
	Ref<IEntityComponentDefinition> EntityComponentDefinitionRegistry::GetDefinition() const noexcept
	{
		constexpr TypeIndex typeID = getTypeIndex<ComponentType>();
		return GetDefinition(typeID);
	}

	template<typename ComponentType>
	void EntityComponentDefinitionRegistry::Register(const EntityComponentDescriptor& aDescriptor) noexcept
	{
		constexpr TypeIndex typeID = getTypeIndex<ComponentType>();

		if (m_ComponentDefinitions.contains(typeID))
			return;

		m_ComponentDefinitions.emplace(typeID, RLS_NEW EntityComponentDefinition<ComponentType>(aDescriptor));
	}

	template<typename ComponentType>
	void EntityComponentDefinitionRegistry::Unregister() noexcept
	{
		constexpr TypeIndex typeID = getTypeIndex<ComponentType>();

		if (!m_ComponentDefinitions.contains(typeID))
			return;

		m_ComponentDefinitions.erase(typeID);
	}

}