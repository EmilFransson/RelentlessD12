#pragma once

namespace Relentless
{
	struct ProjectConfig
	{
		String Name		= "Untitled";
		Path AssetPath	= "Assets";
	};

	class Project : public RefCounted<Project>
	{
	public:
		NO_DISCARD static Path GetAssetDirectory() noexcept;
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