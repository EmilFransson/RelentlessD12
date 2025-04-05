#include "Scene.h"
#include "../Assets/AssetManager.h"
#include "Input/Keyboard.h"
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
		
		m_pEditorCamera = PerspectiveCamera::Create();
		m_pEditorCamera->SetLocation(Vector3(5.0f, 5.0f, -5.0f ));
		m_pEditorCamera->SetNearPlane(0.1f);
		m_pEditorCamera->SetFarPlane(1'000.0f);
		m_pEditorCamera->SetRotation(Quaternion::CreateFromYawPitchRoll(Math::PI_DIV_4, Math::PI_DIV_4 * 0.5f, 0));
		m_pEditorCamera->SetViewport(FloatRect(0, 0, 800.06, 600.0f));
	}

	Scene::~Scene() noexcept
	{
// 		m_EntityManager.Collect<TransformComponent>().Do([this](entity e)
// 			{
// 				if (m_EntityManager.Has<PointLightComponent>(e))
// 				{
// 					m_LightManager.DeallocatePointLight(e);
// 				}
// 				else if (m_EntityManager.Has<DirectionalLightComponent>(e))
// 				{
// 					m_LightManager.DeallocateDirectionalLight(e);
// 				}
// 			});
	}

	void Scene::SetName(const std::string& name) noexcept
	{
		m_Name = name;
	}

	void Scene::OnUpdate([[maybe_unused]] const float deltaTime) noexcept
	{
		PROFILE_FUNC;
		//ResourceManager& resourceManager = Application::Get().GetResourceManager();
		//const uint32_t frameIndex = Application::Get().GetGPUTaskManager().GetCurrentFrameIndex();

		/*TRANSFORMS*/
		m_EntityManager.Collect<DirtyTransformComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dirty)
		{
			//const ResourceHandle handle = m_EntityManager.Get<TransformComponent>(entityHandle).ConstantBufferHandle;
			DirectX::XMFLOAT4X4 transform = GetWorldTransform(entityHandle);
			//resourceManager.UploadConstantBufferData(handle, &transform, sizeof(transform), frameIndex);
		});

		/*MATERIALS*/
		m_EntityManager.Collect<MeshRendererComponent, DirtyMeshRendererComponent>().Do([this](entity e, MeshRendererComponent& mrc)
			{
				//Application::Get().GetMemorymanager().SetDirtyMaterial(mrc.AssetHandle);
				m_EntityManager.Remove<DirtyMeshRendererComponent>(e);
			});

		//Application::Get().GetMemorymanager().UpdateDirtyMaterials();

		/****LIGHTS****/
		m_EntityManager.Collect<DirectionalLightComponent, DirtyTransformComponent>().Do([&](entity entityID, DirectionalLightComponent& lc)
			{
				auto& tc = m_EntityManager.Get<TransformComponent>(entityID);

				lc.Direction.x = std::sin(DirectX::XMConvertToRadians(tc.WorldTransform.Rotation.y));
				lc.Direction.y = std::cos(DirectX::XMConvertToRadians(tc.WorldTransform.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.WorldTransform.Rotation.y));
				lc.Direction.z = std::sin(DirectX::XMConvertToRadians(tc.WorldTransform.Rotation.x) + DirectX::XMConvertToRadians(90.0f)) * std::cos(DirectX::XMConvertToRadians(tc.WorldTransform.Rotation.y));

				m_LightManager.UpdateDirectionalLight(lc, entityID);
			});

		m_EntityManager.Collect<PointLightComponent, DirtyTransformComponent>().Do([&](entity entityID, PointLightComponent& lc)
			{
				auto& tc = m_EntityManager.Get<TransformComponent>(entityID);

				lc.Position = tc.WorldTransform.Location;
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

		OnEntityCreated(entity);

		return entity;
	}

	entity Scene::CreateLight(const char* name, LightType type) noexcept
	{
		auto lightEntity = CreateEntity(name);
		if (type == LightType::Directional)
		{
			auto& tc = m_EntityManager.Get<TransformComponent>(lightEntity);
			SetLocalRotationFromEulerDegrees(lightEntity, 50.0f, -30.0f, 0.0f);
			tc.WorldTransform.Location = { 0.0f, 3.0f, 0.0f };
			auto& dlc = m_EntityManager.Add<DirectionalLightComponent>(lightEntity);
			dlc.Color = { (255.0f / 255.0f), (244.0f / 255.0f), (214.0f / 255.0f) };
			m_LightManager.AllocateDirectionalLight(lightEntity);
		}
		else if (type == LightType::Point)
		{
			m_EntityManager.Get<TransformComponent>(lightEntity).WorldTransform.Location = { 0.0f, 3.0f, 0.0f };
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
		tc.WorldTransform.Location = {0.0f, 0.0f, -5.0f};

		DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract({ 0.0f, 0.0f, 0.0f }, DirectX::XMLoadFloat3(&tc.WorldTransform.Location)));
		DirectX::XMVECTOR lookAt = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&tc.WorldTransform.Location), forward);
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&tc.WorldTransform.Location), lookAt, DirectX::FXMVECTOR{ 0.0f, 1.0f, 0.0f });

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

		//Check if the entity itself is a child. If it is, remove its entry from parent child list:
		if (HasParent(entityHandle))
			DetachEntity(entityHandle);
		
		const std::vector<entity> children = GetEntityChildren(entityHandle);
		for (entity child : children)
			DetachEntity(child);

		//TODO: CLEAN UP TRANSFORM COMPONENT HANDLES!!
		OnEntityPreDestroyed(entityHandle);
		m_EntityManager.DestroyEntity(entityHandle);
		OnEntityDestroyed(entityHandle);
	}

	void Scene::SetViewportPanelSize(const ImVec2& viewportPanelSize) noexcept
	{
		m_ViewportPanelSize = viewportPanelSize;
		m_Viewport.Width = m_ViewportPanelSize.x;
		m_Viewport.Height = m_ViewportPanelSize.y;
		m_ScissorRect.right = static_cast<LONG>(m_ViewportPanelSize.x);
		m_ScissorRect.bottom = static_cast<LONG>(m_ViewportPanelSize.y);

		m_pEditorCamera->SetViewport(FloatRect(0, 0, m_Viewport.Width, m_Viewport.Height));
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

	bool Scene::EntityIsParent(entity possibleChild, entity possibleParent) noexcept
	{
		return m_EntityManager.Has<IsChildComponent>(possibleChild) && m_EntityManager.Get<IsChildComponent>(possibleChild).Parent == possibleParent;
	}

	bool Scene::EntityIsChild(entity possibleChild, entity possibleParent) noexcept
	{
		if (!m_EntityManager.Has<ParentComponent>(possibleParent))
			return false;

		auto& pc = m_EntityManager.Get<ParentComponent>(possibleParent);
		return std::any_of(pc.Children.begin(), pc.Children.end(), [possibleChild](entity e) { return e == possibleChild; });
	}

	void Scene::AttachEntity(const entity toBecomeChild, const entity toBecomeParent) noexcept
	{
		RLS_ASSERT(m_EntityManager.Exists(toBecomeChild), "Entity to become child does not exist");
		RLS_ASSERT(m_EntityManager.Exists(toBecomeParent), "Entity to become parent does not exist");

		if (HasParent(toBecomeChild) && GetParent(toBecomeChild) == toBecomeParent)
			return;

		//Only entities with no ancestors have RootComponent:
		if (m_EntityManager.Has<RootComponent>(toBecomeChild))
			m_EntityManager.Remove<RootComponent>(toBecomeChild);

		DetachEntity(toBecomeChild);

		m_EntityManager.Add<IsChildComponent>(toBecomeChild).Parent = toBecomeParent;

		//Child is taken care of, now parent should be too:
		if (m_EntityManager.Has<ParentComponent>(toBecomeParent))
			m_EntityManager.Get<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);
		else
			m_EntityManager.Add<ParentComponent>(toBecomeParent).Children.push_back(toBecomeChild);

		//The bond has been established. What remains is to adjust the local transform data of the child accordingly:
		auto& parentTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeParent);
		auto& childTransformComponent = m_EntityManager.Get<TransformComponent>(toBecomeChild);

		const Matrix childWorldMatrix = childTransformComponent.WorldTransform.Matrix;
		Matrix inverseParentWorldMatrix = parentTransformComponent.WorldTransform.Matrix;
		inverseParentWorldMatrix = inverseParentWorldMatrix.Invert();

		SetLocalTransform(toBecomeChild, childWorldMatrix * inverseParentWorldMatrix);

		OnEntityAttached(toBecomeChild, toBecomeParent);
	}

	bool Scene::DetachEntity(const entity entityToDetach) noexcept
	{
		if (!m_EntityManager.Has<IsChildComponent>(entityToDetach))
			return false;

		auto& icc = m_EntityManager.Get<IsChildComponent>(entityToDetach);
		const entity parent = icc.Parent;
		auto& pc = m_EntityManager.Get<ParentComponent>(parent);
		
		pc.Children.erase(std::remove(pc.Children.begin(), pc.Children.end(), entityToDetach), pc.Children.end());
		if (pc.Children.empty())
			m_EntityManager.Remove<ParentComponent>(parent);

		m_EntityManager.Remove<IsChildComponent>(entityToDetach);

		if (!m_EntityManager.Has<RootComponent>(entityToDetach))
			m_EntityManager.Add<RootComponent>(entityToDetach);

		OnEntityDetached(entityToDetach, parent);
		return true;
	}

	std::vector<entity> Scene::GetAllEntityDescendants(entity rootEntity) noexcept
	{
		std::vector<entity> allDecendants;

		if (m_EntityManager.Has<ParentComponent>(rootEntity))
		{
			auto& pc = m_EntityManager.Get<ParentComponent>(rootEntity);
			for (auto child : pc.Children)
			{
				allDecendants.push_back(child);
				const std::vector<entity> descendants = GetAllEntityDescendants(child);
				allDecendants.insert(allDecendants.end(), descendants.begin(), descendants.end());
			}
		}

		return allDecendants;
	}

	std::vector<entity> Scene::GetAllEntityAncestors(entity rootEntity) noexcept
	{
		std::vector<entity> allAncestors;

		if (m_EntityManager.Has<IsChildComponent>(rootEntity))
		{
			entity currentEntity = rootEntity;
			do
			{
				auto& icc = m_EntityManager.Get<IsChildComponent>(currentEntity);
				allAncestors.push_back(icc.Parent);
				currentEntity = icc.Parent;
			} while (m_EntityManager.Has<IsChildComponent>(currentEntity));
		}

		return allAncestors;
	}

	std::vector<entity> Scene::GetEntityChildren(entity parent) noexcept
	{
		std::vector<entity> immediateChildren;

		if (m_EntityManager.Has<ParentComponent>(parent))
			immediateChildren = m_EntityManager.Get<ParentComponent>(parent).Children;

		return immediateChildren;
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

	void Scene::SetEntityVisibleInGame(entity e, bool visibilityState) noexcept
	{
		if (visibilityState == true && !IsEntityVisible(e))
		{
			m_EntityManager.Remove<HiddenInGameComponent>(e);
			OnEntityVisibilityChanged(e, true);
		}
		else if (visibilityState == false && IsEntityVisible(e))
		{
			m_EntityManager.Add<HiddenInGameComponent>(e);
			OnEntityVisibilityChanged(e, false);
		}

		const std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (entity descendant : descendants)
			SetEntityVisibleInGame(descendant, visibilityState);
	}


	bool Scene::IsParent(entity e) noexcept
	{
		return m_EntityManager.Has<ParentComponent>(e);
	}

	bool Scene::HasParent(entity e) noexcept
	{
		return m_EntityManager.Has<IsChildComponent>(e);
	}

	entity Scene::GetParent(entity e) noexcept
	{
		RLS_ASSERT(HasParent(e), "[Scene]: Entity has no parent.");

		return m_EntityManager.Get<IsChildComponent>(e).Parent;
	}

	void Scene::SetHoveredEntity(entity hoveredEntity) noexcept
	{
		m_HoveredEntity = hoveredEntity;
	}

	entity Scene::GetHoveredEntity() const noexcept
	{
		return m_HoveredEntity;
	}

	void Scene::SetLocalTransform(entity e, const Matrix& localTransform) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		Matrix localMatrix = localTransform;
		Vector3 scale = Vector3::Zero;
		Quaternion rotationQuat = Quaternion::Identity;
		Vector3 location = Vector3::Zero;

		if (localMatrix.Decompose(scale, rotationQuat, location))
		{
			tc.LocalTransform.Matrix = localMatrix;
			tc.LocalTransform.Scale = scale;
			tc.LocalTransform.Rotation = rotationQuat;
			tc.LocalTransform.Location = location;

			//tc.IsDirty = true;

			std::vector<entity> descendants = GetAllEntityDescendants(e);
			for (auto& child : descendants)
			{
				//m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
				m_EntityManager.AddOrReplace<DirtyTransformComponent>(child);
			}
		}
	}

	void Scene::SetLocalLocation(entity e, const Vector3& localLocation) noexcept
	{
		m_EntityManager.Get<TransformComponent>(e).LocalTransform.Location = localLocation;
		UpdateLocalTransform(e);
	}

	void Scene::SetLocalRotation(entity e, const Quaternion& localQuaternion) noexcept
	{
		m_EntityManager.Get<TransformComponent>(e).LocalTransform.Rotation = localQuaternion;
		UpdateLocalTransform(e);
	}

	void Scene::SetLocalScale(entity e, const Vector3& localScale) noexcept
	{
		m_EntityManager.Get<TransformComponent>(e).LocalTransform.Scale = localScale;
		UpdateLocalTransform(e);
	}

	void Scene::AddLocalOffset(entity e, const Vector3& localOffset) noexcept
	{
		const Vector3 currentLocalLocation = GetLocalLocation(e);

		const Vector3 desiredLocalLocation =
		{
			currentLocalLocation.x + localOffset.x,
			currentLocalLocation.y + localOffset.y,
			currentLocalLocation.z + localOffset.z
		};

		SetLocalLocation(e, desiredLocalLocation);
	}

	void Scene::AddLocalRotation(entity e, const Vector3& rotationEulerAnglesDegrees) noexcept
	{
		const float pitchRadians = Math::DegToRad(rotationEulerAnglesDegrees.x);
		const float yawRadians = Math::DegToRad(rotationEulerAnglesDegrees.y);
		const float rollRadians = Math::DegToRad(rotationEulerAnglesDegrees.z);

		const Quaternion rotationIncrementQuat = Quaternion::CreateFromYawPitchRoll(yawRadians, pitchRadians, rollRadians);
		
		const Quaternion localRotation = GetLocalRotation(e);
		Quaternion newLocalRotationQuat = localRotation * rotationIncrementQuat;
		newLocalRotationQuat.Normalize();

		SetLocalRotation(e, newLocalRotationQuat);
	}

	void Scene::AddLocalScale(entity e, const Vector3& localScale) noexcept
	{
		const Vector3 currentLocalScale = GetLocalScale(e);
		const Vector3 newScale = currentLocalScale + localScale;
		SetLocalScale(e, newScale);
	}

	void Scene::SetWorldTransform(entity e, const Matrix& worldMatrix) noexcept
	{
		using namespace DirectX;

		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		Matrix parentLocalMatrix = Matrix::Identity;
		if (HasParent(e))
		{
			parentLocalMatrix = GetWorldTransform(GetParent(e));
			parentLocalMatrix = parentLocalMatrix.Invert();
		}

		Matrix localMatrix = worldMatrix * parentLocalMatrix;
		Vector3 scale = Vector3::Zero;
		Quaternion rotationQuat = Quaternion::Identity;
		Vector3 translation = Vector3::Zero;

		if (localMatrix.Decompose(scale, rotationQuat, translation))
		{
			tc.LocalTransform.Matrix = localMatrix;
			rotationQuat.Normalize();
			tc.LocalTransform.Scale = scale;
			tc.LocalTransform.Rotation = rotationQuat;
			tc.LocalTransform.Location = translation;
		
			//tc.IsDirty = true;
			m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);

			const std::vector<entity> descendants = GetAllEntityDescendants(e);
			for (auto& child : descendants)
			{
				//m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
				m_EntityManager.AddOrReplace<DirtyTransformComponent>(child);
			}
		}
	}

	void Scene::SetWorldLocation(entity e, const Vector3& worldLocation) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		
		Matrix parentWorldTransform = Matrix::Identity;
		if (HasParent(e)) 
		{
			parentWorldTransform = GetWorldTransform(GetParent(e));
			parentWorldTransform = parentWorldTransform.Invert();
		}
		tc.LocalTransform.Location = Vector3::Transform(worldLocation, parentWorldTransform);
		
		UpdateLocalTransform(e);
	}

	void Scene::SetWorldRotation(entity e, const Quaternion& worldQuaternion) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		Quaternion parentWorldRotation = Quaternion::Identity;
		if (HasParent(e)) 
		{
			parentWorldRotation = GetWorldRotation(GetParent(e));
			parentWorldRotation.Inverse(parentWorldRotation);
		}
		tc.LocalTransform.Rotation = parentWorldRotation * worldQuaternion;
		
		UpdateLocalTransform(e);
	}

	void Scene::SetWorldScale(entity e, const Vector3& worldScale) noexcept
	{
		// Assuming uniform scaling in the hierarchy for simplicity
		// Non-uniform scaling in parents complicates this process
		using namespace DirectX;
		Vector3 parentWorldScale = Vector3::One;
		if (HasParent(e)) 
			parentWorldScale = GetWorldScale(GetParent(e));

		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		tc.LocalTransform.Scale.x = worldScale.x / parentWorldScale.x;
		tc.LocalTransform.Scale.y = worldScale.y / parentWorldScale.y;
		tc.LocalTransform.Scale.z = worldScale.z / parentWorldScale.z;
		UpdateLocalTransform(e);
	}

	void Scene::AddWorldOffset(entity e, const Vector3& worldOffset) noexcept
	{
		const Vector3 currentWorldLocation = GetWorldLocation(e);

		const Vector3 desiredWorldLocation = 
		{
			currentWorldLocation.x + worldOffset.x,
			currentWorldLocation.y + worldOffset.y,
			currentWorldLocation.z + worldOffset.z
		};

		SetWorldLocation(e, desiredWorldLocation);
	}

	void Scene::AddWorldRotation(entity e, const Vector3& rotationEulerAnglesDegrees) noexcept
	{
		const float pitchRadians = Math::DegToRad(rotationEulerAnglesDegrees.x);
		const float yawRadians = Math::DegToRad(rotationEulerAnglesDegrees.y);
		const float rollRadians = Math::DegToRad(rotationEulerAnglesDegrees.z);

		const Quaternion rollPitchYawQuat = Quaternion::CreateFromYawPitchRoll(yawRadians, pitchRadians, rollRadians);
		const Quaternion currentRotation = GetWorldRotation(e);
		Quaternion desiredWorldRotationQuat = rollPitchYawQuat * currentRotation;
		desiredWorldRotationQuat.Normalize();

		Quaternion parentRotation = Quaternion::Identity;
		if (HasParent(e))
		{
			parentRotation = GetWorldRotation(GetParent(e));
			parentRotation.Inverse(parentRotation);
			parentRotation.Normalize();
		}

		Quaternion newLocalRotationQuat = parentRotation * desiredWorldRotationQuat;
		newLocalRotationQuat.Normalize();

		SetLocalRotation(e, newLocalRotationQuat);
	}

	Matrix Scene::GetWorldTransform(entity e) noexcept
	{
		UpdateWorldTransformIfDirty(e);
		return m_EntityManager.Get<TransformComponent>(e).WorldTransform.Matrix;
	}

	Vector3 Scene::GetWorldLocation(entity e) noexcept
	{
		UpdateWorldTransformIfDirty(e);
		return m_EntityManager.Get<TransformComponent>(e).WorldTransform.Location;
	}

	Vector3 Scene::GetWorldScale(entity e) noexcept
	{
		UpdateWorldTransformIfDirty(e);
		return m_EntityManager.Get<TransformComponent>(e).WorldTransform.Scale;
	}

	Quaternion Scene::GetWorldRotation(entity e) noexcept
	{
		UpdateWorldTransformIfDirty(e);
		return m_EntityManager.Get<TransformComponent>(e).WorldTransform.Rotation;
	}

	Vector3 Scene::GetWorldForward(entity e) noexcept
	{
		Quaternion worldRotation = GetWorldRotation(e);
		worldRotation.Normalize();

		Vector3 worldForwardVector = Vector3::Transform(Vector3::Forward, worldRotation);
		worldForwardVector.Normalize();

		return worldForwardVector;
	}

	Vector3 Scene::GetWorldRight(entity e) noexcept
	{
		Quaternion worldRotation = GetWorldRotation(e);
		worldRotation.Normalize();

		Vector3 worldRightVector = Vector3::Transform(Vector3::Right, worldRotation);
		worldRightVector.Normalize();

		return worldRightVector;
	}

	Vector3 Scene::GetWorldUp(entity e) noexcept
	{
		Quaternion worldRotation = GetWorldRotation(e);
		worldRotation.Normalize();

		Vector3 worldUpVector = Vector3::Transform(Vector3::Up, worldRotation);
		worldUpVector.Normalize();

		return worldUpVector;
	}

	Matrix Scene::GetLocalTransform(entity e) noexcept
	{
		return m_EntityManager.Get<TransformComponent>(e).LocalTransform.Matrix;
	}

	Vector3 Scene::GetLocalLocation(entity e) noexcept
	{
		return m_EntityManager.Get<TransformComponent>(e).LocalTransform.Location;
	}

	Quaternion Scene::GetLocalRotation(entity e) noexcept
	{
		return m_EntityManager.Get<TransformComponent>(e).LocalTransform.Rotation;
	}

	Vector3 Scene::GetLocalScale(entity e) noexcept
	{
		return m_EntityManager.Get<TransformComponent>(e).LocalTransform.Scale;
	}

	bool Scene::IsEntityVisible(entity e) noexcept
	{
		return !m_EntityManager.Has<HiddenInGameComponent>(e);
	}

	void Scene::SetLocalRotationFromEulerDegrees(entity e, float pitchDegrees, float yawDegrees, float rollDegrees) noexcept
	{
		const float pitchRadians = Math::DegToRad(pitchDegrees);
		const float yawRadians = Math::DegToRad(yawDegrees);
		const float rollRadians = Math::DegToRad(rollDegrees);

		Quaternion quaternion = Quaternion::CreateFromYawPitchRoll(yawRadians, pitchRadians, rollRadians);
		quaternion.Normalize();

		SetLocalRotation(e, quaternion);
	}

	Vector3 Scene::GetLocalRotationInEulerDegrees(entity e)
	{
		const Quaternion& localRotation = m_EntityManager.Get<TransformComponent>(e).LocalTransform.Rotation;

		const Matrix matrix = Matrix::CreateFromQuaternion(localRotation);

		const float pitchRad = std::asinf(-matrix._32);
		const float rollRad = atan2f(matrix._12, matrix._22);
		const float yawRad = atan2f(matrix._31, matrix._33);

		Vector3 euler = Vector3::Zero;;
		euler.x = Math::RadToDeg(pitchRad);
		euler.y = Math::RadToDeg(yawRad);
		euler.z = Math::RadToDeg(rollRad);

		return euler;
	}

	void Scene::UpdateWorldTransformIfDirty(entity e) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		//if (tc.IsDirty)
		//{
			UpdateWorldTransform(e);
			//tc.IsDirty = false;
		//}
	}

	void Scene::UpdateWorldTransform(entity e) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		Matrix worldMatrix = Matrix::Identity;
		if (HasParent(e)) 
		{
			const entity parent = GetParent(e);
			const Matrix parentWorld = GetWorldTransform(parent);
			worldMatrix = tc.LocalTransform.Matrix * parentWorld;
		}
		else 
			worldMatrix = tc.LocalTransform.Matrix;

		Vector3 scale = Vector3::Zero;
		Quaternion rotationQuat = Quaternion::Identity;
		Vector3 location = Vector3::Zero;
		if (worldMatrix.Decompose(scale, rotationQuat, location))
		{
			tc.WorldTransform.Matrix = worldMatrix;
			tc.WorldTransform.Scale = scale;
			tc.WorldTransform.Rotation = rotationQuat;
			tc.WorldTransform.Location = location;
			m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);
		}
	}

	void Scene::UpdateLocalTransform(entity e) noexcept
	{
		const Vector3 localLocation = GetLocalLocation(e);
		const Quaternion localRotation = GetLocalRotation(e);
		const Vector3 localScale = GetLocalScale(e);

		const Matrix scaleMatrix = Matrix::CreateScale(localScale);
		const Matrix rotationMatrix = Matrix::CreateFromQuaternion(localRotation);
		const Matrix translationMatrix = Matrix::CreateTranslation(localLocation);

		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		tc.LocalTransform.Matrix = scaleMatrix * rotationMatrix * translationMatrix;

		//tc.IsDirty = true;
		m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);

		std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (auto& child : descendants)
		{
			//m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
			m_EntityManager.AddOrReplace<DirtyTransformComponent>(child);
		}
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
						RLS_ASSERT(false, "TODO");
						//const AssetHandle materialHandle = AssetManager::CreateNew<Material>();
						//Ref<Material> material = AssetManager::Get<Material>(materialHandle);

						//As it will be copy assigned we first invalidate it: //HUH?
						//material = AssetManager::Get<Material>(ct.AssetHandle);

						//dstMgr.Get<MeshRendererComponent>(entityID).AssetHandle = materialHandle;
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