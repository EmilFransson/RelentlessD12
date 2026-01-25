#pragma once
#include "Callback/Callback.h"
#include "Callback/Broadcaster.h"

#include "Core/DLLExport.h"
#include "Core/IAsset.h"

#include "ECS/EntityManager.h"

#include "Threading/ThreadSafeQueue.h"

namespace Relentless
{
	enum class ELightType : uint8 { Directional = 0, Point, Spot };

	class RLS_API Scene : public AssetBase<Scene>
	{
	public:
		explicit Scene(const char* aName = "Sample Scene") noexcept;
		virtual ~Scene() noexcept = default;

		NO_DISCARD bool AnyEntityHasName(const char* pName) const noexcept;

		NO_DISCARD const UUID& GetUUID() const noexcept;

		void SetName(const std::string& name) noexcept;
		void OnUpdate(const float deltaTime) noexcept;
		entity DuplicateEntity(entity entityToCopy, bool preserveHierarchy) noexcept;
		entity CreateCamera(const char* name) noexcept;
		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag, const UUID& guid) noexcept;
		entity CreateLight(const char* aName, ELightType aLightType) noexcept;

		void OnRuntimeStart() noexcept;
		void OnRuntimeStop() noexcept;

		void DestroyEntity(const entity entityHandle) noexcept;
		NO_DISCARD EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		NO_DISCARD const EntityManager& GetEntityManager() const noexcept { return m_EntityManager; }
		NO_DISCARD const std::string& GetName() const noexcept { return m_Name; }
		NO_DISCARD bool EntityHasAncestors(entity e) const noexcept;
		NO_DISCARD bool EntityIsDescendant(const entity ancestor, const entity descendant) const noexcept;
		NO_DISCARD bool EntityIsAncestor(const entity ancestor, const entity descendant) noexcept;
		NO_DISCARD bool EntityIsParent(entity possibleChild, entity possibleParent) const noexcept;
		NO_DISCARD bool EntityIsChild(entity possibleChild, entity possibleParent) const noexcept;
		void AttachEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept;
		bool DetachEntity(const entity entityToDetach) noexcept;

		void ForEachEntityWithAncestorEntity(entity aEntity, bool aIncludeAncestor, const Callback<bool(entity)>& aOperation) noexcept;

		NO_DISCARD std::vector<entity> GetAllEntityDescendants(entity rootEntity) noexcept;
		NO_DISCARD std::vector<entity> GetAllEntityAncestors(entity rootEntity) noexcept;
		NO_DISCARD std::vector<entity> GetEntityChildren(entity parent) noexcept;
		NO_DISCARD entity FindEntityByUUID(const UUID& uuid) noexcept;
		void SetEntityVisibleInGame(entity e, bool visibilityState) noexcept;

		NO_DISCARD bool IsParent(entity e) noexcept;
		NO_DISCARD bool HasParent(entity e) noexcept;
		NO_DISCARD entity GetParent(entity e) noexcept;

		void SetPaused(bool paused) noexcept { m_IsPaused = paused; }
		NO_DISCARD bool IsPaused() const noexcept { return m_IsPaused; }

		NO_DISCARD static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> pSrcScene) noexcept;

		template<typename ComponentType>
		void CopyComponentIfExists(entity srcEntity, entity dstEntity) noexcept;

		NO_DISCARD bool IsEntityVisible(entity e) noexcept;

		void MarkDirty() noexcept;

		NO_DISCARD bool SerializeCore(IArchive& aArchive) noexcept override;

		//std::shared_ptr<TextureCube> m_pSkyBox = nullptr;
		//std::shared_ptr<TextureCube> m_pIrradianceMap = nullptr;
		//std::shared_ptr<TextureCube> m_pRadianceMap = nullptr;

		Broadcaster<void(entity e)> OnEntityCreated;
		Broadcaster<void(entity e)> OnEntityDestroy;
		Broadcaster<void(entity e)> OnEntityDestroyed;
		Broadcaster<void(entity child, entity parent)> OnEntityAttached;
		Broadcaster<void(entity detachedEntity, entity formerParent)> OnEntityDetached;
		Broadcaster<void(entity e, bool visibilityState)> OnEntityVisibilityChanged;
	private:
		bool SerializeEntity(IArchive& aArchive, entity aEntity) noexcept;
	private:
		friend class SceneSerializer;
		friend struct DeferredEntityDeletionSystem;
		
		ThreadSafeQueue<entity> m_PendingEntityDeletionQueue;

		mutable EntityManager m_EntityManager;
		UUID m_UUID = NULL_UUID;
		String m_Name;
		bool m_IsPaused = false;
		bool m_IsDirty = false;
	};

	struct SceneState
	{
		Scene& Scene;
		EntityManager& EntityManager;
		float DeltaTime = 0.0f;
	};

	template<typename ComponentType>
	void Scene::CopyComponentIfExists(entity srcEntity, entity dstEntity) noexcept
	{
		if (!m_EntityManager.Has<ComponentType>(srcEntity))
			return;

		if (!m_EntityManager.Has<ComponentType>(dstEntity))
			m_EntityManager.Add<ComponentType>(dstEntity);

		if constexpr (!std::is_empty_v<ComponentType>)
		{
			const ComponentType& originalComponent = m_EntityManager.Get<ComponentType>(srcEntity);
			ComponentType& newComponent = m_EntityManager.Get<ComponentType>(dstEntity);
			newComponent = originalComponent;
		}
	}
}