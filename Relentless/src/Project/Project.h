#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	struct ProjectConfig
	{
		String Name				= "UntitledRelentlessProject";
		Path AssetPath			= "Assets";
		Path ThumbnailCachePath = "Cache/Thumbnails/";
	};

	class RLS_API Project : public RefCounted<Project>
	{
	public:
		NO_DISCARD static Path GetAssetDirectory() noexcept;
		NO_DISCARD static Path GetThumbnailCacheDirectory() noexcept;
		NO_DISCARD ProjectConfig& GetConfig() noexcept;
		NO_DISCARD static const String& GetName() noexcept;
		NO_DISCARD static const Path& GetProjectDirectory() noexcept;

		static Ref<Project> Load(const Path& aPath) noexcept;
		static bool SaveActive(const Path& aPath) noexcept;
		static Ref<Project> New(const ProjectConfig& aConfig) noexcept;
	private:
		ProjectConfig m_Config;
		Path m_ActiveProjectDirectory;
		
		inline static Ref<Project> s_ActiveProject = nullptr;
	};
	
}