#pragma once
#include "Callback/Callback.h"
namespace Relentless
{
	class Texture2D;
	class TextureCube;

	class UtilityRenderer
	{
	public:
		using EquirectangularToCubeMapConversionCompleteCallback = Callback<void(std::shared_ptr<TextureCube>)>;
		using CreateIrradianceMapCompleteCallback = Callback<void(std::shared_ptr<TextureCube>)>;
		using CreateRadianceMapCompleteCallback = Callback<void(std::shared_ptr<TextureCube>)>;

		UtilityRenderer();
		~UtilityRenderer() = default;
		void ConvertEquirectangularToCubeMap(const std::shared_ptr<Texture2D> pEquirectangularTexture, EquirectangularToCubeMapConversionCompleteCallback&& callback);
		void CreateIrradianceMap(const std::shared_ptr<TextureCube> pEnvironmentTextureCube, CreateIrradianceMapCompleteCallback&& callback);
		void CreateRadianceMap(const std::shared_ptr<TextureCube> pEnvironmentTextureCube, CreateRadianceMapCompleteCallback&& callback);
	private:
		//std::shared_ptr<RenderPass> m_pEquirectangularToCubeMapPass = nullptr;
		//std::shared_ptr<RenderPass> m_pTextureCubeIrradianceConvolutionPass = nullptr;
		//std::shared_ptr<RenderPass> m_pTextureCubeRadianceConvolutionPass = nullptr;

		struct EquirectangularToCubeMapPassData
		{
			uint32_t TextureIndex = 0u;
			uint32_t ViewProjectionIndex = 0u;
		} m_EquirectangularToCubeMapPassData;

		struct TextureCubeCreationViewProjectionMatrix
		{
			DirectX::XMFLOAT4X4 VPMatrix;
		} m_TextureCubeCreationViewProjectionMatrices[6];

		struct RadianceRoughnessData
		{
			float Roughness = 0.0f;
		} m_RadianceRoughnessData;

		struct RadianceRoughnessDataIndex
		{
			uint32_t RoughnessIndex = 0u;
		} m_RadianceRoughnessIndexData;

		//std::unique_ptr<ConstantBufferSet> m_pTextureCubeCreationVPCBs[6];
		//std::unique_ptr<ConstantBufferSet> m_pRadianceMapRougnessCBSets[5];
	};
}