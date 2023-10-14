#include "FileDialogs.h"
#include "../Core/Window.h"

namespace Relentless
{
	std::string FileDialogs::OpenFile(const char* filter) noexcept
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAMEA));
		openFileName.lStructSize = sizeof(OPENFILENAMEA);
		openFileName.hwndOwner = Window::GetHandle();
		openFileName.lpstrFile = szFile;
		openFileName.nMaxFile = sizeof(szFile);
		openFileName.lpstrFilter = filter;
		openFileName.nFilterIndex = 1;
		openFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&openFileName) == TRUE)
		{
			return openFileName.lpstrFile;
		}
		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter) noexcept
	{
		OPENFILENAMEA openFileName;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&openFileName, sizeof(OPENFILENAMEA));
		openFileName.lStructSize = sizeof(OPENFILENAMEA);
		openFileName.hwndOwner = Window::GetHandle();
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