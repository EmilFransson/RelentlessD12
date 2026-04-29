#pragma once
#include <Relentless.h>

#include "FactoryBase.h"

namespace Relentless
{
	class MaterialFactory : public FactoryBase
	{
	public:
		NO_DISCARD virtual bool CanCreateNew() const noexcept override;
		NO_DISCARD virtual bool CanImport(MAYBE_UNUSED const Path& aPath) const noexcept override;
		virtual Ref<IFactory> Clone() noexcept override;
		virtual FactoryCreateResult CreateNew(MAYBE_UNUSED const TypeIndex& aType, const String& aName, const UUID& aUUID = CreateUUID()) noexcept override;
		NO_DISCARD virtual String GetDefaultNewAssetName() const noexcept override;
		NO_DISCARD virtual bool DoesSupportAsset(IAsset* aAsset) const noexcept override;
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
	};
}