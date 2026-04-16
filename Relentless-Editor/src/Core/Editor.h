#pragma once
#include <Relentless.h>
#include "Panels/OutlinerPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/TestLayoutPanel.h"

namespace Relentless
{
	enum class ESceneState : uint8 { Edit = 0u, Play, Simulate };

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
		virtual void OnUpdate(float deltaTime) noexcept;
		virtual void OnRender() noexcept;

		NO_DISCARD CallbackID RegisterEventCallback(Callback<bool(IEvent&)> aEventCallback) noexcept;
		NO_DISCARD CallbackID RegisterUpdateCallback(Callback<void(float)> aUpdateCallback, float aUpdateRate = 0.0f) noexcept;
		NO_DISCARD CallbackID RegisterUIRenderCallback(Callback<void()> aUpdateCallback) noexcept;

		void UnregisterEventCallback(CallbackID aCallbackHandle) noexcept;
		void UnregisterUpdateCallback(CallbackID aCallbackHandle) noexcept;
		void UnregisterUIRenderCallback(CallbackID aCallbackHandle) noexcept;

		NO_DISCARD Scene* GetActiveScene() const noexcept;
		NO_DISCARD EntityFolder* GetFolderContainingEntity(entity aEntity) const noexcept;

		NO_DISCARD std::vector<entity> GetTransformSelection() const noexcept;

		void OnViewportEntityDuplicationRequest() noexcept;

		inline static Broadcaster<void(entity aEntity)> OnEntityTransformed;
		inline static Broadcaster<void()> OnShutDown;
		Broadcaster<void(Scene*)> OnSceneChange;
		Broadcaster<void(Scene*)> OnSceneChanged;

	private:
		void SetActiveScene(const Ref<Scene>& aScene) noexcept;

		void CreateStartScene() noexcept;

		void UI_DrawMainMenuBar() noexcept;

		void LoadModules() noexcept;

		void UpdateSubsystems(float aDeltaTime) noexcept;
	private:
		struct UpdateCallbackContext
		{
			Callback<void(float)> Callback;
			float UpdateRate = 0.0f;
			float AccumulatedTime = 0.0f;
			bool Alive = false;
		};

		std::vector<ViewportRenderView> m_RenderViews;
		std::vector<ViewportPanel*> m_EditorViewports;

		std::vector<UpdateCallbackContext> m_UpdateCallbacks;
		std::queue<CallbackID> m_UpdateCallbacksFreeList;

		std::unordered_map<CallbackID, Callback<void()>> m_UIRenderCallbacks;
		std::unordered_map<CallbackID, Callback<bool(IEvent&)>> m_EventCallbacks;

		Ref<Scene> m_pActiveScene = nullptr;
		Ref<Scene> m_pEditorScene = nullptr;

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

		OutlinerPanel* m_pOutlinerPanel = nullptr;

		std::mutex m_OnUpdateMutex;
		std::mutex m_OnUIRenderMutex;
		std::mutex m_OnEventMutex;
	};
}
