#pragma once
#include "Component.h"
#include "System.h"
#include <StaticTypeInfo/type_id.h>
#include <StaticTypeInfo/type_index.h>
#include <StaticTypeInfo/type_name.h>

namespace Relentless
{
#define set_internal(ID) (static_cast<SparseSet<ComponentType>*>(m_components.at(ID).get()))

#if defined RLS_DEBUG
	#ifndef ECS_DEBUG_OP
		#define ECS_DEBUG_OP(lambda) lambda();
	#endif
	#ifndef ECS_DEBUG_EXPR
		#define ECS_DEBUG_EXPR(expr) expr
	#endif
#else
	#ifndef ECS_DEBUG_OP
		#define ECS_DEBUG_OP(lambda) 
	#endif
	#ifndef ECS_DEBUG_EXPR
		#define ECS_DEBUG_EXPR(expr)
	#endif
#endif

#if defined NDEBUG
#ifndef ASSERT
#define ASSERT(expression, errorString) 
#endif
#else
#ifndef ASSERT
#define ASSERT(expression, errorString) assert(expression && errorString)
#endif
#endif

	#define sti static_type_info
	
	struct SparseSetBase;

	template<typename... ComponentType>
	class Collection;

	class BundleBase;
	template<typename... ComponentType>
	class BundleImpl;

	typedef std::unique_ptr<SparseSetBase> ComponentPool;

	//##################### SPARSE SET #####################

	struct SparseSetBase
	{
		SparseSetBase() noexcept = default;
		virtual ~SparseSetBase() noexcept = default;
		virtual void DestroyInternal(const entity entityID) noexcept = 0;

		std::vector<entity> sparseArray;
		std::vector<entity> denseArray;
		sti::TypeIndex bundle = nullptr;
	};

	template<typename ComponentType>
	struct SparseSet : public SparseSetBase
	{
		SparseSet() noexcept = default;
		virtual ~SparseSet() noexcept override final = default;

		std::vector<ComponentType> components;
	private:
		virtual void DestroyInternal(const entity entityID) noexcept override final;
	};

	template<typename ComponentType>
	void SparseSet<ComponentType>::DestroyInternal(const entity entityID) noexcept
	{
		std::swap(components.back(), components[sparseArray[entityID]]);
		components.pop_back();
	}

	//##################### ENTITY MANAGER #####################

	class EntityManager
	{
	public:
		entity CreateEntity() noexcept;
		void DestroyEntity(const entity entityID) noexcept;
		[[nodiscard]] const std::vector<entity>& GetAllEntities() const noexcept;
		[[nodiscard]] uint32_t GetNrOfEntities() const noexcept { return MAX_ENTITIES - (uint32_t)m_freeList.size(); }
		void Reset() noexcept;
		[[nodiscard]] bool Exists(const entity entityID) const noexcept;
		[[nodiscard]] const std::unordered_map<sti::TypeIndex, ComponentPool>& GetAllComponentPools() const noexcept { return m_components; }

		template<typename ComponentType, typename ...Args>
		ComponentType& Add(const entity entityID, Args&& ...args) noexcept;

		template<typename ComponentType>
		void Remove(const entity entityID) noexcept;

		template<typename ComponentType>
		[[nodiscard]] ComponentType& Get(const entity entityID) const noexcept;

		template<typename... ComponentType>
		[[nodiscard]] Collection<ComponentType...> Collect() noexcept;

		template<typename ComponentType>
		[[nodiscard]] bool Has(const entity entityID) const noexcept;

		template<typename... ComponentType>
		[[nodiscard]] bool HasAllOf(const entity entityID) const noexcept;

		template<typename... ComponentType>
		[[nodiscard]] bool HasAnyOf(const entity entityID) const noexcept;

		template<typename... ComponentType>
		[[nodiscard]] BundleImpl<ComponentType...>& Bundle() noexcept;

		void RegisterSystem(std::unique_ptr<ISystem>&& pSystem) noexcept;

		[[nodiscard]] constexpr const std::vector<std::unique_ptr<ISystem>>::const_iterator begin() const { return m_systems.begin(); }
		[[nodiscard]] constexpr const std::vector<std::unique_ptr<ISystem>>::const_iterator end() const { return m_systems.end(); }

		EntityManager() noexcept;
		~EntityManager() noexcept = default;
	private:
		void Initialize() noexcept;

