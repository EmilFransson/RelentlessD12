#pragma once
#include <Relentless.h>

namespace Relentless
{
	class AssetThumbnailPool;

	class ContentBrowserModule : public IModule
	{
	public:
		ContentBrowserModule() noexcept;
		virtual ~ContentBrowserModule() override;

		NO_DISCARD const SharedPtr<AssetThumbnailPool>& GetAssetThumbnailPool() const noexcept;
	protected:
		void OnLoad() override;
		void OnUnload() override;
		void OnUpdate(float aDeltaTime);
	private:
		SharedPtr<AssetThumbnailPool> m_pThumbnailPool;
		CallbackID m_OnUpdateCallbackHandle = std::numeric_limits<CallbackID>::max();
	};
}