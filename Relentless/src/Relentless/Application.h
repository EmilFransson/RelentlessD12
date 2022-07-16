#pragma once
#include "Events\Layer.h"
namespace Relentless
{
	struct ApplicationSpecification
	{
		std::string Name;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification) noexcept;
		virtual ~Application() noexcept = default;
		void Run() noexcept;
		void PushLayer(std::unique_ptr<Layer> pLayer) const noexcept;
		void PushOverlay(std::unique_ptr<Layer> pLayer) const noexcept;
		void OnEvent(IEvent& event) noexcept;
	private:
		ApplicationSpecification m_ApplicationSpecification;
		bool m_IsRunning;
	};

	//To be defined in client (runtime-project):
	[[nodiscard]] const std::unique_ptr<Application> CreateApplication() noexcept;
}