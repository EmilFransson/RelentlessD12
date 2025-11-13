#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Editor;

	class IEditorPanel : public PanelBase
	{
	public:
		IEditorPanel(const char* aName, ImGuiWindowFlags someFlags, Editor* aEditor) noexcept;
		virtual ~IEditorPanel() noexcept override;
	protected:
		Editor* m_pEditor = nullptr;
	};
}