#include "ContentBrowserModule.h"
#include <Core/Editor.h>

#include "UIModule.h"

#include <Panels/ContentBrowserPanel.h>

#include <Thumbnail/AssetThumbnailPool.h>

namespace Relentless
{
	ContentBrowserModule::ContentBrowserModule() noexcept {}

	ContentBrowserModule::~ContentBrowserModule(){}

	const UniquePtr<AssetThumbnailPool>& ContentBrowserModule::GetAssetThumbnailPool() const noexcept
	{
		return m_pThumbnailPool;
	}

	void ContentBrowserModule::OnLoad()
	{
		m_OnUpdateCallbackHandle = Editor::Get()->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &ContentBrowserModule::OnUpdate));

		m_pThumbnailPool = MakeUnique<AssetThumbnailPool>();
		ModuleManager::LoadModuleChecked<UIModule>().AddPanel<ContentBrowserPanel>();
	}

	void ContentBrowserModule::OnUnload()
	{
		Editor::Get()->UnregisterUpdateCallback(m_OnUpdateCallbackHandle);
	}

	void ContentBrowserModule::OnUpdate(float)
	{
		m_pThumbnailPool->OnUpdate();
	}

}
