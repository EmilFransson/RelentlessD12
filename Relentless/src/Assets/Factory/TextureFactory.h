#pragma once

#include "Callback/Broadcaster.h"
#include "Graphics/RHI/RHI.h"
#include "IFactory.h"

namespace Relentless
{
	class TextureFactory : public IFactory
	{
	public:
		Broadcaster<void(const ImportedAsset& importedAsset, bool success)> OnDone;

		void SetCompressionType(ETextureCompressionType compressionType) noexcept;
		void SetGenerateMipmaps(bool enable) noexcept;
		void SetImportAsSRGB(bool enable) noexcept;

	private:
		virtual void Execute(const Path& filePath, GraphicsDevice* pDevice) noexcept override;
		void Finalize(bool success) noexcept;
		[[nodiscard]] bool ImportTexture() noexcept;
	private:
		ImportedAsset m_ImportedTextureAsset;
		Path m_SrcPath;

		GraphicsDevice* m_pDevice = nullptr;

		ETextureCompressionType m_CompressionType = ETextureCompressionType::Uncompressed;
		bool m_GenerateMipmaps = true;
		bool m_IsSRGB = false;
	};
}