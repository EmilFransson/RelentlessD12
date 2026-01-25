#include "Scene.h"
#include "../Assets/AssetManager.h"
#include "ECS/Systems/DeferredEntityDeletionSystem.h"

#include "Graphics/Resources/Material.h"

namespace Relentless
{
	Scene::Scene(const char* aName) noexcept
		:m_UUID{ CreateUUID() },
		 m_Name{ aName }
	{
	}

	bool Scene::AnyEntityHasName(const char* pName) const noexcept
	{
		const std::string name(pName);

		const std::vector<NameComponent>& components = m_EntityManager.Collect<NameComponent>().GetComponents();
		for (const auto& component : components)
		{
			if (component.Name == name)
				return true;
		}

		return false;
	}

	const UUID& Scene::GetUUID() const noexcept
	{
		return m_UUID;
	}

	void Scene::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Scene::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_FUNC;

		SceneState state
		{
			.Scene = *this,
			.EntityManager = m_EntityManager,
			.DeltaTime = deltaTime
		};

		DeferredEntityDeletionSystem::Execute(state);
	}

	entity Scene::DuplicateEntity(entity entityToCopy, bool /*preserveHierarchy*/) noexcept
	{
		const String originalName = m_EntityManager.Get<NameComponent>(entityToCopy).Name;

		// Find the position where trailing digits start
		size_t end = originalName.size();
		while (end > 0 && std::isdigit(originalName[end - 1]))
			--end;

		const String baseName = originalName.substr(0, end);
		const String numberPart = originalName.substr(end);

		uint32 number = 1u;

		if (!numberPart.empty())
			number = std::stoi(numberPart); 

		int nextNumber = Math::Max(2u, number + 1u);

		String newName{};
		do
		{
			newName = baseName + std::to_string(nextNumber++);
		} while (AnyEntityHasName(newName.c_str()));

		const entity newEntity = CreateEntityWithUUID(newName.c_str(), CreateUUID());

		if (m_EntityManager.Has<IsChildComponent>(entityToCopy))
			AttachEntity(newEntity, m_EntityManager.Get<IsChildComponent>(entityToCopy).Parent);

		CopyComponentIfExists<TransformComponent>(entityToCopy, newEntity);
		auto& tc = m_EntityManager.Get<TransformComponent>(newEntity);
		tc.Scene = this;
		tc.Self = newEntity;

		CopyComponentIfExists<MeshFilterComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<MeshRendererComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<DirectionalLightComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<PointLightComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<SpotLightComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<CameraComponent>(entityToCopy, newEntity);
		CopyComponentIfExists<HiddenInGameComponent>(entityToCopy, newEntity);

		return newEntity;
	}

	entity Scene::CreateEntity(const char* name) noexcept
	{
		return CreateEntityWithUUID(name, CreateUUID());
	}

	entity Scene::CreateEntityWithUUID(const char* name, const UUID& guid) noexcept
	{
		auto entity = m_EntityManager.CreateEntity();

		auto& tc = m_EntityManager.Add<TransformComponent>(entity);
		tc.Self = entity;
		tc.Scene = this;

		m_EntityManager.Add<NameComponent>(entity, name);
		m_EntityManager.Add<IDComponent>(entity, guid);
		m_EntityManager.Add<RootComponent>(entity);

		OnEntityCreated(entity);

		return entity;
	}

	entity Scene::CreateLight(const char* name, ELightType aLightTypetype) noexcept
	{
		auto lightEntity = CreateEntity(name);
		if (aLightTypetype == ELightType::Directional)
			m_EntityManager.Add<DirectionalLightComponent>(lightEntity);
		else if (aLightTypetype == ELightType::Point)
			m_EntityManager.Add<PointLightComponent>(lightEntity);
		else
		{
			RLS_ASSERT(aLightTypetype == ELightType::Spot, "[Scene::CreateLight]: Invalid Light Type");
			m_EntityManager.Add<SpotLightComponent>(lightEntity);
		}
		
		return lightEntity;
	}

	void Scene::OnRuntimeStart() noexcept
	{

	}


	void Scene::OnRuntimeStop() noexcept
	{
		SetPaused(false);

		//Here we make sure the copy-created assets are removed (they are duplicates):
		std::unordered_map<UUID, AssetHandle> assetsToRemove;

		//Materials:
		m_EntityManager.Collect<MeshRendererComponent>().Do([&assetsToRemove](MeshRendererComponent& mrc)
			{
				if (!assetsToRemove.contains(mrc.AssetHandle.Uuid))
				{
					assetsToRemove[mrc.AssetHandle.Uuid] = mrc.AssetHandle;
				}
			});

		for (auto& [uuid, handle] : assetsToRemove)
		{
			AssetManager::Destroy<Material>(handle);
		}
	}

	entity Scene::CreateCamera(const char* name) noexcept
	{
		auto camera = CreateEntity(name);
		auto& cc = m_EntityManager.Add<CameraComponent>(camera);
		auto& tc = m_EntityManager.Get<TransformComponent>(camera);
		tc.SetWorldLocation({0.0f, 0.0f, -5.0f});

		Vector3 forward = Vector3::Zero - Vector3(0.0f, 0.0f, -5.0f);
		forward.Normalize();
		
		cc.WorldToView = Math::CreateLookToMatrix(Vector3(0.0f, 0.0f, -5.0f), forward, Vector3::Up);
		cc.ViewToClip = Math::CreatePerspectiveMatrix(cc.FieldOfViewDegrees, 16.0f / 9.0f, cc.ClippingPlaneNear, cc.ClippingPlaneFar);

		return camera;
	}
			
	void Scene::DestroyEntity(const entity entityHandle) noexcept
	{
		m_PendingEntityDeletionQueue.Push(entityHandle);
	}

	bool Scene::EntityHasAncestors(entity e) const noexcept
	{
		return m_EntityManager.Has<IsChildComponent>(e);
	}

	bool Scene::EntityIsDescendant(const entity ancestor, const entity descendant) const noexcept
	{
		RLS_ASSERT(m_EntityManager.Exists(ancestor), "Ancestor entity does not exist");
		RLS_ASSERT(m_EntityManager.Exists(descendant), "Descendant entity does not exist");

		if (!m_EntityManager.Has<ParentComponent>(ancestor))
			return false;

		auto& children = m_EntityManager.Get<ParentComponent>(ancestor).Children;
		for (auto child : children)
		{
			if (child == descendant)
				return true;
		}

		//Continue to check the hierarchy of descendants:
		for (auto child : children)
		{
			if (EntityIsDescendant(child, descendant))
				return true;
		}

		return false;
	}

	bool Scene::EntityIsAncestor(const entity ancestor, const entity descendant) noexcept
	{
		RLS_ASSERT(m_EntityManager.Exists(ancestor), "Ancestor entity does not exist");
		RLS_ASSERT(m_EntityManager.Exists(descendant), "Descendant entity does not exist");

		return EntityIsDescendant(ancestor, descendant);
	}

	bool Scene::EntityIsParent(entity possibleChild, entity possibleParent) const noexcept
	{
		return m_EntityManager.Has<IsChildComponent>(possibleChild) && m_EntityManager.Get<IsChildComponent>(possibleChild).Parent == possibleParent;
	}

	bool Scene::EntityIsChild(entity possibleChild, entity possibleParent) const noexcept
	{
		if (!m_EntityManager.Has<ParentComponent>(possibleParent))
			return false;

		auto& pc = m_EntityManager.Get<ParentComponent>(possibleParent);
		return std::any_of(pc.Children.begin(), pc.Children.end(), [possibleChild](entity e) { return e == possibleChild; });
	}

	void Scene::AttachEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept
	{
		RLS_ASSERT(m_EntityManager.Exists(toBecomeChild), "Entity to become child does not exist");
		RLS_ASSERT(m_EntityManager.Exists(toBecomeParent), "Entity to become parent does not exist");

		if (HasParent(toBecomeChild) && GetParent(toBecomeChild) == toBecomeParent)
			return;

		//Only entities with no ancestors have RootComponent:
		if (m_EntityManager.Has<RootComponent>(toBecomeChild))
			m_EntityManager.Remove<RootComponent>(toBecomeChild);

		DetachEntity(toBecomeChild);

		m_EntityManager.Add<IsChildComponent>(toBecomeChild).Parent = toBecomeParent;

		//Child is taken care of, now parent should be too:
		if (m_EntityManager.Has<ParentComponent>(toBecomeParent))
			m_EntityManager.Get<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);
		else
			m_EntityManager.Add<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);

		//The bond has been established. What remains is to adjust the local transform data of the child accordingly:
		auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeParent);
		auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeChild);

		const Matrix childWorldMatrix = childTransformComponent.GetWorldMatrix();
		Matrix inverseParentWorldMatrix = parentTransformComponent.GetWorldMatrix();
		inverseParentWorldMatrix = inverseParentWorldMatrix.Invert();

		Matrix childLocalMatrix = childWorldMatrix * inverseParentWorldMatrix;

		Vector3 scale		= Vector3::One;
		Quaternion rotation = Quaternion::Identity;
		Vector3 location	= Vector3::Zero;

		childLocalMatrix.Decompose(scale, rotation, location);
		childTransformComponent.SetLocalLocation(location);
		childTransformComponent.SetLocalRotation(rotation);
		childTransformComponent.SetLocalScale(scale);

		OnEntityAttached(toBecomeChild, toBecomeParent);
	}

	bool Scene::DetachEntity(const entity entityToDetach) noexcept
	{
		if (!m_EntityManager.Has<IsChildComponent>(entityToDetach))
			return false;

		auto& icc = m_EntityManager.Get<IsChildComponent>(entityToDetach);
		const entity parent = icc.Parent;
		auto& pc = m_EntityManager.Get<ParentComponent>(parent);
		
		pc.Children.erase(std::remove(pc.Children.begin(), pc.Children.end(), entityToDetach), pc.Children.end());
		if (pc.Children.empty())
			m_EntityManager.Remove<ParentComponent>(parent);

		m_EntityManager.Remove<IsChildComponent>(entityToDetach);

		if (!m_EntityManager.Has<RootComponent>(entityToDetach))
			m_EntityManager.Add<RootComponent>(entityToDetach);

		OnEntityDetached(entityToDetach, parent);
		return true;
	}

	void Scene::ForEachEntityWithAncestorEntity(entity aEntity, bool aIncludeAncestor, const Callback<bool(entity)>& aOperation) noexcept
	{
		std::vector<entity> entitiesToProcess;
		if (aIncludeAncestor)
			entitiesToProcess.push_back(aEntity);

		const std::vector<entity> descendants = GetAllEntityDescendants(aEntity);
		entitiesToProcess.insert(entitiesToProcess.end(), descendants.begin(), descendants.end());

		for (entity e : entitiesToProcess)
		{
			if (!aOperation(e))
				break;
		}
	}

	std::vector<entity> Scene::GetAllEntityDescendants(entity rootEntity) noexcept
	{
		std::vector<entity> allDecendants;

		if (m_EntityManager.Has<ParentComponent>(rootEntity))
		{
			auto& pc = m_EntityManager.Get<ParentComponent>(rootEntity);
			for (auto child : pc.Children)
			{
				allDecendants.push_back(child);
				const std::vector<entity> descendants = GetAllEntityDescendants(child);
				allDecendants.insert(allDecendants.end(), descendants.begin(), descendants.end());
			}
		}

		return allDecendants;
	}

	std::vector<entity> Scene::GetAllEntityAncestors(entity rootEntity) noexcept
	{
		std::vector<entity> allAncestors;

		if (m_EntityManager.Has<IsChildComponent>(rootEntity))
		{
			entity currentEntity = rootEntity;
			do
			{
				auto& icc = m_EntityManager.Get<IsChildComponent>(currentEntity);
				allAncestors.push_back(icc.Parent);
				currentEntity = icc.Parent;
			} while (m_EntityManager.Has<IsChildComponent>(currentEntity));
		}

		return allAncestors;
	}

	std::vector<entity> Scene::GetEntityChildren(entity parent) noexcept
	{
		std::vector<entity> immediateChildren;

		if (m_EntityManager.Has<ParentComponent>(parent))
			immediateChildren = m_EntityManager.Get<ParentComponent>(parent).Children;

		return immediateChildren;
	}

	entity Scene::FindEntityByUUID(const UUID& uuid) noexcept
	{
		entity toReturn = NULL_ENTITY;
		m_EntityManager.Collect<IDComponent>().Do([&](entity e, const IDComponent& id)
			{
				if (uuid == id.UuId)
				{
					toReturn = e;
					return;
				}
			});

		return toReturn;
	}

	void Scene::SetEntityVisibleInGame(entity e, bool visibilityState) noexcept
	{
		if (visibilityState == true && !IsEntityVisible(e))
		{
			m_EntityManager.Remove<HiddenInGameComponent>(e);
			OnEntityVisibilityChanged(e, true);
		}
		else if (visibilityState == false && IsEntityVisible(e))
		{
			m_EntityManager.Add<HiddenInGameComponent>(e);
			OnEntityVisibilityChanged(e, false);
		}

		const std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (entity descendant : descendants)
			SetEntityVisibleInGame(descendant, visibilityState);
	}


	bool Scene::IsParent(entity e) noexcept
	{
		return m_EntityManager.Has<ParentComponent>(e);
	}

	bool Scene::HasParent(entity e) noexcept
	{
		return m_EntityManager.Has<IsChildComponent>(e);
	}

	entity Scene::GetParent(entity e) noexcept
	{
		RLS_ASSERT(HasParent(e), "[Scene]: Entity has no parent.");

		return m_EntityManager.Get<IsChildComponent>(e).Parent;
	}

	bool Scene::IsEntityVisible(entity e) noexcept
	{
		return !m_EntityManager.Has<HiddenInGameComponent>(e);
	}

 	void Scene::MarkDirty() noexcept
 	{
 		m_IsDirty = true;
 	}

	bool Scene::SerializeCore(IArchive& aArchive) noexcept
	{
		if (aArchive.IsSaving())
		{
			m_EntityManager.Collect<IDComponent>().Do([this, &aArchive](entity aEntity)
				{
					SerializeEntity(aArchive, aEntity);

				});
		}

		return true;
	}

	bool Scene::SerializeEntity(IArchive& /*aArchive*/, entity /*aEntity*/) noexcept
	{
		//RLS_ASSERT(m_EntityManager.Exists(aEntity), "[Scene::SerializeEntity]: Entity is invalid.");
		//
		//auto&& ConditionallyProcess = [&]<typename ComponentType>() -> bool
		//{
		//	bool hasComponent = m_EntityManager.Has<ComponentType>(aEntity);
		//	if (!aArchive.Process(hasComponent))
		//		return false;
		//
		//	if constexpr (!std::is_empty_v<ComponentType>)
		//	{
		//		if (hasComponent)
		//		{
		//			ComponentType& component = m_EntityManager.Get<ComponentType>(aEntity);
		//			if (!aArchive.Process(component))
		//				return false;
		//		}
		//	}
		//
		//	return true;
		//};
		//
		//return
		//	ConditionallyProcess.template operator()<IDComponent>() &&
		//	ConditionallyProcess.template operator()<NameComponent>() &&
		//	ConditionallyProcess.template operator()<RootComponent>() &&
		//	ConditionallyProcess.template operator()<TransformComponent>() &&
		//	ConditionallyProcess.template operator()<MeshFilterComponent>() &&
		//	ConditionallyProcess.template operator()<MeshRendererComponent>() &&
		//	ConditionallyProcess.template operator()<DirectionalLightComponent>() &&
		//	ConditionallyProcess.template operator()<PointLightComponent>() &&
		//	ConditionallyProcess.template operator()<SpotLightComponent>() &&
		//	ConditionallyProcess.template operator()<CameraComponent>() &&
		//	ConditionallyProcess.template operator()<HiddenInGameComponent>() &&
		//	ConditionallyProcess.template operator()<FolderComponent>();

		return true;
	}

	template<typename ComponentType>
	void CopyComponent(std::shared_ptr<Scene> srcScene, std::shared_ptr<Scene> dstScene, std::unordered_map<UUID, entity>& idToEntityMap) noexcept
	{
		auto& srcMgr = srcScene->GetEntityManager();
		auto& dstMgr = dstScene->GetEntityManager();

		if constexpr (std::is_empty_v<ComponentType>)
		{
			srcMgr.Collect<ComponentType>().Do([&](entity e)
				{
					auto& uuid = srcMgr.Get<IDComponent>(e).UuId;
					RLS_ASSERT(idToEntityMap.contains(uuid), "Unknown entity UUID encountered.");

					entity entityID = idToEntityMap[uuid];

					dstMgr.AddOrReplace<ComponentType>(entityID);
				});
		}
		else
		{
			srcMgr.Collect<ComponentType>().Do([&](entity e, ComponentType& ct)
				{
					auto& uuid = srcMgr.Get<IDComponent>(e).UuId;
					RLS_ASSERT(idToEntityMap.contains(uuid), "Unknown entity UUID encountered.");

					entity entityID = idToEntityMap[uuid];

					dstMgr.AddOrReplace<ComponentType>(entityID, ct);

					if constexpr (std::is_same_v<ComponentType, IsChildComponent>)
					{
						entity actualParent = idToEntityMap[srcMgr.Get<IDComponent>(ct.Parent).UuId];
						dstMgr.Get<IsChildComponent>(entityID).Parent = actualParent;
					}
					else if constexpr (std::is_same_v<ComponentType, MeshRendererComponent>)
					{
						RLS_ASSERT(false, "TODO");
						//const AssetHandle materialHandle = AssetManager::CreateNew<Material>();
						//Ref<Material> material = AssetManager::Get<Material>(materialHandle);

						//As it will be copy assigned we first invalidate it: //HUH?
						//material = AssetManager::Get<Material>(ct.AssetHandle);

						//dstMgr.Get<MeshRendererComponent>(entityID).AssetHandle = materialHandle;
					}
					else if constexpr (std::is_same_v<ComponentType, ParentComponent>)
					{
						auto& children = dstMgr.Get<ParentComponent>(entityID).Children;
						children.clear();

						auto& originalChildren = srcMgr.Get<ParentComponent>(entityID).Children;
						for (auto child : originalChildren)
						{
							entity actualChild = idToEntityMap[srcMgr.Get<IDComponent>(child).UuId];
							children.push_back(actualChild);
						}
					}
				});
		}
	}

	std::shared_ptr<Scene> Scene::Copy(std::shared_ptr<Scene> pSrcScene) noexcept
	{
		RLS_ASSERT(pSrcScene, "Scene to copy from is invalid.");

		std::shared_ptr<Scene> pNewScene = std::make_shared<Scene>("Play-In-Editor-Scene");

		std::unordered_map<UUID, entity> UUIDToEntityMap;
		
		auto& mgr = pNewScene->GetEntityManager();
		pSrcScene->GetEntityManager().Collect<NameComponent, IDComponent>().Do([&UUIDToEntityMap, &mgr](NameComponent& nc, IDComponent& idc)
			{
				const auto entity = mgr.CreateEntity();
				mgr.Add<NameComponent>(entity, nc.Name.c_str());
				mgr.Add<IDComponent>(entity, idc.UuId);
				UUIDToEntityMap[idc.UuId] = entity;
			});

		CopyComponent<RootComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<MeshRendererComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<TransformComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<MeshFilterComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<DirectionalLightComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<PointLightComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<IsChildComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<ParentComponent>(pSrcScene, pNewScene, UUIDToEntityMap);

		return pNewScene;
	}
}