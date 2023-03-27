#pragma once
#include <Relentless.h>
namespace Relentless
{
	class SceneHierarchyPanel
	{
	public:
		explicit SceneHierarchyPanel() noexcept;
		~SceneHierarchyPanel() noexcept = default;
		void OnImGuiRender() noexcept;
		void SetActiveScene(Scene* const pScene) noexcept;
		void DrawEntityNode(const entity entityID) noexcept;
		void SetSelectedEntity(const entity entityID) noexcept;
		void SetOnEntityDestroyFunction(std::function<void(entity id)> callBackFunction) noexcept;
		[[nodiscard]] constexpr const entity GetSelectedEntity() const noexcept { return m_SelectedEntity; }
	private:
		Scene* m_pScene;
		entity m_SelectedEntity;
		std::function<void(entity)> m_OnEntityDestroyedCallBack;
		entity m_EntityScheduledForDestruction;
	};
}