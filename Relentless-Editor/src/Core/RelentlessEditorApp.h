#pragma once
#include "EditorLayer.h"

namespace Relentless
{
	class RelentlessEditor : public Application
	{
	public:
		RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~RelentlessEditor() noexcept override final = default;

		virtual void Initialize() noexcept override;
		virtual void Update() noexcept override;
	private:
		UniquePtr<EditorLayer> m_pEditorLayer = nullptr;
		UniquePtr<Renderer> m_pRenderer = nullptr;
	};
}