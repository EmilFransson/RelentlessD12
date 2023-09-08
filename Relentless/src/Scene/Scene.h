#pragma once
#include "..\ECS\EntityManager.h"
#include "../ImGui/ImguiLayer.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/MeshFactory.h"
#include "LightManager.h"
#include "../Graphics/Renderer/Camera/PerspectiveCamera.h"
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

		template<auto ShapeType>
		requires std::is_same_v<decltype(ShapeType), Shape>
		entity CreateShape() noexcept;

		template<auto ExtraType>
		requires std::is_same_v<decltype(ExtraType), Extra>
		entity CreateExtra() noexcept;

		template<auto ShapeType>
		requires std::is_same_v<decltype(ShapeType), Shape>
		[[nodiscard]] std::string GetFullShapePath() noexcept;

		template<auto ExtraType>
		requires std::is_same_v<decltype(ExtraType), Extra>
		[[nodiscard]] std::string GetFullExtraPath() noexcept;

		entity CreateCamera(const char* name) noexcept;
		void DestroyEntity(const entity entityHandle) noexcept;
		[[nodiscard]] EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept 
		{ 
			m_ViewportPanelSize = viewportPanelSize;
			m_Viewport.Width = m_ViewportPanelSize.x;
			m_Viewport.Height = m_ViewportPanelSize.y;
			m_ScissorRect.right = static_cast<LONG>(m_ViewportPanelSize.x);
			m_ScissorRect.bottom = static_cast<LONG>(m_ViewportPanelSize.y);
		}

		[[nodiscard]] bool EntityIsDescendant(const entity ancestor, const entity descendant) noexcept;
		[[nodiscard]] bool EntityIsAncestor(const entity ancestor, const entity descendant) noexcept;

		void ParentEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept;
		[[nodiscard]] entity FindEntityByUUID(const UUID& uuid) noexcept;
		[[nodiscard]] LightManager& GetLightManager() noexcept { return m_LightManager; }
		[[nodiscard]] const D3D12_VIEWPORT& GetViewport() const noexcept { return m_Viewport; }
		[[nodiscard]] const RECT& GetScissorRect() const noexcept { return m_ScissorRect; }
		[[nodiscard]] const std::shared_ptr<PerspectiveCamera>& GetEditorCamera() const noexcept { return m_pEditorCamera; }

		void SetMousePosition(const ImVec2& newMousePosition) noexcept { m_MousePosition = newMousePosition; }
		[[nodiscard]] const ImVec2& GetMousePosition() const noexcept { return m_MousePosition; }
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
	};

	template<auto ShapeType>
	requires std::is_same_v<decltype(ShapeType), Shape>
	entity Scene::CreateShape() noexcept
	{
		std::filesystem::path fullPath = GetFullShapePath<ShapeType>();
		std::string nameString = fullPath.stem().string();

		auto& mm = AssetManager::GetMeshManager();
		MeshHandle& meshHandle = mm.GetHandleByString(nameString);

		auto entity = CreateEntity(nameString.c_str());

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.MeshHandle = meshHandle;
		
		m_EntityManager.Add<OpaquePassComponent>(entity);
		
		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);

		MaterialManager& materialManager = AssetManager::GetMaterialManager();
		mrc.MaterialHandle = materialManager.GetDefaultMaterialHandle();

		m_EntityManager.Add<DirtyMeshRendererComponent>(entity);

		return entity;
	}

	template<auto ExtraType>
	requires std::is_same_v<decltype(ExtraType), Extra>
	entity Scene::CreateExtra() noexcept
	{
		std::filesystem::path fullPath = GetFullExtraPath<ExtraType>();
		std::string nameString = fullPath.stem().string();
		
		auto entity = CreateEntity(nameString.c_str());

		MeshManager& mm = AssetManager::GetMeshManager();
		MeshHandle& meshHandle = mm.GetHandleByString(nameString);

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.MeshHandle = meshHandle;

		m_EntityManager.Add<OpaquePassComponent>(entity);
		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);

		MaterialManager& materialManager = AssetManager::GetMaterialManager();
		mrc.MaterialHandle = materialManager.GetDefaultMaterialHandle();
		m_EntityManager.Add<DirtyMeshRendererComponent>(entity);

		return entity;
	}

	template<auto ShapeType>
	requires std::is_same_v<decltype(ShapeType), Shape>
	std::string Scene::GetFullShapePath() noexcept
	{
		if constexpr (ShapeType == Shape::Triangle)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Triangle.obj";
		}
		else if constexpr (ShapeType == Shape::Cube)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Cube.obj";
		}
		else if constexpr (ShapeType == Shape::Cylinder)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Cylinder.gltf";
		}
		else if constexpr (ShapeType == Shape::Capsule)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Capsule.gltf";
		}
		else if constexpr (ShapeType == Shape::Cone)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Cone.gltf";
		}
		else if constexpr (ShapeType == Shape::Sphere)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Sphere.obj";
		}
		else if constexpr (ShapeType == Shape::IcoSphere)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Icosphere.obj";
		}
		else if constexpr (ShapeType == Shape::Torus)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Torus.obj";
		}
		else if constexpr (ShapeType == Shape::Quad)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Quad.gltf";
		}
		else if constexpr (ShapeType == Shape::Plane)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Plane.gltf";
		}
		else
		{
			RLS_ASSERT(false, "Unknown shape type.");
			return {};
		}
	}

	template<auto ExtraType>
	requires std::is_same_v<decltype(ExtraType), Extra>
	std::string Scene::GetFullExtraPath() noexcept
	{
		if constexpr (ExtraType == Extra::UtahTeapot)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/UtahTeapot.gltf";
		}
		else
		{
			RLS_ASSERT(false, "Unknown extra type.");
			return {};
		}
	}
}