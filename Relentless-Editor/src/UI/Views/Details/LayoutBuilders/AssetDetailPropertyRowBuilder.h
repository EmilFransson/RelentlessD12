#pragma once
#include <Relentless.h>

#include "DetailPropertyRowBuilder.h"

#include "SlotBuilder.h"

namespace Relentless
{
	class AssetDetailNode;

	class AssetDetailPropertyRowBuilder
	{
	public:
		explicit AssetDetailPropertyRowBuilder(const char* aLabel, AssetDetailNode* aDetailNode) noexcept;
		virtual ~AssetDetailPropertyRowBuilder() noexcept;
		
		AssetDetailPropertyRowBuilder& AcceptableAssetTypes(Span<TypeIndex> someTypes) noexcept;
		AssetDetailPropertyRowBuilder& OnAssetsDropped(Callback<void(Span<const AssetData>)>&& aCallback) noexcept;
	private:
		void Build() noexcept;
	private:
		SlotContent m_NameSlot;
		SlotContent m_ValueSlot;
		SlotContent m_RevertSlot;

		std::vector<TypeIndex> m_AcceptableAssetTypes;

		Callback<void()> m_OnRevertCallback;
		Callback<void()> m_OnRevertedCallback;
		Callback<void(Span<const AssetData>)> m_OnAssetsDroppedCallback;

		AssetDetailNode* m_pDetailNode = nullptr;
		bool m_HasBuilt = false;
	};
}