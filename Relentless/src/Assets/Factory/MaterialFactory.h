#pragma once
#include "IFactory.h"

namespace Relentless
{
	class MaterialFactory : public IFactory
	{
	public:
		virtual NO_DISCARD bool CanCreateNew() const noexcept override;
		virtual NO_DISCARD bool CanImport(const Path& aPath) const noexcept override;
		virtual Ref<IFactory> Clone() noexcept override;
		virtual AssetHandle CreateNew(const String& aName, const Path& aPackagePath) noexcept override;
		virtual NO_DISCARD String GetDefaultNewAssetName() const noexcept override;
		virtual NO_DISCARD bool DoesSupportAsset(IAsset* aAsset) const noexcept override;
	};
}