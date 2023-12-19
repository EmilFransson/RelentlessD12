#pragma once
#include <Relentless.h>
namespace Relentless
{
	class PropertiesPanel
	{
	public:
		explicit PropertiesPanel() noexcept;
		~PropertiesPanel() noexcept = default;
		void OnImGuiRender(const bool show) noexcept;
		void SetSelectedEntity(const entity entityID) noexcept;
		void SetActiveScene(Scene* const pScene) noexcept;
		void SetOnMaterialSelectedCallback(std::function<void(const AssetHandle& materialHandle)> callback) noexcept;
	private:
		void DrawAllComponentNodes();
		template<typename Component, typename ContextFunction>
		void DrawComponentNode(const char* nodeName, const ContextFunction&& func) noexcept;
		bool DrawVec3Control(const char* label, DirectX::XMFLOAT3& values, float dragSpeed = 0.1f, float resetValue = 0.0f, float minValue = std::numeric_limits<float>::lowest(), float maxValue = std::numeric_limits<float>::max(), float columnWidth = 100.0f) noexcept;
		bool DrawVec2Control(const char* label, DirectX::XMFLOAT2& values, float dragSpeed = 0.1f, float resetValue = 0.0f, float minValue = std::numeric_limits<float>::lowest(), float maxValue = std::numeric_limits<float>::max(), float columnWidth = 100.0f) noexcept;
	private:
		bool m_FormattingName = false;
		entity m_SelectedEntity;
		Scene* m_pScene;

		std::function<void(const AssetHandle& materialHandle)> m_OnMaterialSelectedCallback;
	};
}