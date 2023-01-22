#pragma once
#include "..\ECS\EntityManager.h"
#include "../ImGui/ImguiLayer.h"
#include "../Mesh/Vertex.h"
#include "../Mesh/MeshFactory.h"
namespace Relentless
{
	struct LightStruct
	{
		DirectX::XMFLOAT3 Direction;
		float Intensity;
		DirectX::XMFLOAT3 Color;
	};

	enum class Shape : uint8_t { Triangle = 0, Cube, Cylinder, Capsule, Cone, Sphere, Quad, Plane };

	class Scene
	{
	public:
		explicit Scene(const char* name = "Sample Scene") noexcept;
		~Scene() noexcept = default;

		void OnUpdate() noexcept;
		entity CreateEntity(const char* tag) noexcept;
		entity CreateEntityWithUUID(const char* tag) noexcept;
		entity CreateLight(const char* name, LightComponent::Type type) noexcept;
		void CreateUtahTeapot() noexcept;

		template<auto ShapeType>
		requires std::is_same_v<decltype(ShapeType), Shape>
		entity CreateShape() noexcept
		{
			std::filesystem::path finalPath = GetFullShapePath<ShapeType>();
			MeshFactory factory;
			Mesh shapeMesh = factory.LoadFromFile(finalPath)[0]; //We know a shape only consists of one mesh

			std::string nameString = finalPath.stem().string();

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
			auto vbID = AssetManager::Get().Load<VertexBuffer>(vbSpec.Name, &vbSpec);
			auto ibID = AssetManager::Get().Load<IndexBuffer>(ibSpec.Name, &ibSpec);

			auto entity = CreateEntityWithUUID(nameString.c_str());

			m_EntityManager.Add<MeshFilterComponent>(entity, vbID, ibID);
			m_EntityManager.Add<ForwardPassComponent>(entity);
			m_EntityManager.Add<MeshRendererComponent>(entity).Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

			return entity;
		}

		template<auto ShapeType>
		requires std::is_same_v<decltype(ShapeType), Shape>
		std::string GetFullShapePath() noexcept
		{
			if constexpr (ShapeType == Shape::Triangle)
			{
				return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Triangle.gltf";
			}
			else if constexpr (ShapeType == Shape::Cube)
			{
				return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Cube.gltf";
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
				return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/Sphere.gltf";
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

		entity CreateCamera(const char* name) noexcept;
		void DestroyEntity(const entity entityHandle) noexcept;
		[[nodiscard]] EntityManager& GetEntityManager() noexcept { return m_EntityManager; }
		[[nodiscard]] constexpr const char* GetName() const noexcept { return m_Name; }
		void SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept { m_ViewportPanelSize = viewportPanelSize; }
	private:
		EntityManager m_EntityManager;
		const char* m_Name;
		ImVec2 m_ViewportPanelSize;
	};
}