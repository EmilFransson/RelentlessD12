#pragma once
#include "Callback/Broadcaster.h"
#include "ECS/EntityManager.h"
#include "Threading/ThreadSafeQueue.h"
namespace Relentless
{
	enum class Shape : uint8_t { Triangle = 0, Cube, Cylinder, Capsule, Cone, Sphere, IcoSphere, Torus, Quad, Plane };
	enum class Extra : uint8_t { UtahTeapot = 0u };
	enum class LightType : uint8_t { Directional = 0, Point };

	class Scene
	{
	public:
		explicit Scene(const char* name = "Sample Scene") noexcept;
		virtual ~Scene() noexcept = default;
		NO_DISCARD bool AnyEntityHasName(const char* pName) const noexcept;
		void SetName(const std::string& name) noexcept;
		void OnUpdate(const float deltaTime) noexcept;
		entity DuplicateEntity(entity entityToCopy, bool preserveHierarchy) noexcept;
		entity CreateCamera(const char* name) noexcept;
		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag, const UUID& guid) noexcept;
		entity CreateLight(const char* name, LightType type) noexcept;
		entity CreateShape(const Shape shape) noexcept;
		entity CreateExtra(const Extra extra) noexcept;
		NO_DISCARD String GetFullShapePath(const Shape shape) noexcept;
		NO_DISCARD String GetFullExtraPath(const Extra extra) noexcept;

		void OnRuntimeStart() noexcept;
		void OnRuntimeStop() noexcept;

		void DestroyEntity(const entity entityHandle) noexcept;
		NO_DISCARD EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		NO_DISCARD const std::string& GetName() const noexcept { return m_Name; }
		NO_DISCARD bool EntityHasAncestors(entity e) const noexcept;
		NO_DISCARD bool EntityIsDescendant(const entity ancestor, const entity descendant) noexcept;
		NO_DISCARD bool EntityIsAncestor(const entity ancestor, const entity descendant) noexcept;
		NO_DISCARD bool EntityIsParent(entity possibleChild, entity possibleParent) noexcept;
		NO_DISCARD bool EntityIsChild(entity possibleChild, entity possibleParent) noexcept;
		void AttachEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept;
		bool DetachEntity(const entity entityToDetach) noexcept;
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

		static NO_DISCARD std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> pSrcScene) noexcept;
		void SetHoveredEntity(entity hoveredEntity) noexcept;
		NO_DISCARD entity GetHoveredEntity() const noexcept;

		//Transform
		void SetLocalTransform(entity e, const Matrix& localMatrix) noexcept;
		void SetLocalLocation(entity e, const Vector3& localLocation) noexcept;
		void SetLocalRotation(entity e, const Quaternion& localQuaternion) noexcept;
		void SetLocalScale(entity e, const Vector3& localScale) noexcept;

		void AddLocalOffset(entity e, const Vector3& localOffset) noexcept;
		void AddLocalRotation(entity e, const Vector3& rotationEulerAnglesDegrees) noexcept;
		void AddLocalScale(entity e, const Vector3& localScale) noexcept;

		void SetWorldTransform(entity e, const Matrix& worldMatrix) noexcept;
		void SetWorldLocation(entity e, const Vector3& worldLocation) noexcept;
		void SetWorldRotation(entity e, const Quaternion& worldQuaternion) noexcept;
		void SetWorldScale(entity e, const Vector3& worldScale) noexcept;

		void AddWorldOffset(entity e, const Vector3& worldOffset) noexcept;
		void AddWorldRotation(entity e, const Vector3& rotationEulerAnglesDegrees) noexcept;
		void AddWorldScale(entity e, const Vector3& worldscale) noexcept;

		template<typename ComponentType>
		void CopyComponentIfExists(entity srcEntity, entity dstEntity) noexcept;

		NO_DISCARD Matrix GetWorldTransform(entity e) noexcept;
		NO_DISCARD Vector3 GetWorldLocation(entity e) noexcept;
		NO_DISCARD Vector3 GetWorldScale(entity e) noexcept;
		NO_DISCARD Quaternion GetWorldRotation(entity e) noexcept;
		NO_DISCARD Vector3 GetWorldForward(entity e) noexcept;
		NO_DISCARD Vector3 GetWorldRight(entity e) noexcept;
		NO_DISCARD Vector3 GetWorldUp(entity e) noexcept;

		NO_DISCARD Matrix GetLocalTransform(entity e) noexcept;
		NO_DISCARD Vector3 GetLocalLocation(entity e) noexcept;
		NO_DISCARD Quaternion GetLocalRotation(entity e) noexcept;
		NO_DISCARD Vector3 GetLocalScale(entity e) noexcept;

		NO_DISCARD bool IsEntityVisible(entity e) noexcept;

		void SetLocalRotationFromEulerDegrees(entity e, float pitchDegrees, float yawDegrees, float rollDegrees) noexcept;
		NO_DISCARD Vector3 GetLocalRotationInEulerDegrees(entity e);
		
		//std::shared_ptr<TextureCube> m_pSkyBox = nullptr;
		//std::shared_ptr<TextureCube> m_pIrradianceMap = nullptr;
		//std::shared_ptr<TextureCube> m_pRadianceMap = nullptr;

		Broadcaster<void(entity e)> OnEntityCreated;
		Broadcaster<void(entity e)> OnEntityPreDestroyed;
		Broadcaster<void(entity e)> OnEntityDestroyed;
		Broadcaster<void(entity child, entity parent)> OnEntityAttached;
		Broadcaster<void(entity detachedEntity, entity formerParent)> OnEntityDetached;
		Broadcaster<void(entity e, bool visibilityState)> OnEntityVisibilityChanged;

	private:
		void UpdateWorldTransformIfDirty(entity e) noexcept;
		void UpdateWorldTransform(entity e) noexcept;
		void UpdateLocalTransform(entity e) noexcept;
	private:
		friend class SceneSerializer;
		friend struct DeferredEntityDeletionSystem;
		
		ThreadSafeQueue<entity> m_PendingEntityDeletionQueue;

		mutable EntityManager m_EntityManager;
		String m_Name;
		bool m_IsPaused = false;
	};

	struct SceneState
	{
		Scene& Scene;
		EntityManager& EntityManager;
		float DeltaTime = 0.0f;
	};

	//FIX FOR EMPTY STRUCTS TO BE COMPATIBLE!

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