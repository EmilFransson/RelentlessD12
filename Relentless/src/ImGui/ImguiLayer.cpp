#include "ImguiLayer.h"
#include "Core/Application.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/ResourceViews.h"
#include "Graphics/RHI/Window.h"

namespace Relentless
{
	ImguiLayer::ImguiLayer(GraphicsDevice* pDevice) noexcept
		:Layer("ImGuiLayer"), m_pDevice{pDevice}
	{}

	void ImguiLayer::BeginFrame(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		//DXCall_STD(pCommandList->SetDescriptorHeaps(1, Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetDescriptorHeapInterface().GetAddressOf()));

		//BackBuffer& backBuffer{ Window::GetBackBuffers()[Application::Get().GetGPUTaskManager().GetCurrentFrameIndex()]};
		D3D12_RESOURCE_BARRIER resourceTransitionBarrier{};
		resourceTransitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		resourceTransitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		//resourceTransitionBarrier.Transition.pResource = backBuffer.pBackBuffer.Get();
		resourceTransitionBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		resourceTransitionBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceTransitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		//DXCall_STD(pCommandList->ResourceBarrier(1u, &resourceTransitionBarrier));

		//backBuffer.CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;

		//DXCall_STD(pCommandList->OMSetRenderTargets(1u, &backBuffer.Handle.CPUHandle, false, nullptr));
		//D3D12_VIEWPORT nviewport = { 0.0f, 0.0f, static_cast<float>(Window::GetWidth()), static_cast<float>(Window::GetHeight()), 0.0f, 1.0f };
		//D3D12_RECT nscissorRect = { 0, 0, static_cast<LONG>(Window::GetWidth()), static_cast<LONG>(Window::GetHeight()) };
		//DXCall_STD(pCommandList->RSSetViewports(1, &nviewport));
		//DXCall_STD(pCommandList->RSSetScissorRects(1, &nscissorRect));

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}


	void ImguiLayer::BeginFrameEx(Ref<TextureEx> pTarget, CommandContext* pCommandContext) noexcept
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		pCommandContext->SetViewport(FloatRect(0, 0, (float)pTarget->GetWidth(), (float)pTarget->GetHeight()), 0, 1);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = pTarget->GetRTV()->GetCPUHandle();
		pCommandContext->GetCommandList()->OMSetRenderTargets(1u, &handle, false, nullptr);

		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void ImguiLayer::EndFrameEx(Ref<TextureEx> pTarget, CommandContext* pCommandContext) noexcept
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandContext->GetCommandList());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		
	}

	void ImguiLayer::EndFrame(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		ImGui::Render();

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList.Get());
		
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		//Application::Get().GetGPUTaskManager().ScheduleCommandList(pCommandList);
	}

	void ImguiLayer::OnImGuiRender() noexcept
	{
		static bool showWindow = true;
		ImGui::ShowDemoWindow(&showWindow);
	}

	void ImguiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();
		
		std::string openSansFontPath = std::string(ENGINE_ASSET_DIRECTORY) + std::string("/Fonts/opensans/");
		RLS_VERIFY(io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Bold.ttf")).c_str(), 22.0f), "Failed to load ImGui Font");
		RLS_VERIFY(io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Bold.ttf")).c_str(), 26.0f), "Failed to load ImGui Font");
		io.FontDefault = io.Fonts->AddFontFromFileTTF((std::string(openSansFontPath) + std::string("OpenSans-Regular.ttf")).c_str(), 22.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 37.0f / 255.0f, 35.0f / 255.0f, 35.0f / 255.0f, 1.0f };
		
		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_WindowBg];

		// Table
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(47.0f / 255.0f, 47.0f / 255.0f, 47.0f / 255.0f, 1.0f);

		// Separator
		colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.115f, 0.0955f, 0.081f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_TitleBg];
		colors[ImGuiCol_TitleBgCollapsed] = colors[ImGuiCol_TitleBg];

		colors[ImGuiCol_ScrollbarBg] = colors[ImGuiCol_WindowBg];

		// Drag Drop
		colors[ImGuiCol_DragDropTarget] = ImVec4();

		style.ScrollbarSize = style.ScrollbarSize - 3.0f;

		constexpr float scaleFactor = 1.0f;  // Adjust this scale factor as needed
		style.ScaleAllSizes(scaleFactor);
		
		ImGui_ImplWin32_Init(::GetActiveWindow());

		//TODO; CHECK THIS OUT!
		DescriptorHeapEx* pDescriptorHeap = m_pDevice->GetGlobalShaderBindableHeap();
		m_DescriptorHandle = m_pDevice->RegisterGlobalDescriptor(DescriptorHandleTypeEx::SRV);

		ImGui_ImplDX12_Init
		(
			m_pDevice->GetDevice(),
			GraphicsDevice::NUM_BUFFERS,
			DXGI_FORMAT_R10G10B10A2_UNORM,
			pDescriptorHeap->GetDescriptorHeapInterface(),
			m_DescriptorHandle.CPUHandle,
			m_DescriptorHandle.GPUHandle
		);
	}

	void ImguiLayer::OnDetach()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

}