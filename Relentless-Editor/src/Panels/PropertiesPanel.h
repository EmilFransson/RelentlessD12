#pragma once
#include <Relentless.h>
namespace Relentless
{
	class PropertiesPanel
	{
	public:
		explicit PropertiesPanel() noexcept;
		~PropertiesPanel() noexcept = default;
		void OnImGuiRender() noexcept;
		void SetSelectedEntity(const entity entityID) noexcept;
		void SetEntityManager(EntityManager* const entityManager) noexcept;
	private:
		void DrawAllComponentNodes();
	private:
		bool m_FormattingName = false;
		entity m_SelectedEntity;
		EntityManager* m_pEntityManager;
	};
}