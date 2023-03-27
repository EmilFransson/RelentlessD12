#pragma once
#include "../Graphics/Resources/AssetManager.h"
#include "../Graphics/Resources/ConstantBuffer.h"
#include "../Graphics/D3D12Core.h"

namespace Relentless
{
	/*! @brief Invalid entity alias. */
	#define NULL_ENTITY (std::numeric_limits<uint32_t>::max() << 12)

	/*! @brief Opaque entity type. */
	using entity = uint32_t;

	struct TransformComponent
	{
		TransformComponent()
			: Translation{ 0.0f, 0.0f, 0.0f },
			  Rotation{ 0.0f, 0.0f, 0.0f },
			  Scale{ 1.0f, 1.0f, 1.0f }
		{
			DirectX::XMStoreFloat4x4(&Transform, DirectX::XMMatrixIdentity());
		}
		DirectX::XMFLOAT4X4 Transform;
		DirectX::XMFLOAT3 Translation;
		DirectX::XMFLOAT3 Rotation;
		DirectX::XMFLOAT3 Scale;
	};

	struct DirtyTransformComponent
	{
		DirtyTransformComponent()
			: Updates{ D3D12Core::GetNrOfBufferedFrames() }
		{}

		uint32_t Updates;
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
		ResourceID VertexBufferID;
		ResourceID IndexBufferID;
	};

	struct MeshRendererComponent
	{
		MeshRendererComponent()
			:Color{ 1.0f, 0.0f, 0.0f },
			 constantBufferID{ static_cast<size_t>(-1) }
		{}

		DirectX::XMFLOAT3 Color;
		size_t constantBufferID;
	};

	struct DirtyMeshRendererComponent
	{
		DirtyMeshRendererComponent()
			: Updates{ D3D12Core::GetNrOfBufferedFrames() }
		{}

		uint32_t Updates;
	};

	struct ForwardPassComponent
	{
		//ID
	};

	struct IDComponent
	{
		IDComponent()
		{
			#if defined RLS_DEBUG
			RLS_ASSERT(UuidCreate(&UuId) == RPC_S_OK, "Failed to generate UUID.");
			#else
			UuidCreate(&UuId);
			#endif
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

	struct DirtyLightComponent
	{
		DirtyLightComponent()
			: Updates{ D3D12Core::GetNrOfBufferedFrames() }
		{}

		uint32_t Updates;
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
			: Parent{ NULL_ENTITY },
			LocalTranslation{ 0.0f, 0.0f, 0.0f },
			LocalRotation{ 0.0f, 0.0f, 0.0f },
			LocalScale{ 1.0f, 1.0f, 1.0f }
		{
			DirectX::XMStoreFloat4x4(&LocalTransform, DirectX::XMMatrixIdentity());
		} 

		entity Parent;

		DirectX::XMFLOAT4X4 LocalTransform;
		DirectX::XMFLOAT3 LocalTranslation;
		DirectX::XMFLOAT3 LocalRotation;
		DirectX::XMFLOAT3 LocalScale;
	};
}