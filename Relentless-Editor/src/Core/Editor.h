#pragma once
#include <Relentless.h>
#include "../Panels/OutlinerPanel.h"
#include "../Panels/ViewportPanel.h"

namespace Relentless
{
	enum class ESceneState : uint8 { Edit = 0, Play, Simulate };

	class EntityFolder;

	class Editor : public ISystemManager
	{
	public:
		Editor() noexcept = default;
		virtual ~Editor() noexcept override;

		static Editor* Get() noexcept
		{
			static Editor editor;
			return &editor;
		}
		
		void CreateSubsystems() noexcept;

		NO_DISCARD const Ref<EntityOutlinerView> GetEntityOutlinerView() const noexcept;

		virtual void OnEvent(IEvent& event) noexcept;
		virtual void OnImGuiRender() noexcept;
		virtual void OnCreate() noexcept;
		virtual void OnDestroy() noexcept;
		virtual void OnUpdate(const float deltaTime) noexcept;
		virtual void OnRender() noexcept;

		NO_DISCARD CallbackID RegisterEventCallback(Callback<bool(IEvent&)> aEventCallback) noexcept;
		NO_DISCARD CallbackID RegisterUpdateCallback(Callback<void(float)> aUpdateCallback) noexcept;
		NO_DISCARD CallbackID RegisterUIRenderCallback(Callback<void()> aUpdateCallback) noexcept;

		void UnregisterEventCallback(CallbackID aCallbackHandle) noexcept;
		void UnregisterUpdateCallback(CallbackID aCallbackHandle) noexcept;
		void UnregisterUIRenderCallback(CallbackID aCallbackHandle) noexcept;

		NO_DISCARD Scene* GetActiveScene() const noexcept;
		NO_DISCARD EntityFolder* GetFolderContainingEntity(entity aEntity) const noexcept;
		NO_DISCARD ViewportRenderView& GetRenderView(uint32 renderViewIndex) noexcept;
		NO_DISCARD std::vector<ViewportRenderView>& GetRenderViews() noexcept;

		NO_DISCARD std::vector<entity> GetTransformSelection() const noexcept;

		void OnViewportEntityDuplicationRequest() noexcept;

		void SetVisibilityForSelectedEntities(bool aVisibilityState) noexcept;

		inline static Broadcaster<void(entity aEntity)> OnEntityTransformed;
		inline static Broadcaster<void()> OnShutDown;
		Broadcaster<void(Scene*)> OnSceneChange;
		Broadcaster<void(Scene*)> OnSceneChanged;

	private:
		void SetActiveScene(const Ref<Scene>& aScene) noexcept;

		void CreateStartScene() noexcept;

		void UI_DrawMainMenuBar() noexcept;

		void LoadBrdfLut_Temp() noexcept;
		void LoadModules() noexcept;

		void OnEntityReadbackDone(uint32 entityID) noexcept;

		NO_DISCARD AssetHandle OnRequestBRDFLut() noexcept;

		void OnViewportHotkeyPressed(ViewportPanel* pPanel, RLS_Key key) noexcept;
		void OnViewportClicked(ViewportPanel* pPanel, Vector2u relativeMouseCoords) noexcept;

		void SpawnViewport() noexcept;
	private:
		std::vector<ViewportRenderView> m_RenderViews;
		std::vector<ViewportPanel*> m_EditorViewports;
		std::unordered_map<CallbackID, Callback<void(float)>> m_UpdateCallbacks;
		std::unordered_map<CallbackID, Callback<void()>> m_UIRenderCallbacks;
		std::unordered_map<CallbackID, Callback<bool(IEvent&)>> m_EventCallbacks;

		entity m_HoveredEntity = NULL_ENTITY;

		Ref<Scene> m_pActiveScene = nullptr;
		Ref<Scene> m_pEditorScene = nullptr;

		std::shared_ptr<UtilityRenderer> m_pUtilityRenderer = nullptr;
		
		bool m_DisplayOutlinerPanel = true;
		bool m_DisplayContentBrowserPanel = true;
		bool m_DisplayPropertiesPanel = true;
		bool m_DisplayInspectorPanel = false;
		bool m_DisplayMetricsPanel = true;
		bool m_DisplaySceneRendererPanel = true;
		bool m_DisplayStatisticsPanel = true;

		bool m_ImmersiveModeEnabled = false;

		AssetHandle m_PlayButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StopButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_PauseButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_SimulateButtonTextureHandle = NULL_HANDLE;
		AssetHandle m_StepButtonTextureHandle = NULL_HANDLE;

		AssetHandle m_BRDFLutTextureHandle = AssetHandle::INVALID;

		std::shared_ptr<TextureCube> m_SkyBox = nullptr;
		
		OutlinerPanel* m_pOutlinerPanel = nullptr;
		float m_MinLogLuminance = -4.0f;
		float m_MinEV100 = -10.0f;
		float m_MaxEV100 = 20.0f;
		float m_ExposureCompensation = 1.0f;

		std::mutex m_OnUpdateMutex;
		std::mutex m_OnUIRenderMutex;
		std::mutex m_OnEventMutex;
	};
}
