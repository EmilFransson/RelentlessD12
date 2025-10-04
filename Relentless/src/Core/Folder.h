#pragma once
#include "Assets/AssetMeta.h"

namespace Relentless
{
	class Scene;

	struct FolderRoot
	{
		enum class Type { None, Scene };

		UUID UUID = NULL_UUID;
		Type Type = Type::None;

		bool operator==(const FolderRoot& aOtherRoot) const noexcept
		{
			return UUID == aOtherRoot.UUID && Type == aOtherRoot.Type;
		}

		static FolderRoot CreateFromScene(const Scene& scene) noexcept;
	};

	class Folder
	{
	public:
		Folder(const FolderRoot& aFolderRoot, const String& aPath) noexcept;
		Folder() noexcept = default;

		NO_DISCARD const FolderRoot& GetRoot() const noexcept { return m_Root; }
		NO_DISCARD const String& GetPath() const noexcept { return m_Path; }

		bool operator==(const Folder& aOtherFolder) const noexcept
		{
			return m_Path == aOtherFolder.m_Path && m_Root == aOtherFolder.m_Root;
		}
	private:
		FolderRoot m_Root{};
		String m_Path = "";
	};
}