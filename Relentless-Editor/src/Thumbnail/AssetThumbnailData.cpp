#include "AssetThumbnailData.h"
#include "AssetThumbnailPool.h"

#include "Core/Editor.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "UI/Widgets/AssetThumbnail.h"

namespace Relentless
{
	AssetThumbnailData::AssetThumbnailData(const AssetData& aAssetData, const SharedPtr<AssetThumbnailPool>& aAssetThumbnailPool) noexcept
		:m_AssetData(aAssetData)
	{
		aAssetThumbnailPool->OnThumbnailRegenerated.Connect(this, &AssetThumbnailData::OnThumbnailRegenerated);
		m_pAssetThumbnailPool = aAssetThumbnailPool->GetWeakPtr();
		
		//Provides default thumbnail if not already cached, and enqueues thumbnail generation if needed
		m_Brush.BackingTexture = aAssetThumbnailPool->GetAssetThumbnail(m_AssetData);
		QueryAssetColor();
	}

	AssetThumbnailData::~AssetThumbnailData() noexcept
	{
		if (SharedPtr<AssetThumbnailPool> pAssetThumbnailPool = m_pAssetThumbnailPool.lock())
			pAssetThumbnailPool->OnThumbnailRegenerated.Detach(this);
	}

	const AssetData& AssetThumbnailData::GetAssetData() const noexcept
	{
		return m_AssetData;
	}

	const ThumbnailBrush& AssetThumbnailData::GetBrush() const noexcept
	{
		return m_Brush;
	}

	Ref<AssetThumbnail> AssetThumbnailData::MakeThumbnailWidget(const Vector2& aSize) const noexcept
	{
		return RLS_NEW AssetThumbnail(GetWeakPtr(), aSize);
	}

	void AssetThumbnailData::SetAssetData(const AssetData& aAssetData) noexcept
	{
		if (m_AssetData == aAssetData)
			return;

		SharedPtr<AssetThumbnailPool> pAssetThumbnailPool = m_pAssetThumbnailPool.lock();
		if (!pAssetThumbnailPool)
			return;

		m_AssetData = aAssetData;
		m_Brush.BackingTexture = pAssetThumbnailPool->GetAssetThumbnail(m_AssetData);
		
		QueryAssetColor();
		OnThumbnailBrushUpdated(m_Brush);
	}

	void AssetThumbnailData::OnThumbnailRegenerated(const AssetData& aAssetData, const Ref<Texture>& aTexture) noexcept
	{
		if (aAssetData != m_AssetData)
			return;

		m_Brush.BackingTexture = aTexture;
		OnThumbnailBrushUpdated(m_Brush);
	}

	void AssetThumbnailData::QueryAssetColor() noexcept
	{
		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		RLS_VERIFY(pAssetDefinitionRegistry, "[AssetThumbnailData::QueryAssetColor]: Asset Definition Registry Is Invalid.");

		const Ref<IAssetDefinition>& pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(m_AssetData);
		RLS_VERIFY(pAssetDefinition, "[AssetThumbnailData::QueryAssetColor]: Asset Definition for asset {0} Is Invalid.", m_AssetData.Name.c_str());
		m_Brush.TypeColor = pAssetDefinition->GetAssetColor();
	}
}
