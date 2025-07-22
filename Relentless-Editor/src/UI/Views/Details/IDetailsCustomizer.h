#pragma once

#include <Relentless.h>

namespace Relentless
{
	class Editor;
	class EntityManager;

	class IDetailsCustomizer : public RefCounted<IDetailsCustomizer>
	{
	public:
		IDetailsCustomizer(Editor* pEditor) noexcept;
		virtual ~IDetailsCustomizer() noexcept = default;
		virtual [[nodiscard]] std::vector<Ref<IDetailsTreeNode>> Build() noexcept = 0;
	protected:
		Editor* m_pEditor = nullptr;
	};
}