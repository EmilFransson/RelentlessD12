#pragma once
#include "ECS/EntityManager.h"
#include "Graphics/Renderer/Camera/PerspectiveCamera.h"
#include "ImGui/ImguiLayer.h"
#include "LightManager.h"
namespace Relentless
{
	enum class Shape : uint8_t { Triangle = 0, Cube, Cylinder, Capsule, Cone, Sphere, IcoSphere, Torus, Quad, Plane };
	enum class Extra : uint8_t { UtahTeapot = 0u };

	class Scene
	{
	public:
		explicit Scene(const char* name = "Sample Scene") noexcept;
		~Scene() noexcept;
		void SetName(const std::string& name) noexcept;
		void OnUpdate(const float deltaTime) noexcept;
		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag, const UUID& guid) noexcept;
		entity CreateLight(const char* name, LightType type) noexcept;
		entity CreateShape(const Shape shape) noexcept;
		entity CreateExtra(const Extra extra) noexcept;
		[[nodiscard]] std::string GetFullShapePath(const Shape shape) noexcept;
		[[nodiscard]] std::string GetFullExtraPath(const Extra extra) noexcept;

		void OnRuntimeStart() noexcept;
		void OnRuntimeStop() noexcept;

		entity CreateCamera(const char* name) noexcept;
		void DestroyEntity(const entity entityHandle) noexcept;
		[[nodiscard]] EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept;
		[[nodiscard]] bool EntityIsDescendant(const entity ancestor, const entity descendant) noexcept;
		[[nodiscard]] bool EntityIsAncestor(const entity ancestor, const entity descendant) noexcept;
		[[nodiscard]] bool EntityIsParent(entity possibleChild, entity possibleParent) noexcept;
		[[nodiscard]] bool EntityIsChild(entity possibleChild, entity possibleParent) noexcept;
		void AttachEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept;
		bool DetachEntity(const entity entityToDetach) noexcept;
		[[nodiscard]] std::vector<entity> GetAllEntityDescendants(entity rootEntity) noexcept;
		[[nodiscard]] std::vector<entity> GetAllEntityAncestors(entity rootEntity) noexcept;
		[[nodiscard]] std::vector<entity> GetEntityChildren(entity parent) noexcept;
		[[nodiscard]] entity FindEntityByUUID(const UUID& uuid) noexcept;
		[[nodiscard]] LightManager& GetLightManager() noexcept { return m_LightManager; }
		[[nodiscard]] const D3D12_VIEWPORT& GetViewport() const noexcept { return m_Viewport; }
		[[nodiscard]] const RECT& GetScissorRect() const noexcept { return m_ScissorRect; }
		[[nodiscard]] const std::shared_ptr<PerspectiveCamera>& GetEditorCamera() const noexcept { return m_pEditorCamera; }
		void SetEntityVisibleInGame(entity e, bool visibilityState) noexcept;

		[[nodiscard]] bool HasParent(entity e) noexcept;
		[[nodiscard]] entity GetParent(entity e)noexcept;

		void SetMousePosition(const ImVec2& newMousePosition) noexcept { m_MousePosition = newMousePosition; }
		[[nodiscard]] const ImVec2& GetMousePosition() const noexcept { return m_MousePosition; }

		void SetPaused(bool paused) noexcept { m_IsPaused = paused; }
		[[nodiscard]] bool IsPaused() const noexcept { return m_IsPaused; }

		static [[nodiscard]] std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> pSrcScene) noexcept;
		void SetHoveredEntity(entity hoveredEntity) noexcept;
		[[nodiscard]] entity GetHoveredEntity() const noexcept;

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

		[[nodiscard]] Matrix GetWorldTransform(entity e) noexcept;
		[[nodiscard]] Vector3 GetWorldLocation(entity e) noexcept;
		[[nodiscard]] Vector3 GetWorldScale(entity e) noexcept;
		[[nodiscard]] Quaternion GetWorldRotation(entity e) noexcept;
		[[nodiscard]] Vector3 GetWorldForward(entity e) noexcept;
		[[nodiscard]] Vector3 GetWorldRight(entity e) noexcept;
		[[nodiscard]] Vector3 GetWorldUp(entity e) noexcept;

		[[nodiscard]] Matrix GetLocalTransform(entity e) noexcept;
		[[nodiscard]] Vector3 GetLocalLocation(entity e) noexcept;
		[[nodiscard]] Quaternion GetLocalRotation(entity e) noexcept;
		[[nodiscard]] Vector3 GetLocalScale(entity e) noexcept;

		[[nodiscard]] bool IsEntityVisible(entity e) noexcept;

		void SetLocalRotationFromEulerDegrees(entity e, float pitchDegrees, float yawDegrees, float rollDegrees) noexcept;
		[[nodiscard]] Vector3 GetLocalRotationInEulerDegrees(entity e);
		
		std::shared_ptr<TextureCube> m_pSkyBox = nullptr;
		std::shared_ptr<TextureCube> m_pIrradianceMap = nullptr;
		std::shared_ptr<TextureCube> m_pRadianceMap = nullptr;

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
		EntityManager m_EntityManager;
		LightManager m_LightManager;
		std::string m_Name;
		ImVec2 m_ViewportPanelSize;
		ImVec2 m_MousePosition;
		D3D12_VIEWPORT m_Viewport;
		RECT m_ScissorRect;
		std::shared_ptr<PerspectiveCamera> m_pEditorCamera{ nullptr };
		friend class SceneSerializer;

		bool m_IsPaused = false;
		entity m_HoveredEntity = NULL_ENTITY;
	};
}