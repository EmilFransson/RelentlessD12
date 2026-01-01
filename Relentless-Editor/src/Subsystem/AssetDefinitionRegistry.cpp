#include "AssetDefinitionRegistry.h"
#include "../Assets/Definition/MaterialAssetDefinition.h"
#include "../Assets/Definition/MeshAssetDefinition.h"
#include "../Assets/Definition/Texture2DAssetDefinition.h"
#include "../Core/Editor.h"

namespace Relentless
{
	std::vector<Ref<IAssetDefinition>> AssetDefinitionRegistry::GetAllAssetDefinitions() const noexcept
	{
		std::vector<Ref<IAssetDefinition>> definitions;
		definitions.reserve(m_AssetDefinitions.size());

		for (const auto& [id, definition] : m_AssetDefinitions)
			definitions.push_back(definition);

		return definitions;
	}

	const Ref<IAssetDefinition>& AssetDefinitionRegistry::GetDefinitionForAsset(const AssetData& aAssetData) const noexcept
	{
		if (m_AssetDefinitions.contains(aAssetData.Type))
			return m_AssetDefinitions.at(aAssetData.Type);

		return nullptr;
	}

	const Ref<IAssetDefinition>& AssetDefinitionRegistry::GetDefinitionForAsset(const IAsset* aAsset) const noexcept
	{
		const TypeIndex& typeIndex = aAsset->GetStaticType();

		if (m_AssetDefinitions.contains(typeIndex))
			return m_AssetDefinitions.at(typeIndex);

		return nullptr;
	}

	bool AssetDefinitionRegistry::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		RegisterAssetDefinition(new MaterialAssetDefinition());
		RegisterAssetDefinition(new MeshAssetDefinition());
		RegisterAssetDefinition(new Texture2DAssetDefinition());

		return true;
	}

	void AssetDefinitionRegistry::RegisterAssetDefinition(const Ref<IAssetDefinition>& aAssetDefinition) noexcept
	{
		const TypeIndex typeID = aAssetDefinition->GetSupportedAssetType();

		if (m_AssetDefinitions.contains(typeID))
			return;

		m_AssetDefinitions.emplace(typeID, aAssetDefinition);
	}

	bool AssetDefinitionRegistry::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	void AssetDefinitionRegistry::UnregisterAssetDefinition(const Ref<IAssetDefinition>& aAssetDefinition) noexcept
	{
		const TypeIndex typeID = aAssetDefinition->GetSupportedAssetType();

		if (m_AssetDefinitions.contains(typeID))
			m_AssetDefinitions.erase(typeID);
	}
}