		[[nodiscard]] bool HasComponentInternal(const sti::TypeIndex componentPoolIndex, const entity entityID) const noexcept;
		void DestroyComponentInternal(const sti::TypeIndex componentPoolIndex, const entity entityID) noexcept;

		template<typename ComponentType>
		void ValidateComponentPool() noexcept;

		template<typename ComponentType> 
		void AddSparseSet() noexcept;
	public:
		template<typename ComponentType>
		SparseSet<ComponentType>* ExpandAsTupleArguments() noexcept;
	private:
		template<typename... ComponentType>
		friend class Collection;
		template<typename... ComponentType>
		friend class BundleImpl;
		friend class ISystem;

		std::vector<entity> m_entities;
		std::queue<entity> m_freeList;
		std::unordered_map<sti::TypeIndex ,ComponentPool> m_components;
		std::unordered_map<sti::TypeIndex, std::unique_ptr<BundleBase>> m_bundles;
		std::vector<std::unique_ptr<ISystem>> m_systems;
	};

	template<typename ComponentType>
	SparseSet<ComponentType>* EntityManager::ExpandAsTupleArguments() noexcept
	{
		static constexpr auto componentID = sti::getTypeIndex<ComponentType>();
		ValidateComponentPool<ComponentType>();
		
		return static_cast<SparseSet<ComponentType>*>(m_components.at(componentID).get());
	}

	template<typename... ComponentType>
	BundleImpl<ComponentType...>& EntityManager::Bundle() noexcept
	{
		static constexpr std::array<sti::TypeIndex, sizeof... (ComponentType)> componentIDs{ sti::getTypeIndex<ComponentType>()... };
		constexpr sti::TypeIndex minComponentID = *std::min_element(std::begin(componentIDs), std::end(componentIDs));

		if (m_bundles[minComponentID] == nullptr)
		{
			std::tuple<SparseSet<ComponentType>*...> pools = { (ExpandAsTupleArguments<ComponentType>()) ...};

			ECS_DEBUG_EXPR(std::apply([](const auto&... pool) {(ASSERT((pool != nullptr) && pool->bundle == nullptr, "Bundle creation failed."), ...); }, pools););
			std::apply([&minComponentID](const auto... pool) {((pool->bundle = (sti::TypeIndex)minComponentID), ...); }, pools);
			
			m_bundles[minComponentID] = (std::unique_ptr<BundleImpl<ComponentType...>>(RLS_NEW BundleImpl<ComponentType...>(this, std::move(pools))));
		}
		return *(static_cast<BundleImpl<ComponentType...>*>(m_bundles[minComponentID].get()));
	}

	template<typename ComponentType, typename ...Args>
	ComponentType& EntityManager::Add(const entity entityID, Args&& ...args) noexcept
	{
		static constexpr auto ComponentID = sti::getTypeIndex<ComponentType>();

		RLS_ASSERT(Exists(entityID), "Entity is invalid");
		RLS_ASSERT(!Has<ComponentType>(entityID), "Entity already has component!");

		ValidateComponentPool<ComponentType>();

		auto& pool = static_cast<SparseSet<ComponentType>&>(*m_components.at(ComponentID));

		const size_t position = pool.denseArray.size();
		pool.denseArray.emplace_back(entityID);
		pool.components.emplace_back(ComponentType(std::forward<Args>(args)...));
		pool.sparseArray[entityID] = static_cast<entity>(position);

		if (pool.bundle != nullptr)
		{
			auto componentIdx = m_bundles[pool.bundle]->UpdateOnAdd(entityID);
			if (componentIdx)
			{
				return pool.components[*componentIdx];
			}
		}
		return pool.components.back();
	}

	template<typename ComponentType>
	void EntityManager::Remove(const entity entityID) noexcept
	{
		static auto constexpr componentID = sti::getTypeIndex<ComponentType>();

		RLS_ASSERT(Exists(entityID), "Entity is invalid");
		RLS_ASSERT(Has<ComponentType>(entityID), "Entity does not have that component.");
		auto& pool = static_cast<SparseSet<ComponentType>&>(*m_components.at(componentID));

		if (pool.bundle != nullptr)
		{
			m_bundles[pool.bundle]->UpdateOnRemove(entityID);
		}

		const auto last = pool.denseArray.back();
		std::swap(pool.denseArray.back(), pool.denseArray[pool.sparseArray[entityID]]);
		std::swap(pool.components.back(), pool.components[pool.sparseArray[entityID]]);
		std::swap(pool.sparseArray[last], pool.sparseArray[entityID]);
		pool.denseArray.pop_back();
		pool.components.pop_back();
		pool.sparseArray[entityID] = NULL_ENTITY;
	}

