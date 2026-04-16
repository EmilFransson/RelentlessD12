#include "AssetDetailPropertyRowBuilder.h"

#include "UI/Nodes/AssetDetailNode.h"

namespace Relentless
{
	AssetDetailPropertyRowBuilder::AssetDetailPropertyRowBuilder(const char* aLabel, AssetDetailNode* aDetailNode) noexcept
		: m_pDetailNode(aDetailNode)
	{
		m_NameSlot.Details.Label = aLabel;
		m_ValueSlot.Details.Label = aLabel;
		m_NameSlot.Details.Range = TVector2<double>(std::numeric_limits<double>::lowest() / 2.0, std::numeric_limits<double>::max() / 2.0);
		m_ValueSlot.Details.Range = TVector2<double>(std::numeric_limits<double>::lowest() / 2.0, std::numeric_limits<double>::max() / 2.0);
	}

	AssetDetailPropertyRowBuilder::~AssetDetailPropertyRowBuilder() noexcept
	{
		if (!m_HasBuilt)
			Build();
	}

	AssetDetailPropertyRowBuilder& AssetDetailPropertyRowBuilder::AcceptableAssetTypes(Span<TypeIndex> someTypes) noexcept
	{
		m_AcceptableAssetTypes = someTypes.Copy();
		return *this;
	}

	AssetDetailPropertyRowBuilder& AssetDetailPropertyRowBuilder::OnAssetsDropped(Callback<void(Span<const AssetData>)>&& aCallback) noexcept
	{
		m_OnAssetsDroppedCallback = std::move(aCallback);
		return *this;
	}

	void AssetDetailPropertyRowBuilder::Build() noexcept
	{

	}

}