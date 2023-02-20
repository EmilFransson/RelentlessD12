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
		m_EntityManager.Collect<TransformComponent, DirectionalLightComponent, DirtyLightComponent>().Do([&](entity entityID, TransformComponent& tc, DirectionalLightComponent& lc, DirtyLightComponent& dirty)
			{
				lc.Direction.x = std::sin(DirectX::XMConvertToRadians(tc.Rotation.y));
				lc.Direction.y = std::cos(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));
				lc.Direction.z = std::sin(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));

				m_LightManager.UpdateDirectionalLight(lc, entityID);
				
				dirty.Updates--;
				if (dirty.Updates == 0u)
					m_EntityManager.Remove<DirtyLightComponent>(entityID);
			});

		m_EntityManager.Collect<TransformComponent, PointLightComponent, DirtyLightComponent>().Do([&](entity entityID, TransformComponent& tc, PointLightComponent& lc, DirtyLightComponent& dirty)
			{
				lc.Position = tc.Translation;
				
				m_LightManager.UpdatePointLight(lc, entityID);

				dirty.Updates--;
				if (dirty.Updates == 0u)
					m_EntityManager.Remove<DirtyLightComponent>(entityID);
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
		if (type == LightType::Directional)
		{
			auto& tc = m_EntityManager.Get<TransformComponent>(lightEntity);
			tc.Rotation = DirectX::XMFLOAT3(50.0f, -30.0f, 0.0f);
			tc.Translation = { 0.0f, 3.0f, 0.0f };
			auto& dlc = m_EntityManager.Add<DirectionalLightComponent>(lightEntity);
			dlc.Color = { (255.0f / 255.0f), (244.0f / 255.0f), (214.0f / 255.0f) };
			m_LightManager.AllocateDirectionalLight(lightEntity);
		}
		else if (type == LightType::Point)
		{
			m_EntityManager.Get<TransformComponent>(lightEntity).Translation = { 0.0f, 3.0f, 0.0f };
			auto& plc = m_EntityManager.Add<PointLightComponent>(lightEntity);
			plc.Position = { 0.0f, 3.0f, 0.0f };
			plc.Color = { (255.0f / 255.0f), (244.0f / 255.0f), (214.0f / 255.0f) };
			m_LightManager.AllocatePointLight(lightEntity);
		}
		m_EntityManager.Add<DirtyLightComponent>(lightEntity);
		
		return lightEntity;
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
		if (m_EntityManager.Has<PointLightComponent>(entityHandle))
		{
			m_LightManager.DeallocatePointLight(entityHandle);
		}
		else if (m_EntityManager.Has<DirectionalLightComponent>(entityHandle))
		{
			m_LightManager.DeallocateDirectionalLight(entityHandle);
		}

		m_EntityManager.DestroyEntity(entityHandle);
	}
}