	template<typename ComponentType>
	ComponentType& EntityManager::Get(const entity entityID) const noexcept
	{
		static auto constexpr componentID = sti::getTypeIndex<ComponentType>();
		RLS_ASSERT(Exists(entityID), "Entity is invalid");
		RLS_ASSERT(Has<ComponentType>(entityID), "Entity does not have that component.");

		auto& pool = static_cast<SparseSet<ComponentType>&>(*m_components.at(componentID));
		return pool.components[pool.sparseArray[entityID]];
	}

	template<typename... ComponentType>
	Collection<ComponentType...> EntityManager::Collect() noexcept
	{
		return Collection<ComponentType...>( this, { ExpandAsTupleArguments<ComponentType>() ...});
	}

	template<typename ComponentType> 
	bool EntityManager::Has(const entity entityID) const noexcept
	{
		static auto constexpr componentID = sti::getTypeIndex<ComponentType>();
		RLS_ASSERT(Exists(entityID), "Entity is invalid");

		const bool componentTypeExists = m_components.contains(componentID);
		if (!componentTypeExists)
			return false;

		auto& pool = static_cast<SparseSet<ComponentType>&>(*m_components.at(componentID));
		
		return 
			( entityID < pool.sparseArray.size())
			&& (pool.sparseArray[entityID] < pool.denseArray.size())
			&& (pool.sparseArray[entityID] != NULL_ENTITY
			);
	}

	template<typename... ComponentType>
	bool EntityManager::HasAllOf(const entity entityID) const noexcept
	{
		return (EntityManager::Has<ComponentType>(entityID) && ...);
	}

	template<typename... ComponentType>
	bool EntityManager::HasAnyOf(const entity entityID) const noexcept
	{
		return (EntityManager::Has<ComponentType>(entityID) || ...);
	}

	template<typename ComponentType>
	void EntityManager::ValidateComponentPool() noexcept
	{
		static constexpr auto ComponentID = sti::getTypeIndex<ComponentType>();
		if (!m_components.contains(ComponentID))
		{
			AddSparseSet<ComponentType>();
		}
	}

	template<typename ComponentType> 
	void EntityManager::AddSparseSet() noexcept
	{
		static constexpr auto ComponentID = sti::getTypeIndex<ComponentType>();
		m_components[ComponentID] = std::move(std::make_unique<SparseSet<ComponentType>>());
		auto& pool = static_cast<SparseSet<ComponentType>&>(*m_components.at(ComponentID));

		pool.sparseArray.reserve(MAX_ENTITIES);
		for (uint32_t i{ 0u }; i < MAX_ENTITIES; i++)
		{
			pool.sparseArray.push_back(NULL_ENTITY);
		}

		pool.denseArray.reserve(MAX_ENTITIES);
		pool.components.reserve(MAX_ENTITIES);
	}

	//##################### COLLECTIONS #####################

	template<typename... ComponentType>
	class Collection
	{
	public:
		explicit Collection(EntityManager* mgr, const std::tuple<SparseSet<ComponentType>*...>& pools) noexcept;
		~Collection() noexcept = default;

		void Do(std::invocable<ComponentType&...> auto&& func) const noexcept;
		void Do(std::invocable<entity, ComponentType&...> auto&& func) const noexcept;

	private:
		DELETE_COPY_MOVE_CONSTRUCTOR(Collection);
		friend class EntityManager;
		std::tuple<SparseSet<ComponentType>*...> m_pools;
		EntityManager* m_mgr;
	};

	template<typename... ComponentType>
	Collection<ComponentType...>::Collection(EntityManager* mgr, const std::tuple<SparseSet<ComponentType>*...>& pools) noexcept
		: m_mgr{ mgr }, m_pools(std::move(pools))
	{
		ECS_DEBUG_EXPR(std::apply([](const auto&... pool) {(ASSERT(pool, "Component type does not exist."), ...); }, m_pools););
	}

