#include "ImguiLayer.h"
#include "../Graphics/D3D12Core.h"
namespace Relentless
{
	ImguiLayer::ImguiLayer() noexcept
		:Layer("ImGuiLayer")
	{
	}

	ImguiLayer::~ImguiLayer() noexcept
	{
		//ImGui_ImplDX12_Shutdown();
		//ImGui_ImplWin32_Shutdown();
		//ImGui::DestroyContext();
	}

	void ImguiLayer::BeginFrame() noexcept
	{
	}

	void ImguiLayer::EndFrame() noexcept
	{
	}

	void ImguiLayer::OnImGuiRender() noexcept
	{
	}

	void ImguiLayer::OnAttach()
	{
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO();
		//io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
		//ImGui::StyleColorsDark();
		//
		//ImGuiStyle& style = ImGui::GetStyle();
		//if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		//{
		//	style.WindowRounding = 0.0f;
		//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		//}
		//
		//auto& colors = ImGui::GetStyle().Colors;
		//colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
		//
		//// Headers
		//colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		//colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		//colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//
		//// Buttons
		//colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		//colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		//colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//
		//// Frame BG
		//colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		//colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		//colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//
		//// Tabs
		//colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		//colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		//colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		//
		//// Title
		//colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		//
		//m_pDescriptorHeap = std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1u, true);
		//ImGui_ImplWin32_Init(::GetActiveWindow());
		//ImGui_ImplDX12_Init
		//(
		//	D3D12Core::GetDevice().Get(),
		//	D3D12Core::GetNrOfBufferedFrames(),
		//	DXGI_FORMAT_R8G8B8A8_UNORM,
		//	m_pDescriptorHeap->GetDescriptorHeapInterface().Get(),
		//	m_pDescriptorHeap->GetCPUStartHandle(),
		//	m_pDescriptorHeap->GetGPUStartHandle()
		//);
	}
}