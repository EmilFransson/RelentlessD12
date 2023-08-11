#include "Scene.h"
namespace Relentless
{
	Scene::Scene(const char* name) noexcept
		: m_Name{ name }
	{
		m_pEditorCamera = PerspectiveCamera::Create(DirectX::XMVECTORF32{ 5.0f, 5.0f, -5.0f }, static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));

		//Viewport:
		m_Viewport.TopLeftX = 0.0f;
		m_Viewport.TopLeftY = 0.0f;
		m_Viewport.Width = 800.0f;
		m_Viewport.Height = 600.0f;
		m_Viewport.MinDepth = 0.0f;
		m_Viewport.MaxDepth = 1.0f;

		//ScissorRect:
		m_ScissorRect.left = 0u;
		m_ScissorRect.top = 0u;
		m_ScissorRect.right = static_cast<LONG>(m_Viewport.Width);
		m_ScissorRect.bottom = static_cast<LONG>(m_Viewport.Height);
	}

	Scene::~Scene() noexcept
	{
		m_EntityManager.Collect<TransformComponent>().Do([this](entity e, TransformComponent& tc)
			{
				if (m_EntityManager.Has<PointLightComponent>(e))
				{
					m_LightManager.DeallocatePointLight(e);
				}
				else if (m_EntityManager.Has<DirectionalLightComponent>(e))
				{
					m_LightManager.DeallocateDirectionalLight(e);
				}

				MemoryManager::Get().FreeConstantBuffer(tc.ConstantBufferID);
			});
	}

	void Scene::OnUpdate([[maybe_unused]] const float deltaTime) noexcept
	{
		PROFILE_FUNC;

		/*TRANSFORMS*/
		m_EntityManager.Collect<DirtyTransformComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dirty)
			{
				auto& transformComponent = m_EntityManager.Get<TransformComponent>(entityHandle);

				if (dirty.AdjustedWorldSpace)
				{
					const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.Rotation.x);
					const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.Rotation.y);
					const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.Rotation.z);

					const DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.Scale))
						* DirectX::XMMatrixRotationX(angleInRadiansX) * DirectX::XMMatrixRotationY(angleInRadiansY) * DirectX::XMMatrixRotationZ(angleInRadiansZ)
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.Translation));
					
					
					DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);

					if (m_EntityManager.Has<IsChildComponent>(entityHandle))
					{
						auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityHandle);
						auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(childComponent.Parent);
						DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);

						DirectX::XMMATRIX childWorldMatrix = DirectX::XMLoadFloat4x4(&transformComponent.Transform);
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
					}
				}
				else
				{
					if (m_EntityManager.Has<IsChildComponent>(entityHandle))
					{
						auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityHandle);

						const float angleInRadiansX = DirectX::XMConvertToRadians(childComponent.LocalRotation.x);
						const float angleInRadiansY = DirectX::XMConvertToRadians(childComponent.LocalRotation.y);
						const float angleInRadiansZ = DirectX::XMConvertToRadians(childComponent.LocalRotation.z);

						const DirectX::XMMATRIX localTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&childComponent.LocalScale))
							* DirectX::XMMatrixRotationX(angleInRadiansX) * DirectX::XMMatrixRotationY(angleInRadiansY) * DirectX::XMMatrixRotationZ(angleInRadiansZ)
							* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&childComponent.LocalTranslation));
						DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, localTransform);

						//child local * parent world = child world
						auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(childComponent.Parent);
						DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
						DirectX::XMMATRIX childLocalMatrix = DirectX::XMLoadFloat4x4(&childComponent.LocalTransform);
						DirectX::XMMATRIX childWorldMatrix = childLocalMatrix * parentWorldMatrix;

						DirectX::XMStoreFloat4x4(&transformComponent.Transform, childWorldMatrix);
						ImGuizmo::DecomposeMatrixToComponents
						(
							&transformComponent.Transform.m[0][0],
							&transformComponent.Translation.x,
							&transformComponent.Rotation.x,
							&transformComponent.Scale.x
						);
					}
				}

				if (m_EntityManager.Has<ParentComponent>(entityHandle))
				{
					std::function<void(entity, DirectX::XMFLOAT4X4&)> SceneGraph;
					SceneGraph = [&](entity entityID, DirectX::XMFLOAT4X4& accumulatedT)
					{
						auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityID);
						auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(entityID);

						const float angleInRadiansX = DirectX::XMConvertToRadians(childComponent.LocalRotation.x);
						const float angleInRadiansY = DirectX::XMConvertToRadians(childComponent.LocalRotation.y);
						const float angleInRadiansZ = DirectX::XMConvertToRadians(childComponent.LocalRotation.z);

						const DirectX::XMMATRIX childLocalTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&childComponent.LocalScale))
							* DirectX::XMMatrixRotationX(angleInRadiansX)
							* DirectX::XMMatrixRotationY(angleInRadiansY)
							* DirectX::XMMatrixRotationZ(angleInRadiansZ)
							* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&childComponent.LocalTranslation));
						DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, childLocalTransform);

						DirectX::XMMATRIX accumulatedTransformAsXMMatrix = DirectX::XMLoadFloat4x4(&accumulatedT);
						DirectX::XMStoreFloat4x4(&childTransformComponent.Transform, DirectX::XMMatrixMultiply(childLocalTransform, accumulatedTransformAsXMMatrix));

						ImGuizmo::DecomposeMatrixToComponents
						(
							*childTransformComponent.Transform.m,
							&childTransformComponent.Translation.x,
							&childTransformComponent.Rotation.x,
							&childTransformComponent.Scale.x
						);

						m_EntityManager.AddOrReplace<DirtyTransformComponent>(entityID).AdjustedWorldSpace = false;

						//Child is also a parent:
						if (m_EntityManager.Has<ParentComponent>(entityID))
						{
							auto& pc = m_EntityManager.Get<ParentComponent>(entityID);
							for (auto child : pc.Children)
							{
								SceneGraph(child, childTransformComponent.Transform);
							}
						}
					};

					auto& children = m_EntityManager.Get<ParentComponent>(entityHandle).Children;
					for (auto child : children)
					{
						SceneGraph(child, transformComponent.Transform);
					}
				}

				MemoryManager::Get().UpdateConstantBuffer(transformComponent.ConstantBufferID, &transformComponent.Transform);
			});

		/*MATERIALS*/
		auto& dirtyMaterials = AssetManager::Get().GetMaterialManager().GetDirtyMaterials();
		for (uint32_t i{0u}; i < dirtyMaterials.size(); ++i)
		{
			Material::UploadToGPU(dirtyMaterials[i].first);
			dirtyMaterials[i].second--;

			if (dirtyMaterials[i].second == 0u)
			{
				dirtyMaterials.erase(dirtyMaterials.begin() + i);
				i--;
			}
		}

		/****LIGHTS****/
		m_EntityManager.Collect<DirectionalLightComponent, DirtyTransformComponent>().Do([&](entity entityID, DirectionalLightComponent& lc)
			{
				auto& tc = m_EntityManager.Get<TransformComponent>(entityID);

				lc.Direction.x = std::sin(DirectX::XMConvertToRadians(tc.Rotation.y));
				lc.Direction.y = std::cos(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));
				lc.Direction.z = std::sin(DirectX::XMConvertToRadians(tc.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.Rotation.y));

				m_LightManager.UpdateDirectionalLight(lc, entityID);
			});

		m_EntityManager.Collect<PointLightComponent, DirtyTransformComponent>().Do([&](entity entityID, PointLightComponent& lc)
			{
				auto& tc = m_EntityManager.Get<TransformComponent>(entityID);

				lc.Position = tc.Translation;
				m_LightManager.UpdatePointLight(lc, entityID);
			});

		/****Clean up****/
		m_EntityManager.Collect<DirtyTransformComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dirty)
			{
				dirty.Updates--;
				if (dirty.Updates == 0u)
					m_EntityManager.Remove<DirtyTransformComponent>(entityHandle);
			});

	}

	entity Scene::CreateEntity(const char* name) noexcept
	{
		return CreateEntityWithUUID(name, IDComponent().UuId);
	}

	entity Scene::CreateEntityWithUUID(const char* name, const UUID& guid) noexcept
	{
		auto entity = m_EntityManager.CreateEntity();
		auto& tc = m_EntityManager.Add<TransformComponent>(entity);
		tc.ConstantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(DirectX::XMFLOAT4X4));
		m_EntityManager.Add<DirtyTransformComponent>(entity);
		m_EntityManager.Add<NameComponent>(entity, name);
		m_EntityManager.Add<IDComponent>(entity, guid);
		m_EntityManager.Add<RootComponent>(entity);

		return entity;
	}

	entity Scene::CreateLight(const char* name, LightType type) noexcept
	{
		auto lightEntity = CreateEntity(name);
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
		
		return lightEntity;
	}

	entity Scene::CreateCamera(const char* name) noexcept
	{
		auto camera = CreateEntity(name);
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

		MemoryManager::Get().FreeConstantBuffer(m_EntityManager.Get<TransformComponent>(entityHandle).ConstantBufferID);

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
}