#include "AssetThumbnail.h"

#include "Thumbnail/AssetThumbnailData.h"

namespace Relentless
{
	AssetThumbnail::AssetThumbnail(WeakPtr<const AssetThumbnailData> aAssetThumbnailData, const Vector2& aSize) noexcept
		: m_Size{ aSize },
		  m_pAssetThumbnailData{ aAssetThumbnailData }
	{
		SharedPtr<const AssetThumbnailData> pData = aAssetThumbnailData.lock();
		RLS_ASSERT(pData, "[Thumbnail::Thumbnail]: Asset Thumbnail Data is invalid.");

		m_Brush = pData->GetBrush();
		pData->OnThumbnailBrushUpdated.Connect(this, &AssetThumbnail::OnThumbnailBrushUpdated);
	}

	AssetThumbnail::~AssetThumbnail() noexcept
	{
		if (SharedPtr<const AssetThumbnailData> pData = m_pAssetThumbnailData.lock())
			pData->OnThumbnailBrushUpdated.Detach(this);
	}

	Vector2 AssetThumbnail::ReportSize() const noexcept
	{
		return m_Size;
	}

	void AssetThumbnail::OnRender() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::ImageButton("##Thumbnail", (ImTextureID)m_Brush.BackingTexture->GetSRV()->GetGPUHandle().ptr, ImVec2(m_Size.x, m_Size.y));
		ImGui::PopStyleVar();

		const ImVec2 minPoint = ImGui::GetItemRectMin();
		const ImVec2 maxPoint = ImGui::GetItemRectMax();

		const ImU32 lineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(m_Brush.TypeColor.R() , m_Brush.TypeColor.G(), m_Brush.TypeColor.B(), m_Brush.TypeColor.A()));
		pDrawList->AddLine(ImVec2(minPoint.x, maxPoint.y - 2.0f), ImVec2(maxPoint.x, maxPoint.y - 2.0f), lineColor, 2.0f);
	}

	void AssetThumbnail::OnThumbnailBrushUpdated(const ThumbnailBrush& aBrush) noexcept
	{
		m_Brush = aBrush;
	}
}
