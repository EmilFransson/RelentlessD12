#pragma once
#include "IEventListener.h"

namespace Relentless
{
	class Layer : public IEventListener
	{
	public:
		Layer(const std::string& debugName = "Layer") noexcept;
		virtual ~Layer() noexcept = default;
		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(const float /*deltaTime*/) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(IEvent& event) noexcept override = 0;
	private:
		std::string m_DebugName;
	};
}