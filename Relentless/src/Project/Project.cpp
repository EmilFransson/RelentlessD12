#include "Project.h"
#include "Utility/FilepathUtils.h"
#include "yaml-cpp/yaml.h"
#include "Module/AssetRegistryModule.h"
#include "Module/ModuleManager.h"

namespace Relentless
{
	Path Project::GetAssetDirectory() noexcept
	{
		RLS_ASSERT(s_ActiveProject, "[Project::GetAssetDirectory]: Project is invalid.");
		return FilepathUtils::Combine(s_ActiveProject->m_ActiveProjectDirectory, s_ActiveProject->GetConfig().AssetPath);
	}

	Path Project::GetThumbnailCacheDirectory() noexcept
	{
		RLS_ASSERT(s_ActiveProject, "[Project::GetThumbnailCacheDirectory]: Project is invalid.");
		return FilepathUtils::Combine(s_ActiveProject->m_ActiveProjectDirectory, s_ActiveProject->GetConfig().ThumbnailCachePath);
	}

	ProjectConfig& Project::GetConfig() noexcept
	{
		return m_Config;
	}

	const String& Project::GetName() noexcept
	{
		RLS_ASSERT(s_ActiveProject, "[Project::GetName]: Project is invalid.");
		return s_ActiveProject->GetConfig().Name;
	}

	const Path& Project::GetProjectDirectory() noexcept
	{
		RLS_ASSERT(s_ActiveProject, "[Project::GetProjectDirectory]: Project is invalid.");
		return s_ActiveProject->m_ActiveProjectDirectory;
	}

	Ref<Project> Project::Load(const Path& aPath) noexcept
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(aPath.string());
		}
		catch (YAML::ParserException aException)
		{
			RLS_CORE_ERROR("Failed to load project file '{0}' with error:\n{1}", aPath, aException.what());
			return nullptr;
		}

		auto projectNode = data["Project"];
		if (!projectNode)
			return nullptr;

		if (!s_ActiveProject)
			New({});

		auto& config = s_ActiveProject->GetConfig();
		config.Name = projectNode["Name"].as<String>();
		config.AssetPath = projectNode["AssetDirectory"].as<String>();
		config.ThumbnailCachePath = projectNode["ThumbnailCacheDirectory"].as<String>();

		s_ActiveProject->m_ActiveProjectDirectory = aPath.parent_path();
		
		FilepathUtils::CreateDirectoryTree(GetAssetDirectory());
		FilepathUtils::CreateDirectoryTree(GetThumbnailCacheDirectory());

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistryModule.RegisterRoot(GetAssetDirectory(), EAssetSourceType::Project);
		assetRegistryModule.ScanForAssets(GetAssetDirectory());

		return s_ActiveProject;
	}

	bool Project::SaveActive(const Path& aPath) noexcept
	{
		Path directory = FilepathUtils::Combine(aPath, s_ActiveProject->GetName());
		directory += "\\";

		if (!FilepathUtils::CreateDirectoryTree(directory))
			return false;

		const ProjectConfig& config = s_ActiveProject->GetConfig();

		YAML::Emitter out;
		{
			out << YAML::BeginMap; //Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; //Project
				out << YAML::Key << "Name" << config.Name;
				out << YAML::Key << "AssetDirectory" << config.AssetPath.string();
				out << YAML::Key << "ThumbnailCacheDirectory" << config.ThumbnailCachePath.string();
				out << YAML::EndMap; //Project
			}
			out << YAML::EndMap; //Root
		}

		const Path destination = FilepathUtils::Combine(directory, s_ActiveProject->GetName() + ".rproject");
		std::ofstream fileOut(destination);
		fileOut << out.c_str();

		return true;
	}

	Ref<Project> Project::New(const ProjectConfig& aConfig) noexcept
	{
		s_ActiveProject = new Project();
		s_ActiveProject->m_Config = aConfig;

		return s_ActiveProject;
	}
}