#include "SystemPaths.h"
namespace Relentless
{
	struct SystemPathsData
	{
		std::filesystem::path WorkingDirectory;
		std::filesystem::path UserDocumentsDirectory;
		std::filesystem::path EngineAssetsDirectory;
		std::filesystem::path EditorAssetsDirectory;
	};

	static SystemPathsData s_SystemPathsData{};

	void SystemPaths::Initialize() noexcept
	{
		const std::filesystem::path coreEnginePath = MAIN_ENGINE_DIRECTORY;
		s_SystemPathsData.WorkingDirectory = coreEnginePath.parent_path().parent_path();
		s_SystemPathsData.EngineAssetsDirectory = s_SystemPathsData.WorkingDirectory / "Relentless\\Assets";
		s_SystemPathsData.EditorAssetsDirectory = s_SystemPathsData.WorkingDirectory / "Relentless-Editor\\Assets";

		char documentsPath[MAX_PATH] = { 0 };
		if (::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, documentsPath) == S_OK)
		{
			s_SystemPathsData.UserDocumentsDirectory = std::filesystem::path(documentsPath) / "Relentless Engine";
			if (!std::filesystem::exists(s_SystemPathsData.UserDocumentsDirectory))
				std::filesystem::create_directory(s_SystemPathsData.UserDocumentsDirectory);
		}
		else
		{
			RLS_ASSERT(false, "Error retrieving user documents path.");
		}
	}

	std::filesystem::path SystemPaths::GetWorkingDirectory() noexcept
	{
		return s_SystemPathsData.WorkingDirectory;
	}

	std::filesystem::path SystemPaths::GetUserDocumentsDirectory() noexcept
	{
		return s_SystemPathsData.UserDocumentsDirectory;
	}

	std::filesystem::path SystemPaths::GetEngineAssetsDirectory() noexcept
	{
		return s_SystemPathsData.EngineAssetsDirectory;
	}

	std::filesystem::path SystemPaths::GetEditorAssetsDirectory() noexcept
	{
		return s_SystemPathsData.EditorAssetsDirectory;
	}
}