	template<typename... ComponentType>
	void Collection<ComponentType...>::Do(std::invocable<ComponentType&...> auto&& func) const noexcept
	{
		std::vector<entity>* ePointer = &(get<0>(m_pools)->denseArray);
		std::apply([&ePointer](const auto&... pool)
			{
				((ePointer = (pool->denseArray.size() < ePointer->size()) ? &pool->denseArray : ePointer), ...);
			}, m_pools);

		auto&& EntityHasAllComponents = [&](const entity entityID)
		{
			bool hasAllComponents = true;
			std::apply([&](const auto&... pool)
				{
					((hasAllComponents &= ((entityID < pool->sparseArray.size())
						&& (pool->sparseArray[entityID] < pool->denseArray.size())
						&& (pool->sparseArray[entityID] != NULL_ENTITY))), ...);

				}, m_pools);
			return hasAllComponents;
		};

		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)
		{
			//m_mgr->HasAllOf<ComponentType...>((*ePointer)[i])
			if (EntityHasAllComponents((*ePointer)[i]))
			{
				//std::apply([&](const auto&... pool)
				//	{
				//		func(pool->components[(*ePointer)[i]] ...);
				//	}, m_pools);
				func(m_mgr->Get<ComponentType>((*ePointer)[i])...);
			}
		}
	}

	template<typename... ComponentType>
	void Collection<ComponentType...>::Do(std::invocable<entity, ComponentType&...> auto&& func) const noexcept
	{
		std::vector<entity>* ePointer = &(get<0>(m_pools)->denseArray);
		std::apply([&ePointer](const auto&... pool)
			{
				((ePointer = (pool->denseArray.size() < ePointer->size()) ? &pool->denseArray : ePointer), ...);
			}, m_pools);

		auto&& EntityHasAllComponents = [&](entity entityID) -> bool
		{
			bool hasAllComponents = true;
			std::apply([&](const auto&... pool)
				{
					((hasAllComponents &= ((entityID < pool->sparseArray.size())
						&& (pool->sparseArray[entityID] < pool->denseArray.size())
						&& (pool->sparseArray[entityID] != NULL_ENTITY))), ...);
				}, m_pools);
			return hasAllComponents;
		};

		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)
		{
			//if (m_mgr->HasAllOf<ComponentType...>((*ePointer)[i]))
			if (EntityHasAllComponents((*ePointer)[i]))
			{
				//std::apply([&](const auto&... pool)
				//	{
				//		func((*ePointer)[i], pool->components[(*ePointer)[i]] ...);
				//	}, m_pools);
				func((*ePointer)[i], m_mgr->Get<ComponentType>((*ePointer)[i])...);
			}
		}
	}

	//##################### BUNDLES #####################

	class BundleBase
	{
	public:
		BundleBase() noexcept = default;
		virtual ~BundleBase() noexcept = default;

		virtual [[nodiscard]] std::optional<uint32_t> UpdateOnAdd(const entity entityID) noexcept = 0;
		virtual void UpdateOnRemove(const entity entityID) noexcept = 0;
		friend ISystem;
	};

	template<typename... ComponentType>
	class BundleImpl : public BundleBase
	{
	public:
		explicit BundleImpl(EntityManager* mgr, const std::tuple<SparseSet<ComponentType>*...>& pools) noexcept;
		virtual ~BundleImpl() noexcept override final = default;

		void Do(std::invocable<ComponentType&...> auto&& func) const noexcept;
		void Do(std::invocable<entity, ComponentType&...> auto&& func) const noexcept;

		[[nodiscard]] constexpr const std::vector<entity>* GetEntityVectorPointer() const noexcept { return ePointer; }
		[[nodiscard]] constexpr const int* GetBundleStartPointer() const noexcept { return &m_bundleStart; }
	private:
		DELETE_COPY_MOVE_CONSTRUCTOR(BundleImpl);
		[[nodiscard]] std::optional<uint32_t> UpdateOnAdd(const entity entityID) noexcept override final;
		virtual void UpdateOnRemove(const entity entityID) noexcept override final;
	private:
		friend EntityManager;
		friend ISystem;
		EntityManager* m_mgr;
		std::tuple<SparseSet<ComponentType>*...> m_pools;
		std::vector<entity>* ePointer;
		int m_bundleStart;
	};

	template<typename... ComponentType>
	BundleImpl<ComponentType...>::BundleImpl(EntityManager* mgr, const std::tuple<SparseSet<ComponentType>*...>& pools) noexcept
		: m_mgr{ mgr }, m_pools{ std::move(pools) }, m_bundleStart{ -1 }, ePointer{ nullptr }
	{
		ECS_DEBUG_EXPR(std::apply([](const auto... pool) {(ASSERT(pool, "Component type does not exist."), ...); }, m_pools);)
		
		ePointer = &(get<0>(m_pools)->denseArray);
		std::apply([&](const auto... pool)
			{
				((ePointer = (pool->denseArray.size() < ePointer->size()) ? &pool->denseArray : ePointer), ...);
			}, m_pools);

		const auto&& myLambda = []<typename ComponentType>(SparseSet<ComponentType>*pool, uint32_t index, int bundleStart)
		{
			std::swap(pool->denseArray[index], pool->denseArray[bundleStart + 1]);
			std::swap(pool->components[index], pool->components[bundleStart + 1]);
			std::swap(pool->sparseArray[pool->denseArray[index]], pool->sparseArray[pool->denseArray[bundleStart + 1]]);
		};

		for (size_t i{ 0u }; i < ePointer->size(); ++i)
		{
			if ((m_mgr->Has<ComponentType>((*ePointer)[i]) && ...))
			{
				std::apply([&](const auto... pool)
					{
						(myLambda(pool, pool->sparseArray[(*ePointer)[i]], m_bundleStart), ...);
					}, m_pools);

				m_bundleStart++;
			}
		}
	}

	template<typename... ComponentType>
	void BundleImpl<ComponentType...>::Do(std::invocable<ComponentType&...> auto&& func) const noexcept
	{
		for (int i{ m_bundleStart }; i >= 0; --i)
		{
			func(m_mgr->Get<ComponentType>((*ePointer)[i])...);
		}
	}

	template<typename... ComponentType>
	void BundleImpl<ComponentType...>::Do(std::invocable<entity, ComponentType&...> auto&& func) const noexcept
	{
		for (int i{ m_bundleStart }; i >= 0; --i)
		{
			func((*ePointer)[i], m_mgr->Get<ComponentType>((*ePointer)[i])...);
		}
	}

	template<typename... ComponentType>
	std::optional<uint32_t> BundleImpl<ComponentType...>::UpdateOnAdd(const entity entityID) noexcept
	{
		const auto&& SwapLambda = []<typename ComponentType>(SparseSet<ComponentType>* pool, uint32_t index, int bundleStart)
		{
			std::swap(pool->denseArray[index], pool->denseArray[bundleStart + 1]);
			std::swap(pool->components[index], pool->components[bundleStart + 1]);
			std::swap(pool->sparseArray[pool->denseArray[index]], pool->sparseArray[pool->denseArray[bundleStart + 1]]);
		};

		if (m_mgr->HasAllOf<ComponentType...>(entityID))
		{
			std::apply([&](const auto... pool)
				{
					(SwapLambda(pool, pool->sparseArray[entityID], m_bundleStart), ...);
				}, m_pools);

			m_bundleStart++;
			return m_bundleStart;
		}
		return std::nullopt;
	}

	template<typename... ComponentType>
	void BundleImpl<ComponentType...>::UpdateOnRemove(const entity entityID) noexcept
	{
		const auto&& SwapLambda = []<typename ComponentType>(SparseSet<ComponentType>* pool, uint32_t index, int bundleStart)
		{
			std::swap(pool->denseArray[index], pool->denseArray[bundleStart]);
			std::swap(pool->components[index], pool->components[bundleStart]);
			std::swap(pool->sparseArray[pool->denseArray[index]], pool->sparseArray[pool->denseArray[bundleStart]]);
		};

		if (m_mgr->HasAllOf<ComponentType...>(entityID))
		{
			std::apply([&](const auto... pool)
				{
					(SwapLambda(pool, pool->sparseArray[entityID], m_bundleStart), ...);
				}, m_pools);

			m_bundleStart--;
		}
	}

	
}

