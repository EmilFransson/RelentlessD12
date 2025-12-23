#include "Thumbnail.h"

namespace Relentless
{
	Thumbnail::Thumbnail(const Vector2& aSize) noexcept
		:m_Size{ aSize }
	{

	}

	float Thumbnail::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	Vector2 Thumbnail::ReportSize() const noexcept
	{
		return m_Size;
	}

	Thumbnail* Thumbnail::SetBackgroundColor(const Color& aColor) noexcept
	{
		m_BackgroundColor = aColor;
		return this;
	}

	void Thumbnail::OnRender() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		ImGui::BeginGroup();

		const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(m_Size.x, m_Size.y));
		ImGui::SetItemAllowOverlap();

		ImGui::SetCursorScreenPos(cursorPos);

		const ImVec2 thumbnailMinPoint(cursorPos);
		const ImVec2 thumbnailMaxPoint(thumbnailMinPoint.x + m_Size.x, thumbnailMinPoint.y + m_Size.y);

		constexpr const ImU32 black = IM_COL32(0, 0, 0, 128);
		pDrawList->AddRectFilled(thumbnailMinPoint, thumbnailMaxPoint, black, 7.0f);

		const ImVec2 thumbnailClientMinPoint = ImVec2(thumbnailMinPoint.x + 2.0f, thumbnailMinPoint.y + 2.0f);
		const ImVec2 thumbnailClientMaxPoint = ImVec2(thumbnailMaxPoint.x - 2.0f, thumbnailMaxPoint.y - 2.0f);

		const ImU32 bgColor = IM_COL32(m_BackgroundColor.x, m_BackgroundColor.y, m_BackgroundColor.z, m_BackgroundColor.w);
		pDrawList->AddRectFilled(thumbnailClientMinPoint, thumbnailClientMaxPoint, bgColor, 7.0f);

		const ImVec2 thumbnailImageMinPoint = ImVec2(thumbnailClientMinPoint.x + 3.0f, thumbnailClientMinPoint.y + 3.0f);
		const ImVec2 thumbnailImageMaxPoint = ImVec2(thumbnailClientMaxPoint.x - 3.0f, thumbnailClientMaxPoint.y - ((thumbnailClientMaxPoint.y - thumbnailClientMinPoint.y) / 2.0f));

		constexpr const ImU32 imgColor = IM_COL32(255.0f, 255.0f, 255.0f, 255.0f);
		pDrawList->AddRectFilled(thumbnailImageMinPoint, thumbnailImageMaxPoint, imgColor, 1.0f);

		const ImVec2 thumbnailLineMinPoint = ImVec2(thumbnailClientMinPoint.x + 3.0f, thumbnailClientMaxPoint.y - ((thumbnailClientMaxPoint.y - thumbnailClientMinPoint.y) / 2.0f) + 1.0f);
		const ImVec2 thumbnailLineMaxPoint = ImVec2(thumbnailClientMaxPoint.x - 3.0f, thumbnailClientMaxPoint.y - ((thumbnailClientMaxPoint.y - thumbnailClientMinPoint.y) / 2.0f) + 1.0f);

		constexpr const ImU32 lineColor = IM_COL32(255.0f, 0.0f, 0.0f, 255.0f);
		pDrawList->AddLine(thumbnailLineMinPoint, thumbnailLineMaxPoint, lineColor, 2.0f);

		ImGui::SetCursorScreenPos(thumbnailImageMinPoint);

		const float imageWidth = static_cast<float>(thumbnailImageMaxPoint.x - thumbnailImageMinPoint.x);
		const float imageHeight = static_cast<float>(thumbnailImageMaxPoint.y - thumbnailImageMinPoint.y);

		const uint64 imageID = m_ImageIDCallback();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::ImageButton((ImTextureID)imageID, ImVec2(imageWidth, imageHeight));
		ImGui::PopStyleVar();

		ImGui::SetCursorScreenPos(ImVec2(thumbnailImageMinPoint.x, thumbnailImageMaxPoint.y + 5.0f));

		// Convert to window-local coordinates
		ImVec2 cursorPosLocal = ImGui::GetCursorPos();

		// Width available for the text inside the client rect
		const float textRegionWidth = (thumbnailClientMaxPoint.x - 3.0f) - (thumbnailClientMinPoint.x + 3.0f);

		ImGui::SetWindowFontScale(0.85f);
		ImGui::PushTextWrapPos(cursorPosLocal.x + textRegionWidth);
		ImGui::TextWrapped("MyMesh");
		ImGui::PopTextWrapPos();
		ImGui::SetWindowFontScale(1.0f);


		ImGui::SetWindowFontScale(0.75f);
		const float textHeight = ImGui::CalcTextSize("Mesh").y;
		ImGui::SetCursorScreenPos(ImVec2(thumbnailImageMinPoint.x, thumbnailClientMaxPoint.y - textHeight));
		ImGui::PushTextWrapPos(cursorPosLocal.x + textRegionWidth);
		ImGui::TextWrapped("Mesh");
		ImGui::PopTextWrapPos();
		ImGui::SetWindowFontScale(1.0f);

		ImGui::EndGroup();

		if (!m_IsHovered && ImGui::IsItemHovered())
		{ 
			m_IsHovered = true;
			m_OnMouseEnterCallback(this);
		}
		else if (m_IsHovered && !ImGui::IsItemHovered())
		{
			m_IsHovered = false;
			m_OnMouseExitCallback(this);
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
			m_OnClickedCallback(this, Mouse::CreatePointerInfo());
	}

}
