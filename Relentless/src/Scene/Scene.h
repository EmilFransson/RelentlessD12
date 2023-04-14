#pragma once
#include "..\ECS\EntityManager.h"
#include "../ImGui/ImguiLayer.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/MeshFactory.h"
#include "LightManager.h"
namespace Relentless
{
	enum class Shape : uint8_t { Triangle = 0, Cube, Cylinder, Capsule, Cone, Sphere, IcoSphere, Torus, Quad, Plane };
	enum class Extra : uint8_t { UtahTeapot = 0u };

	class Scene
	{
	public:
		explicit Scene(const char* name = "Sample Scene") noexcept;
		~Scene() noexcept = default;

		void OnUpdate() noexcept;
		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag) noexcept;
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
		[[nodiscard]] constexpr const char* GetName() const noexcept { return m_Name; }
		void SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept { m_ViewportPanelSize = viewportPanelSize; }

		[[nodiscard]] bool EntityIsDescendant(const entity ancestor, const entity descendant) noexcept;
		[[nodiscard]] bool EntityIsAncestor(const entity ancestor, const entity descendant) noexcept;

		void ParentEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept;

		[[nodiscard]] LightManager& GetLightManager() noexcept { return m_LightManager; }
	private:
		EntityManager m_EntityManager;
		LightManager m_LightManager;
		const char* m_Name;
		ImVec2 m_ViewportPanelSize;
	};

	template<auto ShapeType>
	requires std::is_same_v<decltype(ShapeType), Shape>
	entity Scene::CreateShape() noexcept
	{
		std::filesystem::path fullPath = GetFullShapePath<ShapeType>();
		std::string nameString = fullPath.stem().string();

		ResourceID vbID;
		ResourceID ibID;
		if (!AssetManager::Get().HasLoaded(nameString + " Vertex Buffer"))
		{
			MeshFactory factory;
			Mesh shapeMesh = factory.LoadFromFile(fullPath)[0]; //We know a shape only consists of one mesh

			VertexBuffer::Specification vbSpec
			{
				.NrOfVertices = (uint32_t)shapeMesh.Vertices.size(),
				.TotalSizeInBytes = (uint32_t)shapeMesh.Vertices.size() * sizeof(SimpleVertex),
				.Stride = sizeof(SimpleVertex),
				.pBuffer = (void*)shapeMesh.Vertices.data(),
				.Name = nameString + std::string(" Vertex Buffer")
			};

			IndexBuffer::Specification ibSpec
			{
				.NrOfIndices = (uint32_t)shapeMesh.Indices.size(),
				.TotalSizeInBytes = (uint32_t)shapeMesh.Indices.size() * sizeof(uint32_t),
				.Stride = sizeof(uint32_t),
				.pBuffer = (void*)shapeMesh.Indices.data(),
				.Name = nameString + std::string(" Index Buffer")
			};
			vbID = AssetManager::Get().Load<VertexBuffer>(vbSpec.Name, &vbSpec);
			ibID = AssetManager::Get().Load<IndexBuffer>(ibSpec.Name, &ibSpec);
		}
		else
		{
			vbID = AssetManager::Get().Load<VertexBuffer>(nameString + " Vertex Buffer", nullptr);
			ibID = AssetManager::Get().Load<IndexBuffer>(nameString + " Index Buffer", nullptr);
		}

		auto entity = CreateEntityWithUUID(nameString.c_str());

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.VertexBufferID = vbID;
		mfc.IndexBufferID = ibID;
		
		m_EntityManager.Add<ForwardPassComponent>(entity);
		auto& atc = m_EntityManager.Get<AlbedoTextureComponent>(entity);
		//atc.AlbedoTextureID = AssetManager::Get().Load<Texture2D>("brickwall.jpg");
		Texture2D* pTex = AssetManager::Get().GetAsset<Texture2D>(atc.AlbedoTextureID);
		
		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);
		mrc.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		mrc.UsesAlbedoMap = 0xFFFFFFFF;
		mrc.AlbedoTextureID = pTex->GetSRVDescriptorHandle().Index;

		mrc.constantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(MeshRendererComponent) - sizeof(uint32_t));
		m_EntityManager.Add<DirtyMeshRendererComponent>(entity);

		return entity;
	}

	template<auto ExtraType>
	requires std::is_same_v<decltype(ExtraType), Extra>
	entity Scene::CreateExtra() noexcept
	{
		std::filesystem::path fullPath = GetFullExtraPath<ExtraType>();
		std::string nameString = fullPath.stem().string();

		ResourceID vbID;
		ResourceID ibID;
		if (!AssetManager::Get().HasLoaded(nameString + " Vertex Buffer"))
		{
			MeshFactory factory;
			Mesh shapeMesh = factory.LoadFromFile(fullPath)[0]; //We know a shape only consists of one mesh

			VertexBuffer::Specification vbSpec
			{
				.NrOfVertices = (uint32_t)shapeMesh.Vertices.size(),
				.TotalSizeInBytes = (uint32_t)shapeMesh.Vertices.size() * sizeof(SimpleVertex),
				.Stride = sizeof(SimpleVertex),
				.pBuffer = (void*)shapeMesh.Vertices.data(),
				.Name = nameString + std::string(" Vertex Buffer")
			};

			IndexBuffer::Specification ibSpec
			{
				.NrOfIndices = (uint32_t)shapeMesh.Indices.size(),
				.TotalSizeInBytes = (uint32_t)shapeMesh.Indices.size() * sizeof(uint32_t),
				.Stride = sizeof(uint32_t),
				.pBuffer = (void*)shapeMesh.Indices.data(),
				.Name = nameString + std::string(" Index Buffer")
			};
			vbID = AssetManager::Get().Load<VertexBuffer>(vbSpec.Name, &vbSpec);
			ibID = AssetManager::Get().Load<IndexBuffer>(ibSpec.Name, &ibSpec);
		}
		else
		{
			vbID = AssetManager::Get().Load<VertexBuffer>(nameString + " Vertex Buffer", nullptr);
			ibID = AssetManager::Get().Load<IndexBuffer>(nameString + " Index Buffer", nullptr);
		}

		auto entity = CreateEntityWithUUID(nameString.c_str());

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.VertexBufferID = vbID;
		mfc.IndexBufferID = ibID;

		m_EntityManager.Add<ForwardPassComponent>(entity);
		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);
		mrc.Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		mrc.constantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(MeshRendererComponent) - sizeof(uint32_t));
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
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/IcoSphere.obj";
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