//##################### SYSTEMS #####################

	template<typename... ComponentType>
	struct SystemHelper
	{
		SystemHelper() noexcept
			: m_mgr{ RLS::EntityManager::Get() }, m_pools{ RLS::EntityManager::Get().ExpandAsTupleArguments<ComponentType>() ... } {}

		inline [[nodiscard]] std::vector<RLS::entity>* GetMinimumEntityVector() const
		{
			std::vector<RLS::entity>* ePointer = &(get<0>(m_pools)->denseArray);
			std::apply([&ePointer](const auto&... pool)
				{
					((ePointer = (pool->denseArray.size() < ePointer->size()) ? &pool->denseArray : ePointer), ...);
				}, m_pools);

			return ePointer;
		}
		DELETE_COPY_MOVE_CONSTRUCTOR(SystemHelper);
		RLS::EntityManager& m_mgr;
		std::tuple<RLS::SparseSet<ComponentType>* ...> m_pools;
	};

	template<typename... ComponentType>
	struct CriticalSystemHelper
	{
		CriticalSystemHelper() noexcept
			: m_mgr{ RLS::EntityManager::Get() }
		{
			static_assert(sizeof...(ComponentType) > 1);
			auto& bundle = RLS::EntityManager::Get().Bundle<ComponentType...>();
			m_ePointer = bundle.GetEntityVectorPointer();
			m_bundleStart = bundle.GetBundleStartPointer();
		}
		DELETE_COPY_MOVE_CONSTRUCTOR(CriticalSystemHelper);
		RLS::EntityManager& m_mgr;
		const std::vector<RLS::entity>* m_ePointer;
		const int* m_bundleStart;
	};

