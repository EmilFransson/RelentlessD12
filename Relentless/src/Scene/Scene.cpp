#include "Scene.h"
#include "../Assets/AssetManager.h"
namespace Relentless
{
	Scene::Scene(const char* name) noexcept
		: m_Name{ name }
	{
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
		
		m_pEditorCamera = PerspectiveCamera::Create(DirectX::XMVECTORF32{ 5.0f, 5.0f, -5.0f }, static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
	}

	Scene::~Scene() noexcept
	{
		m_EntityManager.Collect<TransformComponent>().Do([this](entity e)
			{
				if (m_EntityManager.Has<PointLightComponent>(e))
				{
					m_LightManager.DeallocatePointLight(e);
				}
				else if (m_EntityManager.Has<DirectionalLightComponent>(e))
				{
					m_LightManager.DeallocateDirectionalLight(e);
				}
			});
	}

	void Scene::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Scene::OnUpdate([[maybe_unused]] const float deltaTime) noexcept
	{
		PROFILE_FUNC;
		ResourceManager& resourceManager = Application::Get().GetResourceManager();
		const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();

		/*TRANSFORMS*/
		m_EntityManager.Collect<DirtyTransformComponent, RootComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dtc)
			{
				TransformComponent& transformComponent = m_EntityManager.Get<TransformComponent>(entityHandle);
				if (dtc.AdjustedWorldSpace)
				{
					//Calculate world transform and convert to equivalent local
					//As this is a root entity world == local!
					const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.Rotation.x);
					const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.Rotation.y);
					const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.Rotation.z);

