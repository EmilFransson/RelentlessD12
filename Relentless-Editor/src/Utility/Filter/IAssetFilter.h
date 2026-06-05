#pragma once
#include <Relentless.h>

#include "Subsystem/AssetDefinitionRegistry.h"

namespace Relentless
{
	class IAssetFilter
	{
	public:
		IAssetFilter() noexcept;
		virtual ~IAssetFilter() = default;
		NO_DISCARD virtual bool PassesFilter(const AssetData& aData) const = 0;
		NO_DISCARD virtual bool IsActive() const = 0;

		Broadcaster<void()> OnChanged;
	protected:
		AssetDefinitionRegistry& m_AssetDefinitionRegistry;
	};
}