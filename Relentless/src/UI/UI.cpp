#include "UI.h"
#include "Assets/AssetManager.h"

namespace Relentless
{
	namespace COLOR
	{
		constexpr const ImU32 SEARCHBAR_BACKGROUND = IM_COL32(17, 17, 17, 255);
		constexpr const ImU32 SEARCHBAR_BORDER = IM_COL32(60, 60, 60, 200);
		constexpr const ImU32 WIDGET_SELECTED = IM_COL32(30, 120, 255, 200);
		constexpr const ImU32 WIDGET_HOVERED = IM_COL32(100, 100, 100, 200);
	}

	struct GlobalData
	{
		AssetHandle SearchIconTextureHandle = NULL_HANDLE;
		AssetHandle CancelIconTextureHandle = NULL_HANDLE;
	};

	static GlobalData s_GlobalData;

	class SearchBar
	{
	public:
		[[nodiscard]] std::string Draw(float width, const char* hintText) noexcept
		{
			ImGui::PushID(this);

			ImGui::PushItemWidth(width);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f));

			const ImU32 borderColor = m_IsActive ? COLOR::WIDGET_SELECTED : m_IsHovered ? COLOR::WIDGET_HOVERED : COLOR::SEARCHBAR_BORDER;
			ImGui::PushStyleColor(ImGuiCol_Border, borderColor);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, COLOR::SEARCHBAR_BACKGROUND);

			const ImVec2 cursorPositionPreSearchBar = ImGui::GetCursorPos();

			struct CallbackUserData
			{
				std::string Input;
			} callbackUserData;

			//We do custom hint solution below
			ImGui::InputTextWithHint("##SearchBar", nullptr, m_FullInputBuffer, IM_ARRAYSIZE(m_FullInputBuffer), ImGuiInputTextFlags_CallbackAlways,
				[](ImGuiInputTextCallbackData* data) -> int
				{
					constexpr const uint8_t paddingLength = 8u;

					if (data->EventFlag != ImGuiInputTextFlags_CallbackAlways)
						return 0;

					for (uint8_t i = 0; i < paddingLength; ++i)
					{
						if (data->Buf[i] != ' ')
						{
							data->InsertChars(i, " ");
							data->CursorPos += 1;
						}
					}

					const bool searchBarContainsText = paddingLength < strlen(data->Buf);
					if (searchBarContainsText)
					{
						CallbackUserData* pUserData = static_cast<CallbackUserData*>(data->UserData);
						pUserData->Input = std::string(data->Buf + paddingLength);
					}

					return 0;
				}, &callbackUserData);

			ImGui::SetItemAllowOverlap();
			m_IsActive = ImGui::IsItemActive();
			m_IsHovered = ImGui::IsItemHovered();

			const bool textInInputField = strlen(m_FullInputBuffer) > 8;
			if (!textInInputField)
			{
				//Draw hint text:
				ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 32.0f, cursorPositionPreSearchBar.y + 5.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
				ImGui::Text(hintText);
				ImGui::PopStyleVar();
			}

			const ImVec2 cursorPositionPostSearchBar = ImGui::GetCursorPos();

			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(3);
			ImGui::PopItemWidth();

			if (!textInInputField)
			{
				ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 3.0f, cursorPositionPreSearchBar.y + 5.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

				const Texture& texture = AssetManager::Get<Texture2D>(s_GlobalData.SearchIconTextureHandle);
				constexpr const ImVec4 tintCol = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
				ImGui::ImageButton((ImTextureID)texture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);
				const bool searchIconHovered = ImGui::IsItemHovered();
				if (searchIconHovered)
					ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);
				
				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar();
			}
			else
			{
				ImGui::SetCursorPos(ImVec2(cursorPositionPreSearchBar.x + 6.0f, cursorPositionPreSearchBar.y + 8.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

				const Texture& texture = AssetManager::Get<Texture2D>(s_GlobalData.CancelIconTextureHandle);
				const ImVec4 tintCol = m_CancelIconHovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
				ImGui::ImageButton((ImTextureID)texture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(15.0f, 15.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), tintCol);

				m_CancelIconHovered = ImGui::IsItemHovered();
				if (m_CancelIconHovered)
					ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);

				if (ImGui::IsItemClicked())
				{
					memset(m_FullInputBuffer, 0, sizeof(m_FullInputBuffer));
				}

				ImGui::PopStyleColor(2);
				ImGui::PopStyleVar();
			}

			ImGui::SetCursorPos(cursorPositionPostSearchBar);

			ImGui::PopID();
			return callbackUserData.Input;
		}
	private:
		char m_FullInputBuffer[128] = "        ";
		bool m_IsActive = false;
		bool m_IsHovered = false;
		bool m_CancelIconHovered = false;
	};

	struct UIElements
	{
		std::unordered_map<const char*, SearchBar> SearchBars;
	};

	static UIElements s_UIElements;

	void UI::Initialize() noexcept
	{
		s_GlobalData.SearchIconTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\searchicon.rasset");
		s_GlobalData.CancelIconTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\cancelicon.rasset");
	}

	std::string UI::SearchBar(const char* uniqueID, const char* hintText, float width) noexcept
	{
		return s_UIElements.SearchBars[uniqueID].Draw(width, hintText);
	}
}