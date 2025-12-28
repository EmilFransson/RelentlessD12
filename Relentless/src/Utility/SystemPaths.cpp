#include "SystemPaths.h"
namespace Relentless
{
	struct SystemPathsData
	{
		Path WorkingDirectory;
		Path UserDocumentsDirectory;
		Path UserHomeDirectory;
		Path EngineAssetsDirectory;
		Path EditorAssetsDirectory;
	};

	static SystemPathsData s_SystemPathsData{};

	static Path GetKnownFolder(REFKNOWNFOLDERID folderID)
	{
		PWSTR path = nullptr;
		if (SUCCEEDED(::SHGetKnownFolderPath(folderID, 0, nullptr, &path)))
		{
			Path result(path);
			::CoTaskMemFree(path);
			return result;
		}
		return {};
	}

	void SystemPaths::Initialize() noexcept
	{
		const Path coreEnginePath = MAIN_ENGINE_DIRECTORY;
		s_SystemPathsData.WorkingDirectory = coreEnginePath.parent_path().parent_path();
		s_SystemPathsData.EngineAssetsDirectory = s_SystemPathsData.WorkingDirectory / "Relentless\\Assets";
		s_SystemPathsData.EditorAssetsDirectory = s_SystemPathsData.WorkingDirectory / "Relentless-Editor\\Assets";
		s_SystemPathsData.UserHomeDirectory = GetKnownFolder(::FOLDERID_Desktop);

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

	Path SystemPaths::GetWorkingDirectory() noexcept
	{
		return s_SystemPathsData.WorkingDirectory;
	}

	Path SystemPaths::GetUserDocumentsDirectory() noexcept
	{
		return s_SystemPathsData.UserDocumentsDirectory;
	}

	Path SystemPaths::GetUserHomeDirectory() noexcept
	{
		return s_SystemPathsData.UserHomeDirectory;
	}

	Path SystemPaths::GetEngineAssetsDirectory() noexcept
	{
		return s_SystemPathsData.EngineAssetsDirectory;
	}

	Path SystemPaths::GetEditorAssetsDirectory() noexcept
	{
		return s_SystemPathsData.EditorAssetsDirectory;
	}
}