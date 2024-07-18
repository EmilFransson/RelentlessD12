#pragma once
#include "Assets/AssetMeta.h"
#include "ECSCommon.h"
#include "Graphics/D3D12Core.h"
#include "Graphics/GPUTaskManager.h"
#include "Graphics/Resources/Material.h"
#include "Mesh/Mesh.h"
#include "Core/Application.h"

namespace Relentless
{
	struct TransformComponent
	{
		TransformComponent() noexcept
			: Translation{ 0.0f, 0.0f, 0.0f },
			  Rotation{ 0.0f, 0.0f, 0.0f },
			  Scale{ 1.0f, 1.0f, 1.0f },
			  LocalTranslation{ 0.0f, 0.0f, 0.0f },
			  LocalRotation{0.0f, 0.0f, 0.0f },
			  LocalScale{ 1.0f, 1.0f, 1.0f }
		{
			DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&LocalTransform, DirectX::XMMatrixIdentity());

			ConstantBufferHandle = Application::Get().GetResourceManager().CreateConstantBufferSet("", sizeof(Transform));
		}

		~TransformComponent() noexcept 
		{
			//if (ConstantBufferID != INVALID_CONSTANT_BUFFER_ID)
			//{
			//	Application::Get().GetMemorymanager().FreeConstantBuffer(ConstantBufferID);
			//}
		}

		TransformComponent(const TransformComponent& otherComponent) noexcept
		{
			Transform = otherComponent.Transform;
			Translation = otherComponent.Translation;
			Rotation = otherComponent.Rotation;
			Scale = otherComponent.Scale;

			LocalTransform = otherComponent.LocalTransform;
			LocalTranslation = otherComponent.LocalTranslation;
			LocalRotation = otherComponent.LocalRotation;
			LocalScale = otherComponent.LocalScale;

			ConstantBufferHandle = Application::Get().GetResourceManager().CreateConstantBufferSet("", sizeof(Transform));
		}

		TransformComponent(TransformComponent&& otherComponent) noexcept
		{
			Transform = std::move(otherComponent.Transform);
			Translation = std::move(otherComponent.Translation);
			Rotation = std::move(otherComponent.Rotation);
			Scale = std::move(otherComponent.Scale);

			LocalTransform = std::move(otherComponent.LocalTransform);
			LocalTranslation = std::move(otherComponent.LocalTranslation);
			LocalRotation = std::move(otherComponent.LocalRotation);
			LocalScale = std::move(otherComponent.LocalScale);

			ConstantBufferHandle = std::move(otherComponent.ConstantBufferHandle);

			//Invalidate the moved from constant buffer ID to catch any misuse:
			otherComponent.ConstantBufferHandle = NULL_RESOURCE_HANDLE;
		}

		TransformComponent& operator=(TransformComponent&& otherComponent) noexcept
		{
			if (this != &otherComponent)
			{
				Transform = std::move(otherComponent.Transform);
				Translation = std::move(otherComponent.Translation);
				Rotation = std::move(otherComponent.Rotation);
				Scale = std::move(otherComponent.Scale);

				LocalTransform = std::move(otherComponent.LocalTransform);
				LocalTranslation = std::move(otherComponent.LocalTranslation);
				LocalRotation = std::move(otherComponent.LocalRotation);
				LocalScale = std::move(otherComponent.LocalScale);

				ConstantBufferHandle = std::move(otherComponent.ConstantBufferHandle);

				//Invalidate the moved from constant buffer ID to catch any misuse:
				otherComponent.ConstantBufferHandle = NULL_RESOURCE_HANDLE;
			}

			return *this;
		}

		DirectX::XMFLOAT4X4 Transform;
		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT3 Rotation;
		DirectX::XMFLOAT3 Scale;

		DirectX::XMFLOAT4X4 LocalTransform;
		DirectX::XMFLOAT3 LocalTranslation;
		DirectX::XMFLOAT3 LocalRotation;
		DirectX::XMFLOAT3 LocalScale;

		ResourceHandle ConstantBufferHandle = NULL_RESOURCE_HANDLE;
	};

	struct DirtyTransformComponent
	{
		DirtyTransformComponent()
			: Updates{ GPUTaskManager::FRAMES_IN_FLIGHT },
			  AdjustedWorldSpace{true},
			  OnlyUpload{false}
		{}

		uint32_t Updates;
		bool AdjustedWorldSpace;
		bool OnlyUpload;
	};

	struct NameComponent
	{
		explicit NameComponent(const char* name)
			: Name{name}
		{ }

		std::string Name;
	};

	struct MeshFilterComponent
	{
		MeshFilterComponent()
			: AssetHandle{ NULL_HANDLE }
		{}

		AssetHandle AssetHandle;
	};

	struct MeshRendererComponent
	{
		MeshRendererComponent()
			: AssetHandle{ NULL_HANDLE }
		{}

		AssetHandle AssetHandle;
	};

	struct DirtyMeshRendererComponent
	{
		DirtyMeshRendererComponent()
			: Updates{ GPUTaskManager::FRAMES_IN_FLIGHT }
		{}

		uint32_t Updates;
	};

	struct OpaquePassComponent
	{
		//ID
	};

	struct IDComponent
	{
		IDComponent()
		{
			UuId = CreateUUID();
		}
		
		IDComponent(const UUID& id)
			: UuId{id}
		{

		}

		UUID UuId;
	};

	struct DirectionalLightComponent
	{
		explicit DirectionalLightComponent(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			: Direction{ 0.0f, 0.0f, 0.0f },
			  Intensity{1.0f},
			  Color{color}
		{}

		DirectX::XMFLOAT3 Direction;
		float Intensity;
		DirectX::XMFLOAT3 Color;
	};

	struct PointLightComponent
	{
		explicit PointLightComponent(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			:Position{0.0f, 0.0f, 0.0f},
			 Intensity{ 1.0f },
			 Color{ color }
		{}

		DirectX::XMFLOAT3 Position;
		float Intensity;
		DirectX::XMFLOAT3 Color;
	};

	struct CameraComponent
	{
		CameraComponent()
			: ViewMatrix{},
			  ProjectionMatrix{},
			  FieldOfViewDegrees{60.0f},
			  ClippingPlaneNear{0.3f},
			  ClippingPlaneFar{1000.0f},
			  ClearColor{ DirectX::XMFLOAT4(DirectX::Colors::CornflowerBlue) },
			  IsMainCamera{false}
		{}
		DirectX::XMFLOAT4X4 ViewMatrix;
		DirectX::XMFLOAT4X4 ProjectionMatrix;
		float FieldOfViewDegrees;
		float ClippingPlaneNear;
		float ClippingPlaneFar;
		DirectX::XMFLOAT4 ClearColor;
		bool IsMainCamera;
	};

	struct RootComponent
	{
		//ID
	};

	struct ParentComponent
	{
		std::vector<entity> Children;
	};

	struct IsChildComponent
	{
		explicit IsChildComponent() noexcept
			: Parent{ NULL_ENTITY }
		{} 

		entity Parent;
	};

	struct SelectedInEditorComponent
	{
		//ID
	};

	struct HiddenInGameComponent
	{
		//ID
	};
}