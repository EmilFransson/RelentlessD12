#pragma once
#include <Relentless.h>
#include "Editor.h"

namespace Relentless
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName = "EditorLayer") noexcept;
		virtual ~EditorLayer() noexcept override final = default;
		virtual void OnEvent(IEvent& event) noexcept override final;
		virtual void OnImGuiRender() noexcept override final;
		virtual void OnAttach() noexcept override final;
		virtual void OnDetach() noexcept override final;
		virtual void OnUpdate(const float deltaTime) noexcept override final;
		virtual void OnRender() noexcept override final;
		virtual void OnPostRender() noexcept override final;
	private:
		Editor m_pEditor;
	};
}
