#include "Project.h"

#include "File/File.h"

#include "Module/AssetRegistryModule.h"
#include "Module/ModuleManager.h"

#include "Utility/FilepathUtils.h"

#include "yaml-cpp/yaml.h"

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

		s_ActiveProject->m_ActiveProjectDirectory = aPath.parent_path().string() + "/";
		
		FilepathUtils::CreateDirectoryTree(GetAssetDirectory());
		FilepathUtils::CreateDirectoryTree(GetThumbnailCacheDirectory());

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistryModule.RegisterRoot(GetAssetDirectory(), EAssetSourceType::Project);
		assetRegistryModule.ScanForAssets(GetAssetDirectory());

		return s_ActiveProject;
	}

	Ref<Project> Project::LoadOrCreateDefault() noexcept
	{
		ProjectConfig config{};

		const Path baseDir = FilepathUtils::Combine(Path(PROJECT_BUILD_DIRECTORY), std::format("{}/", config.Name));
		const Path projectFile = FilepathUtils::Combine(baseDir, std::format("{}.rproject", config.Name));

		if (File::Exists(projectFile))
		{
			if (Ref<Project> loaded = Load(projectFile))
				return loaded;

			RLS_CORE_WARN("[Project::LoadOrCreateDefault]: Default project at '{0}' failed to load; recreating.", projectFile.string());
		}

		RLS_CORE_INFO("[Project::LoadOrCreateDefault]: Creating default project at '{0}'.", baseDir.string());

		New(config);

		s_ActiveProject->m_ActiveProjectDirectory = baseDir;

		FilepathUtils::CreateDirectoryTree(baseDir);
		FilepathUtils::CreateDirectoryTree(GetAssetDirectory());
		FilepathUtils::CreateDirectoryTree(GetThumbnailCacheDirectory());

		SaveActive(baseDir);

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistryModule.RegisterRoot(GetAssetDirectory(), EAssetSourceType::Project);
		assetRegistryModule.ScanForAssets(GetAssetDirectory());

		return s_ActiveProject;
	}
	
	Ref<Project> Project::New(const ProjectConfig& aConfig) noexcept
	{
		s_ActiveProject = RLS_NEW Project();
		s_ActiveProject->m_Config = aConfig;

		return s_ActiveProject;
	}

	bool Project::SaveActive(const Path& aPath) noexcept
	{
		if (!FilepathUtils::CreateDirectoryTree(aPath))
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

		const Path destination = FilepathUtils::Combine(aPath, s_ActiveProject->GetName() + ".rproject");
		std::ofstream fileOut(destination);
		fileOut << out.c_str();

		return true;
	}
}