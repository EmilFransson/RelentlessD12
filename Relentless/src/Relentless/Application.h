#pragma once
#include "Events\Layer.h"
#include "ImGui\ImguiLayer.h"
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
		uint8_t m_CurrentFrameIndex;
		std::unique_ptr<uint64_t[]> m_pFenceValues;
		Microsoft::WRL::ComPtr<ID3D12Fence1> m_pFence;
		HANDLE m_FenceEvent;
	};

	//To be defined in client (runtime-project):
	[[nodiscard]] const std::unique_ptr<Application> CreateApplication() noexcept;
}