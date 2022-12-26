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

	struct LightComponent
	{
		enum class Type : uint8_t { Directional = 0u };
		
		LightComponent(Type type, const DirectX::XMFLOAT3& color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f))
			:LightType{ type },
			 Color{ color }
		{}

		Type LightType;
		DirectX::XMFLOAT3 Color;
	};
}