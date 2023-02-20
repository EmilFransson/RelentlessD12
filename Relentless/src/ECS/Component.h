#pragma once
#include "../Graphics/Resources/AssetManager.h"
#include "../Graphics/Resources/ConstantBuffer.h"
#include "../Graphics/D3D12Core.h"

namespace Relentless
{
	constexpr const uint32_t MAX_ENTITIES = 10'000u;
	constexpr const uint32_t NULL_ENTITY = MAX_ENTITIES;
	typedef uint32_t entity;

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
			:Color{ 1.0f, 0.0f, 0.0f }
		{
			constantBuffer = RLS_NEW ConstantBuffer(sizeof(DirectX::XMFLOAT3));
		}

		DirectX::XMFLOAT3 Color;
		ConstantBuffer* constantBuffer;
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
}