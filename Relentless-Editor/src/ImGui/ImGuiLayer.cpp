#include "ImGuiLayer.h"
#include "ImGuiFonts.h"

#include "Core/Editor.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

#include "Panels/EditorViewportPanel.h"

#include "Subsystem/EditorViewportSubsystem.h"
#include "Subsystem/EngineContentSubsystem.h"
#include "Subsystem/EntityComponentDefinitionRegistry.h"
#include "Subsystem/SelectionSubsystem.h"

#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/ContextMenu.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/Spacer.h"

namespace Relentless
{
	static Button* FileButton = nullptr;
	static Button* SpawnButton = nullptr;
	static Button* MinimizeButton = nullptr;
	static Button* MaximizeButton = nullptr;
	static Button* ExitButton = nullptr;

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

		constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pCommandContext->GetCommandList()->ClearRenderTargetView(handle, clearColor, 0, nullptr);
		pCommandContext->GetCommandList()->OMSetRenderTargets(1u, &handle, false, nullptr);

		static bool dockspaceOpen = true;

		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		const float topInset = 70.0f;
		const float bottomInset = 30.0f;

		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + topInset));
			ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - topInset - bottomInset));
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

		// --- TEMP: top chrome band ---
		{
			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(vp->WorkPos);
			ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, topInset));
			ImGui::SetNextWindowViewport(vp->ID);

			constexpr ImGuiWindowFlags barFlags =
				ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (ImGui::Begin("##TopChromeBand", nullptr, barFlags))
			{
				m_pTopChromeBox->AssignSize(Vector2(vp->WorkSize.x, topInset));
				m_pTopChromeBox->Render();
			}
			ImGui::End();

			ImGui::PopStyleVar(3);
		}

		// --- TEMP: bottom chrome band ---
		{
			const ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - bottomInset));
			ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, bottomInset));
			ImGui::SetNextWindowViewport(vp->ID);

			constexpr ImGuiWindowFlags barFlags =
				ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

			if (ImGui::Begin("##BottomChromeBand", nullptr, barFlags))
			{
				m_pBottomChromeBox->AssignSize(Vector2(vp->WorkSize.x, bottomInset));
				m_pBottomChromeBox->Render();
			}
			ImGui::End();

			ImGui::PopStyleVar(3);
		}

		m_pFPSLabel->SetText(std::format("FPS: {}", Time::GetFramesPerSecond()));
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

	bool ImGuiLayer::IsAnyMainMenuButtonHovered() noexcept
	{
		return FileButton->IsHovered() || SpawnButton->IsHovered() || MinimizeButton->IsHovered() || MaximizeButton->IsHovered() || ExitButton->IsHovered();
	}

	void ImGuiLayer::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		//static bool showWindow = true;
		//ImGui::ShowDemoWindow(&showWindow);
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

		auto AddFontAwesomeMerge = [](const char* uniqueName, float aSize)
			{
				static const ImWchar s_FontAwesomeRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
				UI::FontConfiguration fa;
				fa.FontName = uniqueName;
				fa.FilePath = "Fonts/FontAwesome/fa-solid-900.ttf";
				fa.Size = aSize;
				fa.GlyphRanges = s_FontAwesomeRanges;
				fa.MergeWithLast = true;
				UI::Fonts::Add(fa);
			};

		//Configure fonts:
		{
			UI::FontConfiguration robotoBold;
			robotoBold.FontName = "Bold";
			robotoBold.FilePath = "Fonts/Roboto/Roboto-Bold.ttf";
			robotoBold.Size = 18.0f;
			UI::Fonts::Add(robotoBold);

			AddFontAwesomeMerge("FA_BOLD", 19.0f);

			UI::FontConfiguration robotoLarge;
			robotoLarge.FontName = "Large";
			robotoLarge.FilePath = "Fonts/Roboto/Roboto-Regular.ttf";
			robotoLarge.Size = 24.0f;
			UI::Fonts::Add(robotoLarge);

			AddFontAwesomeMerge("FA_LARGE", 25.0f);

			UI::FontConfiguration robotoDefault;
			robotoDefault.FontName = "Default";
			robotoDefault.FilePath = "Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoDefault.Size = 15.0f;
			UI::Fonts::Add(robotoDefault, true);

			AddFontAwesomeMerge("FA_DEFAULT", 16.0f);

			UI::FontConfiguration robotoMedium;
			robotoMedium.FontName = "Medium";
			robotoMedium.FilePath = "Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoMedium.Size = 18.0f;
			UI::Fonts::Add(robotoMedium);

			AddFontAwesomeMerge("FA_MEDIUM", 19.0f);

			UI::FontConfiguration robotoSmall;
			robotoSmall.FontName = "Small";
			robotoSmall.FilePath = "Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoSmall.Size = 12.0f;
			UI::Fonts::Add(robotoSmall);

			AddFontAwesomeMerge("FA_SMALL", 13.0f);
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
		//ImGui_ImplWin32_EnableDpiAwareness();

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

		m_pTopChromeBox = RLS_NEW VerticalBox();
		m_pTopChromeBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pTopChromeBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		HorizontalBox* pMainMenuBox = m_pTopChromeBox->AddWidget(RLS_NEW HorizontalBox());
		pMainMenuBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pMainMenuBox->SetVerticalSizePolicy(ESizePolicy::Fixed);
		pMainMenuBox->SetSize(Vector2(-1.0f, 30.0f));
		pMainMenuBox->SetBackgroundColor(Color(0.1f, 0.1f, 0.1f, 1.0f));

		FileButton = pMainMenuBox->AddWidget(Button::CreateTransparent("File"));
		FileButton->OnClicked(this, &ImGuiLayer::OnFileButtonClicked);
		FileButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		FileButton->SetMargin(FloatRect::WithLeft(5.0f));

		SpawnButton = pMainMenuBox->AddWidget(Button::CreateTransparent("Spawn"));
		SpawnButton->OnClicked(this, &ImGuiLayer::OnSpawnButtonClicked);
		SpawnButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		pMainMenuBox->AddWidget(RLS_NEW Spacer())
			->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		m_pProjectNameLabel = pMainMenuBox->AddWidget(RLS_NEW Label("Project Name"));
		m_pProjectNameLabel->SetPadding(Vector2(100.0f, 0.0f));
		m_pProjectNameLabel->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		HorizontalBox* pRightBox = pMainMenuBox->AddWidget(RLS_NEW HorizontalBox());
		pRightBox->SetSpacing(20.0f);

		MinimizeButton = pRightBox->AddWidget(Button::CreateTransparent(ICON_FA_WINDOW_MINIMIZE));
		MinimizeButton->OnClicked([] { Application::Get().GetWindow()->Minimize(); });
		MinimizeButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		MaximizeButton = pRightBox->AddWidget(Button::CreateTransparent(ICON_FA_WINDOW_MAXIMIZE));
		MaximizeButton->OnClicked([] { Application::Get().SubmitToMainThread([]() { Application::Get().GetWindow()->ToggleMaximize();  }); });
		MaximizeButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		ExitButton = pRightBox->AddWidget(Button::CreateTransparent(ICON_FA_XMARK));
		ExitButton->OnClicked([] { Application::Get().InitializeShutdownProcedure(); });
		ExitButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		ExitButton->SetMargin(FloatRect::WithRight(5.0f));

		m_pBottomChromeBox = RLS_NEW VerticalBox();
		m_pBottomChromeBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pBottomChromeBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		HorizontalBox* pBottomChromeBox = m_pBottomChromeBox->AddWidget(RLS_NEW HorizontalBox());
		pBottomChromeBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBottomChromeBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pBottomChromeBox->SetBackgroundColor(Color(0.1f, 0.1f, 0.1f, 1.0f));
		pBottomChromeBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Right);

		m_pFPSLabel = pBottomChromeBox->AddWidget(RLS_NEW Label("FPS: "));
		m_pFPSLabel->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pFPSLabel->SetMargin(FloatRect::WithRight(5.0f));

		HorizontalBox* pMainChromeBand = m_pTopChromeBox->AddWidget(RLS_NEW HorizontalBox());
		pMainChromeBand->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pMainChromeBand->SetVerticalSizePolicy(ESizePolicy::Stretch);

		pMainChromeBand->AddWidget(RLS_NEW Button(std::format("{}  {}", ICON_FA_CUBE, ICON_FA_CHEVRON_DOWN)))
			->OnClicked(this, &ImGuiLayer::OnAddEntityButtonClicked)
			->SetTooltipText("Quickly add to the project.")
			->SetMargin(FloatRect::WithLeft(5.0f))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center)
			->SetHorizontalSizePolicy(ESizePolicy::Fixed)
			->SetVerticalSizePolicy(ESizePolicy::Fixed)
			->SetSize(Vector2(60.0f, 30.0f));

		Project::OnProjectChanged.Connect(this, &ImGuiLayer::OnProjectChanged);
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		Project::OnProjectChanged.Detach(this);
	}

	void ImGuiLayer::OnAddEntityButtonClicked()
	{
		Editor* pEditor = Editor::Get();

		EntityComponentDefinitionRegistry* pEntityComponentDefinitionRegistry = pEditor->GetSubsystem<EntityComponentDefinitionRegistry>();

		auto CreateEntityItem = [pEntityComponentDefinitionRegistry]<typename ComponentType>(ContextMenuBuilder& aBuilder, Callback<void()>&& OnSelectedCallback)
		{
			Ref<IEntityComponentDefinition> pComponentDefinition = pEntityComponentDefinitionRegistry->GetDefinition<ComponentType>();
			aBuilder.AddItem(std::format("{} {}", pComponentDefinition->GetIcon(), pComponentDefinition->GetDisplayName()), [callback = std::forward<Callback<void()>>(OnSelectedCallback)]()
				{
					callback();
					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				})
				.Tooltip(pComponentDefinition->GetDisplayName());
		};

		auto TranslateAndSelectEntity = [](entity aEntity)
		{
				Editor* pEditor = Editor::Get();
				EditorViewportSubsystem* pEditorViewportSubsystem = pEditor->GetSubsystem<EditorViewportSubsystem>();
				SelectionSubsystem* pSelectionSubsystem = pEditor->GetSubsystem<SelectionSubsystem>();
				pSelectionSubsystem->DeselectAllEntities();

				Scene* pActiveScene = pEditor->GetActiveScene();
				EntityManager& entityManager = pActiveScene->GetEntityManager();

				const std::vector<ViewportPanel*>& viewportPanels = pEditorViewportSubsystem->GetViewportPanels();
				const ViewportPanel* pViewportPanel = viewportPanels.back();
				const ViewportClient& client = pViewportPanel->GetClient();
				const Vector3& cameraLocation = client.GetCamera().GetLocation();
				const Vector3 cameraForward = client.GetCamera().GetForwardVector();

				entityManager.Get<TransformComponent>(aEntity).SetWorldLocation(cameraLocation + (cameraForward * 5.0f));
				pSelectionSubsystem->SelectEntity(aEntity);
		};

		ContextMenuBuilder builder;
		builder.AddSubmenu(ICON_FA_LIGHTBULB "  Lights", [CreateEntityItem, TranslateAndSelectEntity](ContextMenuBuilder& aBuilder)
			{
				CreateEntityItem.operator()<DirectionalLightComponent>(aBuilder, [TranslateAndSelectEntity]()
					{
						TranslateAndSelectEntity(Editor::Get()->GetActiveScene()->CreateLight("DirectionalLight", ELightType::Directional));
					});
				CreateEntityItem.operator()<PointLightComponent>(aBuilder, [TranslateAndSelectEntity]()
					{
						TranslateAndSelectEntity(Editor::Get()->GetActiveScene()->CreateLight("PointLight", ELightType::Point));
					});
				CreateEntityItem.operator()<SpotLightComponent> (aBuilder, [TranslateAndSelectEntity]()
					{
						TranslateAndSelectEntity(Editor::Get()->GetActiveScene()->CreateLight("SpotLight", ELightType::Spot));
					});
				//CreateEntityItem.operator()<SkyLightComponent>(aBuilder, [TranslateAndSelectEntity]() 
				//	{
				//		TranslateAndSelectEntity(Editor::Get()->GetActiveScene()->CreateLight("SkyLight", ELightType::Sky));
				//	});
			});

		builder.AddSubmenu(ICON_FA_SHAPES "   Shapes", [TranslateAndSelectEntity](ContextMenuBuilder& aBuilder)
			{
				aBuilder.AddItem("Cube", [TranslateAndSelectEntity]()
					{
						Editor* pEditor = Editor::Get();
						Scene* pActiveScene = pEditor->GetActiveScene();
						EngineContentSubsystem* pContentSubsystem = pEditor->GetSubsystem<EngineContentSubsystem>();
						EntityManager& entityManager = pActiveScene->GetEntityManager();
						
						const entity cubeEntity = pActiveScene->CreateEntity("Cube");
						entityManager.Add<MeshFilterComponent>(cubeEntity).SetMesh(pContentSubsystem->GetCubeMeshHandle());
						entityManager.Add<MeshRendererComponent>(cubeEntity).SetMaterial(pContentSubsystem->GetWhiteMaterialHandle());
						
						TranslateAndSelectEntity(cubeEntity);
						ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
					})
					.Tooltip("Cube");

				aBuilder.AddItem("Sphere", [TranslateAndSelectEntity]()
					{
						Editor* pEditor = Editor::Get();
						Scene* pActiveScene = pEditor->GetActiveScene();
						EngineContentSubsystem* pContentSubsystem = pEditor->GetSubsystem<EngineContentSubsystem>();
						EntityManager& entityManager = pActiveScene->GetEntityManager();

						const entity sphereEntity = pActiveScene->CreateEntity("Sphere");
						entityManager.Add<MeshFilterComponent>(sphereEntity).SetMesh(pContentSubsystem->GetSphereMeshHandle());
						entityManager.Add<MeshRendererComponent>(sphereEntity).SetMaterial(pContentSubsystem->GetWhiteMaterialHandle());

						TranslateAndSelectEntity(sphereEntity);
						ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
					})
					.Tooltip("Sphere");
			});

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void ImGuiLayer::OnFileButtonClicked()
	{
		ContextMenuBuilder builder;
		builder.AddItem("Exit", [](){ Application::Get().InitializeShutdownProcedure(); });

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void ImGuiLayer::OnProjectChanged()
	{
		m_pProjectNameLabel->SetText(Project::GetName());
	}

	void ImGuiLayer::OnSpawnButtonClicked()
	{
		static int counter = 0;

		ContextMenuBuilder builder;
		builder.AddItem("Empty Entity", []() {  Editor::Get()->GetActiveScene()->CreateEntity(std::format("Entity_{}", counter++).c_str()); });
		builder.AddItem("1000 Entities", []() 
			{  
				Scene* pScene = Editor::Get()->GetActiveScene();

				for (auto _ : std::views::repeat(std::monostate{}, 1'000))
					pScene->CreateEntity(std::format("Entity_{}", counter++).c_str()); 
			});
		builder.AddItem("Editor Viewport", [](){ ModuleManager::LoadModuleChecked<UIModule>().OpenPanel<EditorViewportPanel>(); });

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

}