#include "Scene.h"
namespace Relentless
{
	Scene::Scene(const char* name) noexcept
		: m_Name{ name }
	{}

	void Scene::OnUpdate() noexcept
	{
		/*TRANSFORMS*/
		//m_EntityManager.Collect<TransformComponent>().Do([&](entity entityID, TransformComponent& tc)
		//	{
		//		const float angleInRadiansX = DirectX::XMConvertToRadians(tc.Rotation.x);
		//		const float angleInRadiansY = DirectX::XMConvertToRadians(tc.Rotation.y);
		//		const float angleInRadiansZ = DirectX::XMConvertToRadians(tc.Rotation.z);
		//		
		//		DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc.Scale))
		//			* DirectX::XMMatrixRotationX(angleInRadiansX)
		//			* DirectX::XMMatrixRotationY(angleInRadiansY)
		//			* DirectX::XMMatrixRotationZ(angleInRadiansZ)
		//			* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc.Translation));
		//		DirectX::XMStoreFloat4x4(&tc.Transform, world);
		//
		//		//if (m_EntityManager.Has<ChildComponent>(entityID))
		//		//{
		//		//	auto& ptc = m_EntityManager.Get<TransformComponent>(m_EntityManager.Get<ChildComponent>(entityID).Parent);
		//		//	DirectX::XMMATRIX inverse = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&ptc.Transform));
		//		//	DirectX::XMStoreFloat4x4(&tc.LocalTransform, DirectX::XMMatrixMultiply(world, inverse));
		//		//	ImGuizmo::DecomposeMatrixToComponents(*tc.LocalTransform.m, &tc.LocalTranslation.x, &tc.LocalRotation.x, &tc.LocalScale.x);
		//		//}
		//	});

		//m_EntityManager.Collect<TransformComponent, RootComponent>().Do([&](entity entityID, TransformComponent& tc, RootComponent& rc)
		//	{
		//		std::function<void(entity, DirectX::XMFLOAT4X4&)> SceneGraph;
		//		SceneGraph = [&](entity entityID, DirectX::XMFLOAT4X4& accumulatedT) 
		//		{
		//			auto& childTC = m_EntityManager.Get<TransformComponent>(entityID);
		//
		//			const float angleInRadiansX = DirectX::XMConvertToRadians(childTC.LocalRotation.x);
		//			const float angleInRadiansY = DirectX::XMConvertToRadians(childTC.LocalRotation.y);
		//			const float angleInRadiansZ = DirectX::XMConvertToRadians(childTC.LocalRotation.z);
		//
		//			DirectX::XMMATRIX childLocalTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&childTC.LocalScale))
		//				* DirectX::XMMatrixRotationX(angleInRadiansX)
		//				* DirectX::XMMatrixRotationY(angleInRadiansY)
		//				* DirectX::XMMatrixRotationZ(angleInRadiansZ)
		//				* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&childTC.LocalTranslation));
		//			DirectX::XMStoreFloat4x4(&childTC.LocalTransform, childLocalTransform);
		//
		//			DirectX::XMMATRIX accumulatedTransformAsXMMatrix = DirectX::XMLoadFloat4x4(&accumulatedT);
		//
		//			DirectX::XMStoreFloat4x4(&childTC.Transform, DirectX::XMMatrixMultiply(childLocalTransform, accumulatedTransformAsXMMatrix));
		//			ImGuizmo::DecomposeMatrixToComponents(*childTC.Transform.m, &childTC.Translation.x, &childTC.Rotation.x, &childTC.Scale.x);
		//			//Child is a parent:
		//			if (m_EntityManager.Has<ParentComponent>(entityID))
		//			{
		//				auto& pc = m_EntityManager.Get<ParentComponent>(entityID);
		//				for (auto child : pc.Children)
		//				{
		//					SceneGraph(child, childTC.Transform);
		//				}
		//			}
		//		};
		//
		//		for (auto child : rc.Children)
		//		{
		//			SceneGraph(child, tc.Transform);
		//		}
		//	});

		/*MATERIALS*/
		m_EntityManager.Collect<MeshRendererComponent, DirtyMeshRendererComponent>().Do([&](entity entityID, MeshRendererComponent& mrc, DirtyMeshRendererComponent& dirty)
			{
				MemoryManager::Get().UpdateConstantBuffer(mrc.constantBufferID, &mrc.Color);
				
				dirty.Updates--;
				if (dirty.Updates == 0u)
					m_EntityManager.Remove<DirtyMeshRendererComponent>(entityID);
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
		m_EntityManager.Add<RootComponent>(entity);

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

		//Check if entity is parent and, if so, destroy all children as well:
		if (m_EntityManager.Has<ParentComponent>(entityHandle))
		{
			auto& children = m_EntityManager.Get<ParentComponent>(entityHandle).Children;
			for (auto child : children)
				DestroyEntity(child);
		}
		//Check if the entity itself is a child. If it is, remove its entry from parent child list:
		if (m_EntityManager.Has<IsChildComponent>(entityHandle))
		{
			entity parent = m_EntityManager.Get<IsChildComponent>(entityHandle).Parent;
			auto& children = m_EntityManager.Get<ParentComponent>(parent).Children;
			for (uint32_t childIndex{0u}; childIndex < children.size(); ++childIndex)
			{
				if (children[childIndex] == entityHandle)
				{
					children.erase(children.begin() + childIndex);
					if (children.empty())
						m_EntityManager.Remove<ParentComponent>(parent);

					break;
				}
			}
		}


		m_EntityManager.DestroyEntity(entityHandle);
	}

	bool Scene::EntityIsDescendant(const entity ancestor, const entity descendant) noexcept
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

	void Scene::ParentEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept
	{
		RLS_ASSERT(m_EntityManager.Exists(toBecomeChild), "Entity to become child does not exist");
		RLS_ASSERT(m_EntityManager.Exists(toBecomeParent), "Entity to become parent does not exist");

		//Only entities with no ancestors have RootComponent:
		if (m_EntityManager.Has<RootComponent>(toBecomeChild))
			m_EntityManager.Remove<RootComponent>(toBecomeChild);

		//If entity to become child already is child that bond should be broken (multiple parents not allowed):
		if (m_EntityManager.Has<IsChildComponent>(toBecomeChild))
		{
			auto& icc = m_EntityManager.Get<IsChildComponent>(toBecomeChild);
			auto& children = m_EntityManager.Get<ParentComponent>(icc.Parent).Children;
			for (uint32_t childIndex{ 0u }; childIndex < children.size(); ++childIndex)
			{
				if (toBecomeChild == children[childIndex])
				{
					children.erase(children.begin() + childIndex);
					if (children.empty())
						m_EntityManager.Remove<ParentComponent>(icc.Parent);
					break;
				}
			}
			icc.Parent = toBecomeParent;
		}
		else
		{
			m_EntityManager.Add<IsChildComponent>(toBecomeChild).Parent = toBecomeParent;
		}

		//Child is taken care of, now parent should be too:
		if (m_EntityManager.Has<ParentComponent>(toBecomeParent))
			m_EntityManager.Get<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);
		else
			m_EntityManager.Add<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);

		//The bond has been established. What remains is to adjust the local transform data of the child accordingly:
		auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeParent);
		auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeChild);
		auto& childComponent = m_EntityManager.Get<IsChildComponent>(toBecomeChild);

		DirectX::XMMATRIX childWorldMatrix = DirectX::XMLoadFloat4x4(&childTransformComponent.Transform);
		DirectX::XMMATRIX inverseParentWorldMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform));
		DirectX::XMMATRIX childLocalMatrix = childWorldMatrix * inverseParentWorldMatrix;
		DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, childLocalMatrix);
		ImGuizmo::DecomposeMatrixToComponents
		(
			&childComponent.LocalTransform.m[0][0],
			&childComponent.LocalTranslation.x, 
			&childComponent.LocalRotation.x, 
			&childComponent.LocalScale.x
		);

		//child local * parent world = child world
		//DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
		//childWorldMatrix = childLocalMatrix * parentWorldMatrix;
		//DirectX::XMStoreFloat4x4(&childTransformComponent.Transform, childWorldMatrix);
		//ImGuizmo::DecomposeMatrixToComponents
		//(
		//	&childTransformComponent.Transform.m[0][0],
		//	&childTransformComponent.Translation.x,
		//	&childTransformComponent.Rotation.x,
		//	&childTransformComponent.Scale.x
		//);
	}
}