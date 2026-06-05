#include "IAssetFilter.h"

#include "Core/Editor.h"

namespace Relentless
{
	IAssetFilter::IAssetFilter() noexcept
		: m_AssetDefinitionRegistry(*Editor::Get()->GetSubsystem<AssetDefinitionRegistry>())
	{}
}