#include "Scene.h"
namespace Relentless
{
	Scene::Scene(const char* name) noexcept
		: m_Name{ name }
	{}

	void Scene::OnUpdate() noexcept
	{
		m_EntityManager.Collect<MeshRendererComponent>().Do([](MeshRendererComponent& mrc)
			{
				auto& cb = *mrc.constantBuffer;
				MemoryManager::Get().UpdateConstantBuffer(cb, &mrc.Color);
			});

		/****LIGHTS****/
		static DirectionalLightStruct directionalLightStruct;
		m_EntityManager.Collect<TransformComponent, DirectionalLightComponent>().Do([&](TransformComponent& tc, DirectionalLightComponent& lc)
			{
				directionalLightStruct.Direction.x = std::sin(DirectX::XMConvertToRadians(tc.Rotation.y));
				directionalLightStruct.Direction.y = std::cos(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));
				directionalLightStruct.Direction.z = std::sin(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));
				directionalLightStruct.Intensity = lc.Intensity;
				directionalLightStruct.Color = lc.Color;

				auto& cb = *lc.constantBuffer;
				MemoryManager::Get().UpdateConstantBuffer(cb, &directionalLightStruct);
			});

		static PointLightStruct pointLightStruct;
		m_EntityManager.Collect<TransformComponent, PointLightComponent>().Do([&](TransformComponent& tc, PointLightComponent& lc)
			{
				pointLightStruct.Position = tc.Translation;
				pointLightStruct.Intensity = lc.Intensity;
				pointLightStruct.Color = lc.Color;

				auto& cb = *lc.constantBuffer;
				MemoryManager::Get().UpdateConstantBuffer(cb, &pointLightStruct);
			});
	}

	entity Scene::CreateEntity(const char* name) noexcept
	{
		return CreateEntityWithUUID(name);
	}

	entity Scene::CreateEntityWithUUID(const char* name) noexcept
	{
		auto entity = m_EntityManager.CreateEntity();
		m_EntityManager.Add<TransformComponent>(entity);
		m_EntityManager.Add<NameComponent>(entity, name);
		m_EntityManager.Add<IDComponent>(entity);

		return entity;
	}

	entity Scene::CreateLight(const char* name, LightType type) noexcept
	{
		auto lightEntity = CreateEntityWithUUID(name);
		m_EntityManager.Get<TransformComponent>(lightEntity).Rotation = DirectX::XMFLOAT3(50.0f, -30.0f, 0.0f);
		if (type == LightType::Directional)
		{
			auto& dlc = m_EntityManager.Add<DirectionalLightComponent>(lightEntity);
			dlc.Color = { (255.0f / 255.0f), (244.0f / 255.0f), (214.0f / 255.0f) };
			m_EntityManager.Get<TransformComponent>(lightEntity).Translation = { 0.0f, 3.0f, 0.0f };
		}
		else if (type == LightType::Point)
		{
			auto& plc = m_EntityManager.Add<PointLightComponent>(lightEntity);
			plc.Color = { (255.0f / 255.0f), (244.0f / 255.0f), (214.0f / 255.0f) };
			m_EntityManager.Get<TransformComponent>(lightEntity).Translation = { 0.0f, 3.0f, 0.0f };
		}
		
		return lightEntity;
	}

	entity Scene::CreateUtahTeapot() noexcept
	{
		std::filesystem::path finalPath = std::string(ENGINE_ASSET_DIRECTORY) + "Meshes/UtahTeapot.gltf";
		std::string nameString = finalPath.stem().string();

		ResourceID vbID;
		ResourceID ibID;
		if (!AssetManager::Get().HasLoaded(nameString + " Vertex Buffer"))
		{
			MeshFactory factory;
			Mesh shapeMesh = factory.LoadFromFile(finalPath)[0];

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
		m_EntityManager.Add<MeshFilterComponent>(entity, vbID, ibID);
		m_EntityManager.Add<ForwardPassComponent>(entity);
		m_EntityManager.Add<MeshRendererComponent>(entity).Color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		return entity;
	}

	entity Scene::CreateCamera(const char* name) noexcept
	{
		auto camera = CreateEntityWithUUID(name);
		auto& cc = m_EntityManager.Add<CameraComponent>(camera);
		auto& tc = m_EntityManager.Get<TransformComponent>(camera);
		tc.Translation = {0.0f, 0.0f, -5.0f};

		DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract({ 0.0f, 0.0f, 0.0f }, DirectX::XMLoadFloat3(&tc.Translation)));
		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&tc.Translation), forward);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&tc.Translation), lookAt, DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f });

		float aspectRatio = static_cast<float>(m_ViewportPanelSize.x) / static_cast<float>(m_ViewportPanelSize.y);
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(cc.FieldOfViewDegrees),
			aspectRatio,
			cc.ClippingPlaneNear,
			cc.ClippingPlaneFar);

		DirectX::XMStoreFloat4x4(&cc.ViewMatrix, viewMatrix);
		DirectX::XMStoreFloat4x4(&cc.ProjectionMatrix, projectionMatrix);

		return camera;
	}
			
	void Scene::DestroyEntity(const entity entityHandle) noexcept
	{
		m_EntityManager.DestroyEntity(entityHandle);
	}
}