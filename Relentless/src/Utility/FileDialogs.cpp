#include "FileDialogs.h"
#include "Core/Application.h"
#include "Graphics/RHI/Window.h"

namespace Relentless
{
	std::vector<std::string> FileDialogs::OpenFile(const char* filter) noexcept
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[2048] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAMEA));
		openFileName.lStructSize = sizeof(OPENFILENAMEA);
		openFileName.hwndOwner = Application::Get().GetWindow()->GetNativeWindow();
		openFileName.lpstrFile = szFile;
		openFileName.nMaxFile = sizeof(szFile);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		
		std::vector<std::string> files;
		if (GetOpenFileNameA(&openFileName) == TRUE)
		{
			// Process the buffer to split the directory and filenames
			char* p = openFileName.lpstrFile;
			std::string directoryPath = p;
			p += directoryPath.length() + 1; // Move past the directory path and the null terminator
			if (*p == 0) 
			{
				// Only one file selected, directoryPath is the full path
				files.push_back(directoryPath);
			}
			else 
			{
				// Multiple files selected, concatenate directory path with each filename
				while (*p) 
				{
					std::string filePath = directoryPath + "\\" + p;
					files.push_back(filePath);
					p += strlen(p) + 1; // Move to the next file name
				}
			}
		}
		return files;
	}

	std::string FileDialogs::SaveFile(const char* filter) noexcept
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAMEA));
		openFileName.lStructSize = sizeof(OPENFILENAMEA);
		openFileName.hwndOwner = Application::Get().GetWindow()->GetNativeWindow();
		openFileName.lpstrFile = szFile;
		openFileName.nMaxFile = sizeof(szFile);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}
		return std::string();
	}

}