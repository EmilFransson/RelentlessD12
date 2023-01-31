#pragma once
#include "../Graphics/Resources/AssetManager.h"
#include "../Graphics/Resources/ConstantBuffer.h"

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
		NameComponent(const char* name)
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
			: Color{color},
			  Intensity{1.0f}
		{
			//We multiply by 2 as we make room for the direction too, however this is part of the "DirectionalLightStruct type"
			constantBuffer = RLS_NEW ConstantBuffer((sizeof(DirectX::XMFLOAT3) * 2) + sizeof(float));
		}

		DirectX::XMFLOAT3 Color;
		float Intensity;

		ConstantBuffer* constantBuffer;
	};

	struct PointLightComponent
	{
		explicit PointLightComponent(const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			:Color{color},
			 Intensity{1.0f}
		{
			//We multiply by 2 as we make room for the position too, however this is part of the "PointLightStruct type"
			constantBuffer = RLS_NEW ConstantBuffer((sizeof(DirectX::XMFLOAT3) * 2) + sizeof(float));
		}

		DirectX::XMFLOAT3 Color;
		float Intensity;

		ConstantBuffer* constantBuffer;
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