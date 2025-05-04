#pragma once

#include "Assets/AssetMeta.h"
#include "Assets/ImportSettings.h"
#include "Core/IAsset.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	enum class EExtensionType : uint8_t { UNKNOWN = 0, TGA, JPG, JPEG, PNG, BMP, DDS, TIFF, HDR, EXR, FBX, OBJ, GLTF };
	
	struct ImportedAsset
	{
		IAsset* pAsset = nullptr;
		AssetHandle Handle = NULL_HANDLE;
		AssetType Type = AssetType::Undefined;
	};

	class IFactory : public RefCounted<IFactory>
	{
	public:
		virtual ~IFactory() = default;
		virtual void Execute(const Path& filePath, GraphicsDevice* pDevice) noexcept = 0;
	};
}