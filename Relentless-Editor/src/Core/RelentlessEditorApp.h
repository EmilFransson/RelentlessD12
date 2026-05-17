#pragma once
#include "EditorLayer.h"

namespace Relentless
{
	class ImGuiLayer;

	class RelentlessEditor : public Application
	{
	public:
		RelentlessEditor(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~RelentlessEditor() noexcept override;

		virtual void Initialize() noexcept override;
		virtual void Update() noexcept override;
		virtual void UIRenderBegin(CommandContext* aCommandContext) noexcept override;
		virtual void UIRenderEnd(CommandContext* aCommandContext) noexcept override;

		NO_DISCARD const UniquePtr<Renderer>& GetRenderer() const noexcept;
	protected:
		void ShutDown() noexcept override;
	private:
		UniquePtr<ImGuiLayer> m_pImGuiLayer;
		UniquePtr<EditorLayer> m_pEditorLayer = nullptr;
		UniquePtr<Renderer> m_pRenderer = nullptr;

		Ref<Texture> m_pColorTarget = nullptr;
	};
}