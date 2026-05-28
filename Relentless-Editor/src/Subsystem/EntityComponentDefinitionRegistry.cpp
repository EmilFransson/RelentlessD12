#include "EntityComponentDefinitionRegistry.h"

#include "Core/Editor.h"

#include <Relentless.h>

namespace Relentless
{
	std::vector<Ref<IEntityComponentDefinition>> EntityComponentDefinitionRegistry::GetAllComponentDefinitions() const noexcept
	{
		std::vector<Ref<IEntityComponentDefinition>> definitions;
		definitions.reserve(m_ComponentDefinitions.size());

		for (const auto& [id, definition] : m_ComponentDefinitions)
			definitions.push_back(definition);

		return definitions;
	}

	bool EntityComponentDefinitionRegistry::Exists(TypeIndex aType) const noexcept
	{
		return m_ComponentDefinitions.contains(aType);
	}

	Ref<IEntityComponentDefinition> EntityComponentDefinitionRegistry::GetDefinition(TypeIndex aType) const noexcept
	{
		if (!Exists(aType))
			return nullptr;

		return m_ComponentDefinitions.at(aType);
	}

	bool EntityComponentDefinitionRegistry::OnLoad(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept
	{
		//Core:
		Register<TransformComponent>(
			{ 
				.DisplayName = "Transform", 
				.Category = "Core",
				.Description = "Core component governing translation, rotation & scale.",
				.Icon = ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT, 
				.Flags = EEntityComponentFlags::None 
			});

		Register<NameComponent>(
			{ 
				.DisplayName = "Name", 
				.Category = "Core", 
				.Description = "Core component detailing an entity's name.",
				.Icon = ICON_FA_PEN, 
				.Flags = EEntityComponentFlags::None 
			});

		Register<IDComponent>(
			{ 
				.DisplayName = "ID", 
				.Category = "Core", 
				.Description = "Core component detailing an entity's unique ID.",
				.Icon = ICON_FA_HASHTAG, 
				.Flags = EEntityComponentFlags::None 
			});
		
		//Lighting:
		Register<PointLightComponent>(
			{ 
				.DisplayName = "Point Light", 
				.Category = "Lighting", 
				.Description = "A light component which emits light equally in all directions.",
				.Icon = ICON_FA_LIGHTBULB, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});
		
		Register<DirectionalLightComponent>(
			{ 
				.DisplayName = "Directional Light", 
				.Category = "Lighting",
				.Description = "A light component which emits light uniformly as parallel rays.",
				.Icon = ICON_FA_SUN, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});
		
		Register<SpotLightComponent>(
			{ 
				.DisplayName = "Spot Light", 
				.Category = "Lighting", 
				.Description = "A light component which emits light in a directional cone shape.",
				.Icon = ICON_FA_BULLSEYE, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});

		Register<SkyLightComponent>(
			{ 
				.DisplayName = "Sky Light", 
				.Category = "Lighting",
				.Description = "A light component which emits indirect light from an environment source.",
				.Icon = ICON_FA_CLOUD_SUN, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});

		//Environment:
		Register<SkyBoxComponent>(
			{ 
				.DisplayName = "Sky Box", 
				.Category = "Environment",
				.Description = "A component that represents the background environment.",
				.Icon = ICON_FA_GLOBE, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});

		//Rendering:
		Register<MeshRendererComponent>(
			{ 
				.DisplayName = "Mesh Renderer", 
				.Category = "Rendering",
				.Description = "A component that represents a entity's material.",
				.Icon = ICON_FA_PALETTE, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});
		Register<MeshFilterComponent>(
			{ 
				.DisplayName = "Mesh Filter", 
				.Category = "Rendering",
				.Description = "A component that represent an entity's mesh.",
				.Icon = ICON_FA_CUBE, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});

		//Post Process:
		Register<PostProcessVolumeComponent>(
			{ 
				.DisplayName = "Post Process Volume", 
				.Category = "Post Process",
				.Description = "A component for authoring a number of post process effects.",
				.Icon = ICON_FA_PAINTBRUSH, 
				.Flags = EEntityComponentFlags::ShowInEditor 
			});

		return true;
	}

	bool EntityComponentDefinitionRegistry::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

}