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

		NO_DISCARD const UniquePtr<Editor>& GetEditor() const noexcept;
		NO_DISCARD const UniquePtr<Renderer>& GetRenderer() const noexcept;
	private:
		UniquePtr<EditorLayer> m_pEditorLayer = nullptr;
		UniquePtr<Renderer> m_pRenderer = nullptr;

		Ref<Texture> m_pColorTarget = nullptr;
	};
}