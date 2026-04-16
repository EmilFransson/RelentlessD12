#include "ImGuiLayer.h"
#include "ImGuiFonts.h"

namespace Relentless
{
	ImGuiLayer::ImGuiLayer(GraphicsDevice* pDevice) noexcept
		:Layer("ImGuiLayer"), m_pDevice{ pDevice }
	{
	}

	void ImGuiLayer::BeginFrame(Ref<Texture> pTarget, CommandContext* pCommandContext) noexcept
	{
		PROFILE_FUNC;

		ImGuiIO& io = ImGui::GetIO();
		
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
		io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();
	}

	void ImGuiLayer::EndFrame(CommandContext* pCommandContext) noexcept
	{
		PROFILE_FUNC;

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandContext->GetCommandList());

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void ImGuiLayer::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		static bool showWindow = true;
		ImGui::ShowDemoWindow(&showWindow);
	}

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		style.AntiAliasedLines = true;
		style.AntiAliasedFill = true;
		style.WindowBorderHoverPadding = 1.0f;

		ImGui::StyleColorsDark();

		//Configure fonts:
		{
			UI::FontConfiguration robotoDefault;
			robotoDefault.FontName = "Default";
			robotoDefault.FilePath = "Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoDefault.Size = 15.0f;
			UI::Fonts::Add(robotoDefault, true);

			static const ImWchar s_FontAwesomeRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
			UI::FontConfiguration fontAwesome;
			fontAwesome.FontName = "FontAwesome";
			fontAwesome.FilePath = "Fonts/FontAwesome/fa-solid-900.ttf";
			fontAwesome.Size = 16.0f;
			fontAwesome.GlyphRanges = s_FontAwesomeRanges;
			fontAwesome.MergeWithLast = true;
			UI::Fonts::Add(fontAwesome);
		}

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
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 0.4f);

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

		RLS_VERIFY(ImGui_ImplWin32_Init(Application::Get().GetWindow()->GetNativeWindow()), "[ImGuiLayer::OnAttach]: Failed to initialize ImGui win32 backend.");
		ImGui_ImplWin32_EnableDpiAwareness();

		m_Allocator.Initialize(m_pDevice);

		ImGui_ImplDX12_InitInfo info = {};
		info.Device = m_pDevice->GetDevice();
		info.CommandQueue = m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue();
		info.NumFramesInFlight = GraphicsDevice::NUM_BUFFERS;
		info.RTVFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
		info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		info.UserData = &m_Allocator;
		info.SrvDescriptorHeap = m_pDevice->GetGlobalShaderBindableHeap()->GetDescriptorHeapInterface();

		info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* init, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
			{
				static_cast<ImGuiSRVAllocator*>(init->UserData)->Allocate(outCpu, outGpu);
			};
		info.SrvDescriptorFreeFn =
			[](ImGui_ImplDX12_InitInfo* aInitInfo, D3D12_CPU_DESCRIPTOR_HANDLE aCPU, D3D12_GPU_DESCRIPTOR_HANDLE aGPU)
			{
				static_cast<ImGuiSRVAllocator*>(aInitInfo->UserData)->Free(aCPU, aGPU);
			};

		RLS_VERIFY(ImGui_ImplDX12_Init(&info));
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

}