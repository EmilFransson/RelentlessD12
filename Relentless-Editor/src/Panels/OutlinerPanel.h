#pragma once
#include <Relentless.h>
#include "../UI/OutlinerTable.h"
namespace Relentless
{
	class OutlinerPanel
	{
	public:
		explicit OutlinerPanel() noexcept;
		~OutlinerPanel() noexcept = default;
		void OnEvent(IEvent& event);
		void OnImGuiRender(const bool show) noexcept;
		void SetActiveScene(Scene* pScene) noexcept;
		[[nodiscard]] OutlinerTable& GetTable() noexcept;
	private:
		//Scene* m_pScene;
		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;

		OutlinerTable m_Table;
	};
}