					const DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.Scale))
						* DirectX::XMMatrixRotationX(angleInRadiansX)
						* DirectX::XMMatrixRotationY(angleInRadiansY)
						* DirectX::XMMatrixRotationZ(angleInRadiansZ)
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.Translation));

					DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);

					transformComponent.LocalTransform = transformComponent.Transform;
					transformComponent.LocalTranslation = transformComponent.Translation;
					transformComponent.LocalRotation = transformComponent.Rotation;
					transformComponent.LocalScale = transformComponent.Scale;
				}
				else
				{
					//Calculate local transform and convert to equivalent world:
					//As this is a root entity local == world!
					const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.LocalRotation.x);
					const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.LocalRotation.y);
					const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.LocalRotation.z);
				
					const DirectX::XMMATRIX local = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.LocalScale))
						* DirectX::XMMatrixRotationX(angleInRadiansX)
						* DirectX::XMMatrixRotationY(angleInRadiansY)
						* DirectX::XMMatrixRotationZ(angleInRadiansZ)
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.LocalTranslation));
				
					DirectX::XMStoreFloat4x4(&transformComponent.LocalTransform, local);
				
					transformComponent.Transform = transformComponent.LocalTransform;
					transformComponent.Translation = transformComponent.LocalTranslation;
					transformComponent.Rotation = transformComponent.LocalRotation;
					transformComponent.Scale = transformComponent.LocalScale;
				}
				
				std::function<void(entity)> UpdateChildWorldMatrices;
				UpdateChildWorldMatrices = [&](entity parentEntity)
					{
						if (m_EntityManager.Has<ParentComponent>(parentEntity))
						{
							auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(parentEntity);
							DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
				
							for (auto& childEntity : m_EntityManager.Get<ParentComponent>(parentEntity).Children)
							{
								auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(childEntity);
				
								DirectX::XMMATRIX childLocalMatrix = DirectX::XMLoadFloat4x4(&childTransformComponent.LocalTransform);
								DirectX::XMMATRIX childWorldMatrix = DirectX::XMMatrixMultiply(childLocalMatrix, parentWorldMatrix);
				
								DirectX::XMStoreFloat4x4(&childTransformComponent.Transform, childWorldMatrix);
								ImGuizmo::DecomposeMatrixToComponents
								(
									*childTransformComponent.Transform.m,
									&childTransformComponent.Translation.x,
									&childTransformComponent.Rotation.x,
									&childTransformComponent.Scale.x
								);
				
								// Recursively update the world matrices for this child's children
								
								resourceManager.UploadConstantBufferData(childTransformComponent.ConstantBufferHandle, &childTransformComponent.Transform, sizeof(childTransformComponent.Transform), frameIndex);
								//Application::Get().GetMemorymanager().UpdateConstantBuffer(childTransformComponent.ConstantBufferID, &childTransformComponent.Transform);
								UpdateChildWorldMatrices(childEntity);
							}
						}
					};
				
				// Kick off the recursive update for this entity's children
				UpdateChildWorldMatrices(entityHandle);
				resourceManager.UploadConstantBufferData(transformComponent.ConstantBufferHandle, &transformComponent.Transform, sizeof(transformComponent.Transform), frameIndex);

				//Application::Get().GetMemorymanager().UpdateConstantBuffer(transformComponent.ConstantBufferID, &transformComponent.Transform);
			});

		/*TRANSFORMS*/
		m_EntityManager.Collect<DirtyTransformComponent, IsChildComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dtc)
			{
				TransformComponent& transformComponent = m_EntityManager.Get<TransformComponent>(entityHandle);
				if (dtc.AdjustedWorldSpace)
				{
					//Calculate world transform and convert to equivalent local
					//As this is a root entity world == local!
					const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.Rotation.x);
					const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.Rotation.y);
					const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.Rotation.z);

					const DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.Scale))
						* DirectX::XMMatrixRotationX(angleInRadiansX)
						* DirectX::XMMatrixRotationY(angleInRadiansY)
						* DirectX::XMMatrixRotationZ(angleInRadiansZ)
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.Translation));

					DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);

					auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(m_EntityManager.Get<IsChildComponent>(entityHandle).Parent);

					DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
					DirectX::XMMATRIX inverseParentWorldMatrix = DirectX::XMMatrixInverse(nullptr, parentWorldMatrix);
					DirectX::XMMATRIX local = DirectX::XMMatrixMultiply(world, inverseParentWorldMatrix);

					DirectX::XMStoreFloat4x4(&transformComponent.LocalTransform, local);

					ImGuizmo::DecomposeMatrixToComponents
					(
						&transformComponent.LocalTransform.m[0][0],
						&transformComponent.LocalTranslation.x,
						&transformComponent.LocalRotation.x,
						&transformComponent.LocalScale.x
					);
				}
				else
				{
					//Calculate local transform and convert to equivalent world:
					const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.LocalRotation.x);
					const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.LocalRotation.y);
					const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.LocalRotation.z);

					const DirectX::XMMATRIX local = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.LocalScale))
						* DirectX::XMMatrixRotationX(angleInRadiansX)
						* DirectX::XMMatrixRotationY(angleInRadiansY)
						* DirectX::XMMatrixRotationZ(angleInRadiansZ)
						* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.LocalTranslation));

					DirectX::XMStoreFloat4x4(&transformComponent.LocalTransform, local);

					auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(m_EntityManager.Get<IsChildComponent>(entityHandle).Parent);

					DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
					DirectX::XMMATRIX world = DirectX::XMMatrixMultiply(local, parentWorldMatrix);

					DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);
					ImGuizmo::DecomposeMatrixToComponents
					(
						&transformComponent.Transform.m[0][0],
						&transformComponent.Translation.x,
						&transformComponent.Rotation.x,
						&transformComponent.Scale.x
					);
				}

				std::function<void(entity)> UpdateChildWorldMatrices;
				UpdateChildWorldMatrices = [&](entity parentEntity)
					{
						if (m_EntityManager.Has<ParentComponent>(parentEntity))
						{
							auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(parentEntity);
							DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);

							for (auto& childEntity : m_EntityManager.Get<ParentComponent>(parentEntity).Children)
							{
								auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(childEntity);

								DirectX::XMMATRIX childLocalMatrix = DirectX::XMLoadFloat4x4(&childTransformComponent.LocalTransform);
								DirectX::XMMATRIX childWorldMatrix = DirectX::XMMatrixMultiply(childLocalMatrix, parentWorldMatrix);

								DirectX::XMStoreFloat4x4(&childTransformComponent.Transform, childWorldMatrix);
								ImGuizmo::DecomposeMatrixToComponents
								(
									*childTransformComponent.Transform.m,
									&childTransformComponent.Translation.x,
									&childTransformComponent.Rotation.x,
									&childTransformComponent.Scale.x
								);

								// Recursively update the world matrices for this child's children
								resourceManager.UploadConstantBufferData(childTransformComponent.ConstantBufferHandle, &childTransformComponent.Transform, sizeof(childTransformComponent.Transform), frameIndex);
								//Application::Get().GetMemorymanager().UpdateConstantBuffer(childTransformComponent.ConstantBufferID, &childTransformComponent.Transform);
								UpdateChildWorldMatrices(childEntity);
							}
						}
					};

				// Kick off the recursive update for this entity's children
				UpdateChildWorldMatrices(entityHandle);

				//Application::Get().GetMemorymanager().UpdateConstantBuffer(transformComponent.ConstantBufferID, &transformComponent.Transform);
				resourceManager.UploadConstantBufferData(transformComponent.ConstantBufferHandle, &transformComponent.Transform, sizeof(transformComponent.Transform), frameIndex);

			});


		//m_EntityManager.Collect<DirtyTransformComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dirty)
		//	{
				//if (dirty.AdjustedWorldSpace)
				//{
				//	const float angleInRadiansX = DirectX::XMConvertToRadians(transformComponent.Rotation.x);
				//	const float angleInRadiansY = DirectX::XMConvertToRadians(transformComponent.Rotation.y);
				//	const float angleInRadiansZ = DirectX::XMConvertToRadians(transformComponent.Rotation.z);
				//
				//	const DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.Scale))
				//		* DirectX::XMMatrixRotationX(angleInRadiansX) * DirectX::XMMatrixRotationY(angleInRadiansY) * DirectX::XMMatrixRotationZ(angleInRadiansZ)
				//		* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.Translation));
				//	
				//	
				//	DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);
				//
				//	if (m_EntityManager.Has<IsChildComponent>(entityHandle))
				//	{
				//		auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityHandle);
				//		auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(childComponent.Parent);
				//		DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
				//
				//		DirectX::XMMATRIX childWorldMatrix = DirectX::XMLoadFloat4x4(&transformComponent.Transform);
				//		DirectX::XMMATRIX inverseParentWorldMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform));
				//		DirectX::XMMATRIX childLocalMatrix = childWorldMatrix * inverseParentWorldMatrix;
				//		DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, childLocalMatrix);
				//
				//		ImGuizmo::DecomposeMatrixToComponents
				//		(
				//			&childComponent.LocalTransform.m[0][0],
				//			&childComponent.LocalTranslation.x,
				//			&childComponent.LocalRotation.x,
				//			&childComponent.LocalScale.x
				//		);
				//	}
				//}
				//else
				//{
				//	if (m_EntityManager.Has<IsChildComponent>(entityHandle))
				//	{
				//		auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityHandle);
				//
				//		const float angleInRadiansX = DirectX::XMConvertToRadians(childComponent.LocalRotation.x);
				//		const float angleInRadiansY = DirectX::XMConvertToRadians(childComponent.LocalRotation.y);
				//		const float angleInRadiansZ = DirectX::XMConvertToRadians(childComponent.LocalRotation.z);
				//
				//		const DirectX::XMMATRIX localTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&childComponent.LocalScale))
				//			* DirectX::XMMatrixRotationX(angleInRadiansX) * DirectX::XMMatrixRotationY(angleInRadiansY) * DirectX::XMMatrixRotationZ(angleInRadiansZ)
				//			* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&childComponent.LocalTranslation));
				//		DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, localTransform);
				//
				//		//child local * parent world = child world
				//		auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(childComponent.Parent);
				//		DirectX::XMMATRIX parentWorldMatrix = DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform);
				//		DirectX::XMMATRIX childLocalMatrix = DirectX::XMLoadFloat4x4(&childComponent.LocalTransform);
				//		DirectX::XMMATRIX childWorldMatrix = childLocalMatrix * parentWorldMatrix;
				//
				//		DirectX::XMStoreFloat4x4(&transformComponent.Transform, childWorldMatrix);
				//		ImGuizmo::DecomposeMatrixToComponents
				//		(
				//			&transformComponent.Transform.m[0][0],
				//			&transformComponent.Translation.x,
				//			&transformComponent.Rotation.x,
				//			&transformComponent.Scale.x
				//		);
				//	}
				//}
				//
				//if (m_EntityManager.Has<ParentComponent>(entityHandle))
				//{
				//	std::function<void(entity, DirectX::XMFLOAT4X4&)> SceneGraph;
				//	SceneGraph = [&](entity entityID, DirectX::XMFLOAT4X4& accumulatedT)
				//	{
				//		auto& childComponent = m_EntityManager.Get<IsChildComponent>(entityID);
				//		auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(entityID);
				//
				//		const float angleInRadiansX = DirectX::XMConvertToRadians(childComponent.LocalRotation.x);
				//		const float angleInRadiansY = DirectX::XMConvertToRadians(childComponent.LocalRotation.y);
				//		const float angleInRadiansZ = DirectX::XMConvertToRadians(childComponent.LocalRotation.z);
				//
				//		const DirectX::XMMATRIX childLocalTransform = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&childComponent.LocalScale))
				//			* DirectX::XMMatrixRotationX(angleInRadiansX)
				//			* DirectX::XMMatrixRotationY(angleInRadiansY)
				//			* DirectX::XMMatrixRotationZ(angleInRadiansZ)
				//			* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&childComponent.LocalTranslation));
				//		DirectX::XMStoreFloat4x4(&childComponent.LocalTransform, childLocalTransform);
				//
				//		DirectX::XMMATRIX accumulatedTransformAsXMMatrix = DirectX::XMLoadFloat4x4(&accumulatedT);
				//		DirectX::XMStoreFloat4x4(&childTransformComponent.Transform, DirectX::XMMatrixMultiply(childLocalTransform, accumulatedTransformAsXMMatrix));
				//
				//		ImGuizmo::DecomposeMatrixToComponents
				//		(
				//			*childTransformComponent.Transform.m,
				//			&childTransformComponent.Translation.x,
				//			&childTransformComponent.Rotation.x,
				//			&childTransformComponent.Scale.x
				//		);
				//
				//		m_EntityManager.AddOrReplace<DirtyTransformComponent>(entityID).AdjustedWorldSpace = false;
				//
				//		//Child is also a parent:
				//		if (m_EntityManager.Has<ParentComponent>(entityID))
				//		{
				//			auto& pc = m_EntityManager.Get<ParentComponent>(entityID);
				//			for (auto child : pc.Children)
				//			{
				//				SceneGraph(child, childTransformComponent.Transform);
				//			}
				//		}
				//	};
				//
				//	auto& children = m_EntityManager.Get<ParentComponent>(entityHandle).Children;
				//	for (auto child : children)
				//	{
				//		SceneGraph(child, transformComponent.Transform);
				//	}
				//}
		//auto&& GetLocalModelMatrix = [&](const DirectX::XMFLOAT3& translation, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale) -> DirectX::XMFLOAT4X4
				//	{
				//		DirectX::XMMATRIX transformX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.x));
				//		DirectX::XMMATRIX transformY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotation.y));
				//		DirectX::XMMATRIX transformZ = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotation.z));
				//
				//		DirectX::XMMATRIX rotationMatrix = transformY * transformX * transformZ;
				//
				//		// Create the translation and scaling matrices
				//		DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&translation));
				//		DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scale));
				//
				//		// Create the final transformation matrix: translation * rotation * scale
				//		DirectX::XMMATRIX localModelMatrix = scaleMatrix * rotationMatrix * translationMatrix;
				//		DirectX::XMFLOAT4X4 localModelMatrix4x4;
				//		DirectX::XMStoreFloat4x4(&localModelMatrix4x4, localModelMatrix);
				//		return localModelMatrix4x4;
				//	};
				//
				//std::function<void(entity)> UpdateSelfAndChild;
				//UpdateSelfAndChild = [&](entity e)
				//	{
				//		if (m_EntityManager.Has<IsChildComponent>(e))
				//		{
				//			auto& icc = m_EntityManager.Get<IsChildComponent>(e);
				//			entity parent = icc.Parent;
				//
				//			DirectX::XMMATRIX mat1 = DirectX::XMLoadFloat4x4(&m_EntityManager.Get<TransformComponent>(parent).Transform);
				//			DirectX::XMFLOAT4X4 mat2prev = GetLocalModelMatrix(icc.LocalTranslation, icc.LocalRotation, icc.LocalScale);
				//			DirectX::XMMATRIX mat2 = DirectX::XMLoadFloat4x4(&mat2prev);
				//
				//			// Multiply the matrices
				//			DirectX::XMMATRIX resultMatrix = DirectX::XMMatrixMultiply(mat1, mat2);
				//
				//			// Store the result back into an XMFLOAT4X4
				//			DirectX::XMStoreFloat4x4(&icc.LocalTransform, resultMatrix);
				//			ImGuizmo::DecomposeMatrixToComponents(*icc.LocalTransform.m,
				//				&icc.LocalTranslation.x,
				//				&icc.LocalRotation.x,
				//				&icc.LocalScale.x);
				//
				//			std::cout << "CHILD LOCAL TRANSLATION = [" << icc.LocalTranslation.x << "," << icc.LocalTranslation.y << "," << icc.LocalTranslation.z << "]\n";
				//			std::cout << "CHILD LOCAL ROTATION = [" << icc.LocalRotation.x << "," << icc.LocalRotation.y << "," << icc.LocalRotation.z << "]\n";
				//			std::cout << "CHILD LOCAL SCALE = [" << icc.LocalScale.x << "," << icc.LocalScale.y << "," << icc.LocalScale.z << "]\n";
				//		}
				//		else
				//		{
				//			auto& transformComponent = m_EntityManager.Get<TransformComponent>(e);
				//
				//			transformComponent.Transform = GetLocalModelMatrix(transformComponent.Translation, transformComponent.Rotation, transformComponent.Scale);
				//			ImGuizmo::DecomposeMatrixToComponents(*transformComponent.Transform.m,
				//				&transformComponent.Translation.x,
				//				&transformComponent.Rotation.x,
				//				&transformComponent.Scale.x);
				//
				//			MemoryManager::Get().UpdateConstantBuffer(transformComponent.ConstantBufferID, &transformComponent.Transform);
				//		}
				//		
				//		if (m_EntityManager.Has<ParentComponent>(e))
				//		{
				//			auto& children = m_EntityManager.Get<ParentComponent>(e).Children;
				//			for (auto& child : children)
				//			{
				//				UpdateSelfAndChild(child);
				//			}
				//		}
				//	};
				//
				//UpdateSelfAndChild(entityHandle);
				//auto& transformComponent = m_EntityManager.Get<TransformComponent>(entityHandle);
				//MemoryManager::Get().UpdateConstantBuffer(transformComponent.ConstantBufferID, &transformComponent.Transform);
			//});

		m_EntityManager.Collect<MeshRendererComponent, DirtyMeshRendererComponent>().Do([this](entity e, MeshRendererComponent& mrc)
			{
				Application::Get().GetMemorymanager().SetDirtyMaterial(mrc.AssetHandle);
				m_EntityManager.Remove<DirtyMeshRendererComponent>(e);
			});

		/*MATERIALS*/
		Application::Get().GetMemorymanager().UpdateDirtyMaterials();

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
				{
					dirty.OnlyUpload = false;
					m_EntityManager.Remove<DirtyTransformComponent>(entityHandle);
				}
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

	entity Scene::CreateShape(const Shape shape) noexcept
	{
		std::filesystem::path fullPath = GetFullShapePath(shape);
		std::string nameString = fullPath.stem().string();

		AssetHandle meshHandle = AssetManager::GetHandleByPath(fullPath);

		auto entity = CreateEntity(nameString.c_str());

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.AssetHandle = meshHandle;

		m_EntityManager.Add<OpaquePassComponent>(entity);

		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);

		mrc.AssetHandle = AssetManager::GetDefaultMaterialHandle();

		m_EntityManager.Add<DirtyMeshRendererComponent>(entity);

		return entity;
	}

	entity Scene::CreateExtra(const Extra extra) noexcept
	{
		std::filesystem::path fullPath = GetFullExtraPath(extra);
		std::string nameString = fullPath.stem().string();

		auto entity = CreateEntity(nameString.c_str());

		AssetHandle meshHandle = AssetManager::GetHandleByPath(nameString);

		auto& mfc = m_EntityManager.Add<MeshFilterComponent>(entity);
		mfc.AssetHandle = meshHandle;

		m_EntityManager.Add<OpaquePassComponent>(entity);
		auto& mrc = m_EntityManager.Add<MeshRendererComponent>(entity);

		mrc.AssetHandle = AssetManager::GetDefaultMaterialHandle();
		m_EntityManager.Add<DirtyMeshRendererComponent>(entity);

		return entity;
	}

	std::string Scene::GetFullShapePath(const Shape shape) noexcept
	{
		if (shape == Shape::Triangle)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Triangle.rasset";
		}
		else if (shape == Shape::Cube)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Cube.rasset";
		}
		else if (shape == Shape::Cylinder)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Cylinder.rasset";
		}
		else if  (shape == Shape::Capsule)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Capsule.rasset";
		}
		else if (shape == Shape::Cone)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Cone.rasset";
		}
		else if (shape == Shape::Sphere)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Sphere.rasset";
		}
		else if (shape == Shape::IcoSphere)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Icosphere.rasset";
		}
		else if (shape == Shape::Torus)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Torus.rasset";
		}
		else if (shape == Shape::Quad)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Quad.rasset";
		}
		else if (shape == Shape::Plane)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Models\\StarterContent\\Plane.rasset";
		}
		else
		{
			RLS_ASSERT(false, "Unknown shape type.");
			return {};
		}
	}

	std::string Scene::GetFullExtraPath(const Extra extra) noexcept
	{
		if (extra == Extra::UtahTeapot)
		{
			return std::string(ENGINE_ASSET_DIRECTORY) + "Meshes\\UtahTeapot.gltf";
		}
		else
		{
			RLS_ASSERT(false, "Unknown extra type.");
			return {};
		}
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

		//MemoryManager::Get().FreeConstantBuffer(m_EntityManager.Get<TransformComponent>(entityHandle).ConstantBufferID);

		m_EntityManager.DestroyEntity(entityHandle);
	}

	void Scene::SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept
	{
		m_ViewportPanelSize = viewportPanelSize;
		m_Viewport.Width = m_ViewportPanelSize.x;
		m_Viewport.Height = m_ViewportPanelSize.y;
		m_ScissorRect.right = static_cast<LONG>(m_ViewportPanelSize.x);
		m_ScissorRect.bottom = static_cast<LONG>(m_ViewportPanelSize.y);

		m_pEditorCamera->RecalculateProjectionMatrix(m_Viewport.Width, m_Viewport.Height);
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

		DirectX::XMMATRIX childWorldMatrix = DirectX::XMLoadFloat4x4(&childTransformComponent.Transform);
		DirectX::XMMATRIX inverseParentWorldMatrix = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&parentTransformComponent.Transform));
		DirectX::XMMATRIX childLocalMatrix = childWorldMatrix * inverseParentWorldMatrix;

		DirectX::XMStoreFloat4x4(&childTransformComponent.LocalTransform, childLocalMatrix);
		ImGuizmo::DecomposeMatrixToComponents
		(
			&childTransformComponent.LocalTransform.m[0][0],
			&childTransformComponent.LocalTranslation.x,
			&childTransformComponent.LocalRotation.x,
			&childTransformComponent.LocalScale.x
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

	void Scene::SetHoveredEntity(entity hoveredEntity) noexcept
	{
		m_HoveredEntity = hoveredEntity;
	}

	entity Scene::GetHoveredEntity() const noexcept
	{
		return m_HoveredEntity;
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

					if constexpr (std::is_same_v<ComponentType, TransformComponent>)
					{
						//Also give DirtyTransformComponent:
						dstMgr.AddOrReplace<DirtyTransformComponent>(entityID);
					}
					else if constexpr (std::is_same_v<ComponentType, DirectionalLightComponent>)
					{
						dstScene->GetLightManager().AllocateDirectionalLight(entityID);
					}
					else if constexpr (std::is_same_v<ComponentType, PointLightComponent>)
					{
						dstScene->GetLightManager().AllocatePointLight(entityID);
					}
					else if constexpr (std::is_same_v<ComponentType, IsChildComponent>)
					{
						entity actualParent = idToEntityMap[srcMgr.Get<IDComponent>(ct.Parent).UuId];
						dstMgr.Get<IsChildComponent>(entityID).Parent = actualParent;
					}
					else if constexpr (std::is_same_v<ComponentType, MeshRendererComponent>)
					{
						const AssetHandle materialHandle = AssetManager::CreateNew<Material>();
						std::shared_ptr<Material> material = AssetManager::Get<Material>(materialHandle);

						//As it will be copy assigned we first invalidate it:
						material->Invalidate();
						material = AssetManager::Get<Material>(ct.AssetHandle);

						dstMgr.Get<MeshRendererComponent>(entityID).AssetHandle = materialHandle;
						dstMgr.AddOrReplace<DirtyMeshRendererComponent>(entityID);
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
		const D3D12_VIEWPORT vp = pSrcScene->GetViewport();
		pNewScene->SetViewportPanelSize(ImVec2(vp.Width, vp.Height));

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
		CopyComponent<OpaquePassComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<DirectionalLightComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<PointLightComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<IsChildComponent>(pSrcScene, pNewScene, UUIDToEntityMap);
		CopyComponent<ParentComponent>(pSrcScene, pNewScene, UUIDToEntityMap);

		return pNewScene;
	}
}