#include "RenderTypes.h"

#include "Graphics/RHI/Device.h"

namespace Relentless
{
	static std::array<Ref<Texture>, (uint32)DefaultTextureType::Max> s_DefaultTextures;

	namespace GraphicsCommon
	{
		void Create(GraphicsDevice* aGraphicsDevice) noexcept
		{
			auto CreateDefaultTexture = [aGraphicsDevice](DefaultTextureType aType, const char* aName, const TextureDesc& aTextureDesc, const void* aData)
				{
					const uint32 faceCount = (aTextureDesc.Type == TextureType::TextureCube) ? 6u : 1u;
					const uint32 rowPitch = RHI::GetRowPitch(aTextureDesc.Format, aTextureDesc.Width);
					const uint32 slicePitch = RHI::GetSlicePitch(aTextureDesc.Format, aTextureDesc.Width, aTextureDesc.Height);

					std::vector<D3D12_SUBRESOURCE_DATA> initData(faceCount);

					for (uint32 i = 0; i < faceCount; ++i)
					{
						initData[i].pData = static_cast<const uint8*>(aData) + i * slicePitch;
						initData[i].RowPitch = rowPitch;
						initData[i].SlicePitch = slicePitch;
					}

					s_DefaultTextures[(uint32)aType] = aGraphicsDevice->CreateTexture(aTextureDesc, aName, initData);
				};

			constexpr TextureFlag textureFlags = TextureFlag::ShaderResource;
			static const Vector4 WHITE = Vector4::One;
			static const Vector4 BLACK = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			static const Vector4 DEFAULT_NORMAL = Vector4(0.5f, 0.5f, 1.0f, 1.0f);
			static const Vector4 BLACK_CUBE[6] = { BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };
			static const Vector4 WHITE_CUBE[6] = { WHITE, WHITE, WHITE, WHITE, WHITE, WHITE };

			CreateDefaultTexture(DefaultTextureType::White2D, "Default White", TextureDesc::Create2D(1u, 1u, ResourceFormat::RGBA32_FLOAT, 1u, textureFlags), &WHITE);
			CreateDefaultTexture(DefaultTextureType::Black2D, "Default Black", TextureDesc::Create2D(1u, 1u, ResourceFormat::RGBA32_FLOAT, 1u, textureFlags), &BLACK);
			CreateDefaultTexture(DefaultTextureType::Normal2D, "Default Normal", TextureDesc::Create2D(1u, 1u, ResourceFormat::RGBA32_FLOAT, 1u, textureFlags), &DEFAULT_NORMAL);
			CreateDefaultTexture(DefaultTextureType::BlackCube, "Default Black Cube", TextureDesc::CreateCube(1u, 1u, ResourceFormat::RGBA32_FLOAT, 1u, textureFlags), BLACK_CUBE);
			CreateDefaultTexture(DefaultTextureType::WhiteCube, "Default White Cube", TextureDesc::CreateCube(1u, 1u, ResourceFormat::RGBA32_FLOAT, 1u, textureFlags), WHITE_CUBE);
		}

		void Destroy() noexcept
		{
			for (auto& pTexture : s_DefaultTextures)
				pTexture.Reset();
		}

		Texture* GetDefaultTexture(DefaultTextureType aDefaultTextureType) noexcept
		{
			return s_DefaultTextures[(uint32)aDefaultTextureType];
		}

	}
}