#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer(const std::string& layerName = "EditorLayer") noexcept;
		virtual ~EditorLayer() noexcept override final = default;
		virtual void OnEvent(IEvent& event) noexcept override final;
		virtual void OnImGuiRender() noexcept override;
		virtual void OnAttach() noexcept override final;
		virtual void OnUpdate(const float deltaTime) noexcept override final;
	private:
		
	};
}