#if defined RLS_DEBUG																													
#define SYSTEM_CLASS(...)																								\
public:																															\
		SystemHelper<__VA_ARGS__> m_systemHelper;																				\
		[[nodiscard]] virtual std::string_view GetName() const override															\
		{																														\
			constexpr std::string_view s = __FUNCTION__;																		\
			constexpr std::string_view s2 = s.substr(s.find_first_not_of("DOG::"));												\
			constexpr size_t lastIndex = s2.find_first_of(":");																	\
			constexpr std::string_view finalName = s2.substr(0, lastIndex);														\
			return finalName;																									\
		}																														\
		[[nodiscard]] virtual RLS::SystemType GetType() const noexcept															\
		{																														\
			return RLS::SystemType::Standard;																						\
		}																														
#else																															
#define SYSTEM_CLASS(...)																									\
	SystemHelper<__VA_ARGS__> m_systemHelper;
#endif																															


#if defined RLS_DEBUG																													
#define SYSTEM_CLASS_CRITICAL(...)																								\
		CriticalSystemHelper<__VA_ARGS__> m_systemHelper;																		\
		[[nodiscard]] virtual std::string_view GetName() const override															\
		{																														\
			constexpr std::string_view s = __FUNCTION__;																		\
			constexpr std::string_view s2 = s.substr(s.find_first_not_of("DOG::"));												\
			constexpr size_t lastIndex = s2.find_first_of(":");																	\
			constexpr std::string_view finalName = s2.substr(0, lastIndex);														\
			return finalName;																									\
		}																														\
		[[nodiscard]] virtual RLS::SystemType GetType() const noexcept															\
		{																														\
			return RLS::SystemType::Critical;																						\
		}
#else																															
#define SYSTEM_CLASS_CRITICAL(...)																								\
	CriticalSystemHelper<__VA_ARGS__> m_systemHelper;
#endif

	/*ON_CREATE*/

#ifndef ON_CREATE
#define ON_CREATE(...)																										\
	void Create() noexcept override final																					\
	{																														\
		CreateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void CreateImpl()																										\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnCreate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);								\
			}																												\
		}																													\
	}
#endif

#ifndef ON_CREATE_ID
#define ON_CREATE_ID(...)																									\
	void Create() noexcept override final																					\
	{																														\																										\
			CreateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void CreateImpl()																										\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnCreate((*ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);				\
			}																												\
		}																													\
	}
#endif

#ifndef ON_CREATE_CRITICAL
#define ON_CREATE_CRITICAL(...)																								\
	void Create() noexcept override final																					\
	{																														\
		CreateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void CreateImpl()																										\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnCreate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);					\
		}																													\
	}
#endif

#ifndef ON_CREATE_CRITICAL_ID
#define ON_CREATE_CRITICAL_ID(...)																							\
	void Create() noexcept override final																					\
	{																														\
		CreateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void CreateImpl()																										\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnCreate((*m_systemHelper.m_ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);					\
		}																													\
	}
#endif

	/*ON_EARLY_UPDATE*/

#ifndef ON_EARLY_UPDATE
#define ON_EARLY_UPDATE(...)																								\
	void EarlyUpdate() noexcept override final																				\
	{																														\
		EarlyUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void EarlyUpdateImpl()																									\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnEarlyUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);						\
			}																												\
		}																													\
	}
