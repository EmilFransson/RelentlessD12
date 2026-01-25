#pragma once
#include <Relentless.h>
#include <StaticTypeInfo/type_index.h>

#include "../Assets/Definition/IAssetDefinition.h"

namespace Relentless
{
	class AssetDefinitionRegistry : public ISubsystem
	{
	public:
		NO_DISCARD std::vector<Ref<IAssetDefinition>> GetAllAssetDefinitions() const noexcept;
		NO_DISCARD const Ref<IAssetDefinition>& GetDefinitionForAsset(const AssetData& aAssetData) const noexcept;
		NO_DISCARD const Ref<IAssetDefinition>& GetDefinitionForAsset(const IAsset* aAsset) const noexcept;

		NO_DISCARD virtual bool OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;
		
		void RegisterAssetDefinition(const Ref<IAssetDefinition>& aAssetDefinition) noexcept;
		
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	
		void UnregisterAssetDefinition(const Ref<IAssetDefinition>& aAssetDefinition) noexcept;
	private:
		std::unordered_map<TypeIndex, Ref<IAssetDefinition>> m_AssetDefinitions;
	};
}