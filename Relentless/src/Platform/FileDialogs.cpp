#include "FileDialogs.h"

#include "Core/Application.h"
#include "Core/Window.h"

namespace Relentless::Platform
{
	std::vector<Path> OpenFileDialog()
	{
		HWND hwnd = Application::Get().GetWindow()->GetNativeWindow();

		Microsoft::WRL::ComPtr<IFileOpenDialog> pDialog = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog))))
			return {};

		const COMDLG_FILTERSPEC filters[] = 
		{
			{ L"Textures", L"*.png;*.tga;*.dds;*.hdr" },
			{ L"Meshes",   L"*.fbx;*.gltf;*.glb;*.obj" },
			{ L"All files", L"*.*" },
		};
		pDialog->SetFileTypes(ARRAYSIZE(filters), filters);
		pDialog->SetFileTypeIndex(1);

		DWORD flags = 0;
		pDialog->GetOptions(&flags);
		pDialog->SetOptions(flags | FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM);

		if (FAILED(pDialog->Show(hwnd)))
			return {};

		Microsoft::WRL::ComPtr<IShellItemArray> pItems = nullptr;
		if (FAILED(pDialog->GetResults(&pItems)))
			return {};

		DWORD numItems = 0u;
		if (FAILED(pItems->GetCount(&numItems)))
			return {};

		std::vector<Path> paths;
		paths.reserve(numItems);

		for (DWORD i = 0u; i < numItems; ++i)
		{
			Microsoft::WRL::ComPtr<IShellItem> pItem = nullptr;
			if (FAILED(pItems->GetItemAt(i, &pItem)))
				continue;

			PWSTR path = nullptr;
			if (FAILED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &path)))
				continue;

			paths.push_back(Path(path));
			::CoTaskMemFree(path);
		}

		return paths;
	}

	bool ShowInExplorer(const Path& aPath)
	{
		const WideString wideString = aPath.native();

		PIDLIST_ABSOLUTE pidl = ::ILCreateFromPathW(wideString.c_str());
		if (!pidl)
			return false;

		const HRESULT hr = ::SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
		::ILFree(pidl);

		return SUCCEEDED(hr);
	}
}