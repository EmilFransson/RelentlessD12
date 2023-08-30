#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class InspectedAssetType : uint8_t { NONE = 0, MATERIAL };

	class InspectorPanel
	{
	public:
		explicit InspectorPanel() noexcept;
		~InspectorPanel() noexcept = default;
		void OnImGuiRender(const bool show) noexcept;
		void SetContext(const AssetHandle& assetHandle, const InspectedAssetType assetType) noexcept;
	private:
		void RenderMaterialInspector() noexcept;
		bool DrawVec2Control(const char* label, DirectX::XMFLOAT2& values, float dragSpeed = 0.1f, float resetValue = 0.0f, float minValue = std::numeric_limits<float>::lowest(), float maxValue = std::numeric_limits<float>::max(), float columnWidth = 100.0f) noexcept;
	private:
		AssetHandle m_InspectedAssetHandle;
		InspectedAssetType m_InspectedAssetType;
		std::shared_ptr<Texture2D> m_pColorPickerWidgetTexture;
		bool m_ForceDisplay;
		float m_MaterialMapThumbnailSize;
	};
}