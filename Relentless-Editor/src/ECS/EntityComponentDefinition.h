#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class EEntityComponentFlags : uint8 
	{ 
		None = 0, 
		ShowInEditor = 1 << 0
	};
	DECLARE_BITMASK_TYPE(EEntityComponentFlags);

	struct EntityComponentDescriptor
	{
		String DisplayName			= "";
		String Category				= "Misc";
		String Description			= "Entity component type.";
		String Icon					= "";
		EEntityComponentFlags Flags = EEntityComponentFlags::None;
	};

	class IEntityComponentDefinition : public RefCounted<IEntityComponentDefinition>
	{
	public:
		IEntityComponentDefinition(const EntityComponentDescriptor& aDescriptor) noexcept;
		virtual ~IEntityComponentDefinition() = default;

		virtual void Add(EntityManager& aEntityManager, entity aEntity) const noexcept = 0;

		NO_DISCARD virtual bool CanShowInEditor() const noexcept;

		NO_DISCARD const String& GetCategory() const noexcept;
		NO_DISCARD const String& GetDescription() const noexcept;
		NO_DISCARD const String& GetDisplayName() const noexcept;
		NO_DISCARD const String& GetIcon() const noexcept;
		NO_DISCARD virtual TypeIndex GetTypeID() const noexcept = 0;

		NO_DISCARD virtual bool Has(EntityManager& aEntityManager, entity aEntity) const noexcept = 0;

		virtual void Remove(EntityManager& aEntityManager, entity aEntity) const noexcept = 0;
	private:
		EntityComponentDescriptor m_Descriptor;
	};

	template<typename ComponentType>
	class EntityComponentDefinition : public IEntityComponentDefinition
	{
	public:
		EntityComponentDefinition(const EntityComponentDescriptor& aDescriptor) noexcept;

		void Add(EntityManager& aEntityManager, entity aEntity) const noexcept override;

		NO_DISCARD virtual TypeIndex GetTypeID() const noexcept override;

		NO_DISCARD bool Has(EntityManager& aEntityManager, entity aEntity) const noexcept override;

		void Remove(EntityManager& aEntityManager, entity aEntity) const noexcept override;
	};

	template<typename ComponentType>
	EntityComponentDefinition<ComponentType>::EntityComponentDefinition(const EntityComponentDescriptor& aDescriptor) noexcept
		: IEntityComponentDefinition(aDescriptor)
	{}

	template<typename ComponentType>
	void EntityComponentDefinition<ComponentType>::Add(EntityManager& aEntityManager, entity aEntity) const noexcept
	{
		RLS_ASSERT(!Has(aEntityManager, aEntity), "[EntityComponentDefinition::Add]: Component already exists on entity.");
		aEntityManager.Add<ComponentType>(aEntity);
	}

	template<typename ComponentType>
	TypeIndex EntityComponentDefinition<ComponentType>::GetTypeID() const noexcept
	{
		static constexpr TypeIndex typeIndex = getTypeIndex<ComponentType>();
		return typeIndex;
	}

	template<typename ComponentType>
	bool EntityComponentDefinition<ComponentType>::Has(EntityManager& aEntityManager, entity aEntity) const noexcept
	{
		return aEntityManager.Has<ComponentType>(aEntity);
	}

	template<typename ComponentType>
	void EntityComponentDefinition<ComponentType>::Remove(EntityManager& aEntityManager, entity aEntity) const noexcept
	{
		RLS_ASSERT(Has(aEntityManager, aEntity), "[EntityComponentDefinition::Remove]: Component does not exist on entity.");
		aEntityManager.Remove<ComponentType>(aEntity);
	}
}