#endif

#ifndef ON_EARLY_UPDATE_ID
#define ON_EARLY_UPDATE_ID(...)																								\
	void EarlyUpdate() noexcept override final																				\
	{																														\
		EarlyUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void EarlyUpdateImpl()																									\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnEarlyUpdate((*ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);		\
			}																												\
		}																													\
	}
#endif

#ifndef ON_EARLY_UPDATE_CRITICAL
#define ON_EARLY_UPDATE_CRITICAL(...)																						\
	void EarlyUpdate() noexcept override final																				\
	{																														\
		EarlyUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void EarlyUpdateImpl()																									\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnEarlyUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);			\
		}																													\
	}
#endif

#ifndef ON_EARLY_UPDATE_CRITICAL_ID
#define ON_EARLY_UPDATE_CRITICAL_ID(...)																					\
	void EarlyUpdate() noexcept override final																				\
	{																														\
		EarlyUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void EarlyUpdateImpl()																									\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnEarlyUpdate((*m_systemHelper.m_ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);			\
		}																													\
	}
#endif

	/*ON_UPDATE*/

#ifndef ON_UPDATE
#define ON_UPDATE(...)																										\
	void Update() noexcept override final																					\
	{																														\
		UpdateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void UpdateImpl()																										\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);								\
			}																												\
		}																													\
	}
#endif

#ifndef ON_UPDATE_ID
#define ON_UPDATE_ID(...)																									\
	void Update() noexcept override final																					\
	{																														\
		UpdateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void UpdateImpl()																										\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnUpdate((*ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);				\
			}																												\
		}																													\
	}
#endif

#ifndef ON_UPDATE_CRITICAL
#define ON_UPDATE_CRITICAL(...)																								\
	void Update() noexcept override final																					\
	{																														\
		UpdateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void UpdateImpl()																										\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);					\
		}																													\
	}
#endif

#ifndef ON_UPDATE_CRITICAL_ID
#define ON_UPDATE_CRITICAL_ID(...)																							\
	void Update() noexcept override final																					\
	{																														\
		UpdateImpl<__VA_ARGS__>();																							\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void UpdateImpl()																										\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnUpdate((*m_systemHelper.m_ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);				\
		}																													\
	}
#endif

	/*ON_LATE_UPDATE*/

#ifndef ON_LATE_UPDATE
#define ON_LATE_UPDATE(...)																									\
	void LateUpdate() noexcept override final																				\
	{																														\
		LateUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void LateUpdateImpl()																									\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnLateUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);							\
			}																												\
		}																													\
	}
#endif

#ifndef ON_LATE_UPDATE_ID
#define ON_LATE_UPDATE_ID(...)																								\
	void LateUpdate() noexcept override final																				\
	{																														\
		LateUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void LateUpdateImpl()																									\
	{																														\
		auto ePointer = m_systemHelper.GetMinimumEntityVector();															\
		for (int i{ (int)ePointer->size() - 1 }; i >= 0; --i)																\
		{																													\
			if (m_systemHelper.m_mgr.HasAllOf<ComponentType...>((*ePointer)[i]))											\
			{																												\
				OnLateUpdate((*ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*ePointer)[i]) ...);			\
			}																												\
		}																													\
	}
#endif

#ifndef ON_LATE_UPDATE_CRITICAL
#define ON_LATE_UPDATE_CRITICAL(...)																						\
	void LateUpdate() noexcept override final																				\
	{																														\
		LateUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void LateUpdateImpl()																									\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnLateUpdate(m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);				\
		}																													\
	}
#endif

#ifndef ON_LATE_UPDATE_CRITICAL_ID
#define ON_LATE_UPDATE_CRITICAL_ID(...)																						\
	void LateUpdate() noexcept override final																				\
	{																														\
		LateUpdateImpl<__VA_ARGS__>();																						\
	}																														\
																															\
	template<typename... ComponentType>																						\
	void LateUpdateImpl()																									\
	{																														\
		for (int i{*m_systemHelper.m_bundleStart }; i >= 0; --i)															\
		{																													\
			OnLateUpdate((*m_systemHelper.m_ePointer)[i], m_systemHelper.m_mgr.GetComponent<ComponentType>((*m_systemHelper.m_ePointer)[i])...);			\
		}																													\
	}
#endif
