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
		m_EntityManager.Collect<DirtyTransformComponent>().Do([&](entity entityHandle, DirtyTransformComponent& dirty)
		{
			const ResourceHandle handle = m_EntityManager.Get<TransformComponent>(entityHandle).ConstantBufferHandle;
			DirectX::XMFLOAT4X4 transform = GetWorldTransform(entityHandle);
			resourceManager.UploadConstantBufferData(handle, &transform, sizeof(transform), frameIndex);
		});

		/*MATERIALS*/
		m_EntityManager.Collect<MeshRendererComponent, DirtyMeshRendererComponent>().Do([this](entity e, MeshRendererComponent& mrc)
			{
				Application::Get().GetMemorymanager().SetDirtyMaterial(mrc.AssetHandle);
				m_EntityManager.Remove<DirtyMeshRendererComponent>(e);
			});

		Application::Get().GetMemorymanager().UpdateDirtyMaterials();

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

		const Matrix childWorldMatrix = childTransformComponent.WorldTransform.Matrix;
		Matrix inverseParentWorldMatrix = parentTransformComponent.WorldTransform.Matrix;
		inverseParentWorldMatrix.Invert();

		Transform& childLocalTransform = childTransformComponent.LocalTransform;

		childLocalTransform.Matrix = childWorldMatrix * inverseParentWorldMatrix;
		childLocalTransform.Matrix.Decompose(childLocalTransform.Scale, childLocalTransform.Rotation, childLocalTransform.Location);

		childTransformComponent.IsDirty = true;
	}

	bool Scene::DetachEntity(const entity entityToDetach) noexcept
	{
		if (!m_EntityManager.Has<IsChildComponent>(entityToDetach))
			return false;

		auto& icc = m_EntityManager.Get<IsChildComponent>(entityToDetach);
		auto& pc = m_EntityManager.Get<ParentComponent>(icc.Parent);

		pc.Children.erase(std::remove(pc.Children.begin(), pc.Children.end(), entityToDetach), pc.Children.end());
		if (pc.Children.empty())
			m_EntityManager.Remove<ParentComponent>(icc.Parent);

		m_EntityManager.Remove<IsChildComponent>(entityToDetach);
		m_EntityManager.Add<RootComponent>(entityToDetach);

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
		using namespace DirectX;

		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		tc.LocalTransform.Matrix = localTransform;
		
		XMMATRIX localXMTransform = XMLoadFloat4x4(&tc.LocalTransform.Matrix);

		XMVECTOR localTranslation;
		XMVECTOR localRotation;
		XMVECTOR localScale;

		if (XMMatrixDecompose(&localScale, &localRotation, &localTranslation, localXMTransform))
		{
			XMStoreFloat3(&tc.LocalTransform.Location, localTranslation);
			XMStoreFloat4(&tc.LocalTransform.Rotation, localRotation);
			XMStoreFloat3(&tc.LocalTransform.Scale, localScale);
		}

		tc.IsDirty = true;

		std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (auto& child : descendants)
		{
			m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
			m_EntityManager.AddOrReplace<DirtyTransformComponent>(child);
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
		using namespace DirectX;

		// Convert rotation increments from degrees to radians
		const float pitchRadians = XMConvertToRadians(rotationEulerAnglesDegrees.x);
		const float yawRadians = XMConvertToRadians(rotationEulerAnglesDegrees.y);
		const float rollRadians = XMConvertToRadians(rotationEulerAnglesDegrees.z);

		// Create a quaternion representing the rotation increment
		const XMVECTOR rotationIncrementQuat = XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians);

		// Load the current local rotation quaternion
		const Quaternion localRotation = GetLocalRotation(e);
		const XMVECTOR currentLocalRotationQuat = XMLoadFloat4(&localRotation);

		// Compute the new local rotation quaternion
		XMVECTOR newLocalRotationQuat = XMQuaternionMultiply(currentLocalRotationQuat, rotationIncrementQuat);

		// Normalize the quaternion
		newLocalRotationQuat = XMQuaternionNormalize(newLocalRotationQuat);

		// Set the new local rotation
		XMFLOAT4 newLocalRotation;
		XMStoreFloat4(&newLocalRotation, newLocalRotationQuat);
		SetLocalRotation(e, newLocalRotation);
	}

	void Scene::AddLocalScale(entity e, const Vector3& localScale) noexcept
	{
		const Vector3 currentLocalScale = GetLocalScale(e);
		const Vector3 newScale = Vector3(currentLocalScale.x + localScale.x, currentLocalScale.y + localScale.y, currentLocalScale.z + localScale.z);
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
			parentLocalMatrix.Invert();
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
		}
		
		tc.IsDirty = true;
		m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);

		const std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (auto& child : descendants)
		{
			m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
			m_EntityManager.AddOrReplace<DirtyTransformComponent>(child);
		}
	}

	void Scene::SetWorldLocation(entity e, const Vector3& worldLocation) noexcept
	{
		using namespace DirectX;
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		
		XMMATRIX parentWorldTransform = XMMatrixIdentity();
		if (HasParent(e)) 
		{
			DirectX::XMFLOAT4X4 worldTransform = GetWorldTransform(GetParent(e));
			parentWorldTransform = XMLoadFloat4x4(&worldTransform);
		}
		const XMVECTOR desiredWorldPosition = XMLoadFloat3(&worldLocation);
		const XMVECTOR localPosition = XMVector3TransformCoord(desiredWorldPosition, XMMatrixInverse(nullptr, parentWorldTransform));
		XMStoreFloat3(&tc.LocalTransform.Location, localPosition);
		
		UpdateLocalTransform(e);
	}

	void Scene::SetWorldRotation(entity e, const Quaternion& worldQuaternion) noexcept
	{
		using namespace DirectX;
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		XMVECTOR parentWorldRotation = XMQuaternionIdentity();
		if (HasParent(e)) 
		{
			XMFLOAT4 worldRotation = GetWorldRotation(GetParent(e));
			parentWorldRotation = XMLoadFloat4(&worldRotation);
		}
		XMVECTOR desiredWorldRotation = XMLoadFloat4(&worldQuaternion);
		XMVECTOR localRotation = XMQuaternionMultiply(XMQuaternionInverse(parentWorldRotation), desiredWorldRotation);
		XMStoreFloat4(&tc.LocalTransform.Rotation, localRotation);
		
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
		using namespace DirectX;

		// Step 1: Convert rotation increment from degrees to radians
		const float pitchRadians = XMConvertToRadians(rotationEulerAnglesDegrees.x);
		const float yawRadians = XMConvertToRadians(rotationEulerAnglesDegrees.y);
		const float rollRadians = XMConvertToRadians(rotationEulerAnglesDegrees.z);

		// Step 2: Create a quaternion representing the rotation increment
		const XMVECTOR rotationIncrementQuat = XMQuaternionRotationRollPitchYaw(pitchRadians, yawRadians, rollRadians);

		// Step 3: Get the current global rotation quaternion
		const DirectX::XMFLOAT4 currentRotation = GetWorldRotation(e);
		const XMVECTOR currentWorldRotationQuat = XMLoadFloat4(&currentRotation);

		// Step 4: Compute the desired new global rotation quaternion
		// The new rotation is the rotation increment applied to the current rotation
		XMVECTOR desiredWorldRotationQuat = XMQuaternionMultiply(rotationIncrementQuat, currentWorldRotationQuat);

		// Step 5: Adjust the local rotation
		// Get parent's global rotation
		XMVECTOR parentWorldRotationQuat = XMQuaternionIdentity();
		if (HasParent(e))
		{
			DirectX::XMFLOAT4 parentWorldRotation = GetWorldRotation(GetParent(e));
			parentWorldRotationQuat = XMLoadFloat4(&parentWorldRotation);
		}

		// Compute the new local rotation
		XMVECTOR newLocalRotationQuat = XMQuaternionMultiply(XMQuaternionInverse(parentWorldRotationQuat), desiredWorldRotationQuat);

		newLocalRotationQuat = XMQuaternionNormalize(newLocalRotationQuat);

		// Step 6: Store the new local rotation and update the transform
		XMFLOAT4 newRotation;
		XMStoreFloat4(&newRotation, newLocalRotationQuat);
		SetLocalRotation(e, newRotation);
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
		using namespace DirectX;

		// Get the entity's global rotation quaternion
		const XMFLOAT4 worldRotation = GetWorldRotation(e);
		XMVECTOR rotationQuat = XMLoadFloat4(&worldRotation);

		// Ensure the quaternion is normalized
		rotationQuat = XMQuaternionNormalize(rotationQuat);

		// Local forward vector (positive Z-axis)
		const XMVECTOR localForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		// Rotate the local forward vector by the entity's global rotation
		XMVECTOR worldForwardVector = XMVector3Rotate(localForward, rotationQuat);

		// Normalize the forward vector
		worldForwardVector = XMVector3Normalize(worldForwardVector);

		// Store the result in XMFLOAT3
		XMFLOAT3 forward;
		XMStoreFloat3(&forward, worldForwardVector);

		return forward;
	}

	Vector3 Scene::GetWorldRight(entity e) noexcept
	{
		using namespace DirectX;

		// Get the entity's global rotation quaternion
		const XMFLOAT4 worldRotation = GetWorldRotation(e);
		XMVECTOR rotationQuat = XMLoadFloat4(&worldRotation);

		// Ensure the quaternion is normalized
		rotationQuat = XMQuaternionNormalize(rotationQuat);

		// Local right vector (positive X-axis)
		const XMVECTOR localRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

		// Rotate the local right vector by the entity's global rotation
		XMVECTOR worldRightVector = XMVector3Rotate(localRight, rotationQuat);

		// Normalize the right vector
		worldRightVector = XMVector3Normalize(worldRightVector);

		// Store the result in XMFLOAT3
		XMFLOAT3 right;
		XMStoreFloat3(&right, worldRightVector);

		return right;
	}

	Vector3 Scene::GetWorldUp(entity e) noexcept
	{
		using namespace DirectX;

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

	void Scene::SetLocalRotationFromEulerDegrees(entity e, float pitchDegrees, float yawDegrees, float rollDegrees) noexcept
	{
		// Convert degrees to radians
		const float pitchRadians = DirectX::XMConvertToRadians(pitchDegrees);
		const float yawRadians = DirectX::XMConvertToRadians(yawDegrees);
		const float rollRadians = DirectX::XMConvertToRadians(rollDegrees);

		// Create quaternion from Euler angles
		const DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(
			pitchRadians,
			yawRadians,
			rollRadians
		);

		// Store quaternion in XMFLOAT4
		DirectX::XMFLOAT4 rotationQuaternion;
		DirectX::XMStoreFloat4(&rotationQuaternion, quaternion);

		// Set the rotation
		SetLocalRotation(e, rotationQuaternion);
	}

	Vector3 Scene::GetLocalRotationInEulerDegrees(entity e)
	{
		DirectX::XMFLOAT4 localRotation = m_EntityManager.Get<TransformComponent>(e).LocalTransform.Rotation;

		DirectX::XMFLOAT4X4 matrix;
		DirectX::XMStoreFloat4x4(&matrix, DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&localRotation)));

		const float pitch = DirectX::XMScalarASin(-matrix._32);

		const DirectX::XMVECTOR from(DirectX::XMVectorSet(matrix._12, matrix._31, 0.0f, 0.0f));
		const DirectX::XMVECTOR to(DirectX::XMVectorSet(matrix._22, matrix._33, 0.0f, 0.0f));
		const DirectX::XMVECTOR res(DirectX::XMVectorATan2(from, to));

		const float roll = DirectX::XMVectorGetX(res);
		const float yaw = DirectX::XMVectorGetY(res);

		DirectX::XMFLOAT3 euler;
		euler.x = Math::RadToDeg(pitch);
		euler.y = Math::RadToDeg(yaw);
		euler.z = Math::RadToDeg(roll);

		return euler;
	}

	void Scene::UpdateWorldTransformIfDirty(entity e) noexcept
	{
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		if (tc.IsDirty)
		{
			UpdateWorldTransform(e);
			tc.IsDirty = false;
		}
	}

	void Scene::UpdateWorldTransform(entity e) noexcept
	{
		using namespace DirectX;
		
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);

		const XMMATRIX localMatrix = XMLoadFloat4x4(&tc.LocalTransform.Matrix);

		if (HasParent(e)) 
		{
			const entity parent = GetParent(e);
			const XMFLOAT4X4 parentWorld = GetWorldTransform(parent);

			const XMMATRIX parentWorldMatrix = XMLoadFloat4x4(&parentWorld);
			const XMMATRIX worldMatrix = XMMatrixMultiply(localMatrix, parentWorldMatrix);
			XMStoreFloat4x4(&tc.WorldTransform.Matrix, worldMatrix);
		}
		else 
			XMStoreFloat4x4(&tc.WorldTransform.Matrix, localMatrix);

		XMMATRIX worldXMTransform = XMLoadFloat4x4(&tc.WorldTransform.Matrix);

		XMVECTOR worldTranslation;
		XMVECTOR worldRotation;
		XMVECTOR worldScale;

		if (XMMatrixDecompose(&worldScale, &worldRotation, &worldTranslation, worldXMTransform))
		{
			XMStoreFloat3(&tc.WorldTransform.Location, worldTranslation);
			XMStoreFloat4(&tc.WorldTransform.Rotation, worldRotation);
			XMStoreFloat3(&tc.WorldTransform.Scale, worldScale);
		}

		m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);
	}

	void Scene::UpdateLocalTransform(entity e) noexcept
	{
		using namespace DirectX;

		const XMFLOAT3 localTranslation = GetLocalLocation(e);
		const XMFLOAT4 localRotation = GetLocalRotation(e);
		const XMFLOAT3 localScale = GetLocalScale(e);

		const XMMATRIX scaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&localScale));
		const XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&localRotation));
		const XMMATRIX translationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&localTranslation));

		const XMMATRIX localMatrix = scaleMatrix * rotationMatrix * translationMatrix;
		
		TransformComponent& tc = m_EntityManager.Get<TransformComponent>(e);
		XMStoreFloat4x4(&tc.LocalTransform.Matrix, localMatrix);

		tc.IsDirty = true;
		m_EntityManager.AddOrReplace<DirtyTransformComponent>(e);

		std::vector<entity> descendants = GetAllEntityDescendants(e);
		for (auto& child : descendants)
		{
			m_EntityManager.Get<TransformComponent>(child).IsDirty = true;
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