#include "UI.h"
#include "Assets/Factory/TextureFactory.h"
#include "Graphics/RHI/ResourceViews.h"

#include "Module/AssetToolsModule.h"
#include "Module/ModuleManager.h"

namespace Relentless
{
	namespace COLOR
	{
		constexpr const ImU32 SEARCHBAR_BACKGROUND = IM_COL32(17, 17, 17, 255);
		constexpr const ImU32 SEARCHBAR_BORDER = IM_COL32(60, 60, 60, 200);
		constexpr const ImU32 WIDGET_SELECTED = IM_COL32(30, 120, 255, 200);
		constexpr const ImU32 WIDGET_HOVERED = IM_COL32(100, 100, 100, 200);
	}

	AssetHandle UI::SearchIconTextureHandle		= NULL_HANDLE;
	AssetHandle UI::CancelIconTextureHandle		= NULL_HANDLE;
	AssetHandle UI::ArrowDownIconTextureHandle	= NULL_HANDLE;

	void UI::Initialize() noexcept
	{
		//RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\searchicon.rasset", s_GlobalData.SearchIconTextureHandle), "Core engine icon missing.");
		//RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\cancelicon.rasset", s_GlobalData.CancelIconTextureHandle), "Core engine icon missing.");
		//RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\arrowdownicon.rasset", s_GlobalData.ArrowDownIconTextureHandle), "Core engine icon missing.");

		std::vector<AssetImportTask> importTasks;
		importTasks.reserve(3);
		
		//auto&& CreateUIImportTask = [&importTasks](const Path& srcPath, AssetHandle& handleToSet)
		//	{
		//		Ref<TextureFactory> pTextureFactory = RLS_NEW TextureFactory();
		//		pTextureFactory->SetImportAsSRGB(true);
		//		pTextureFactory->OnDone.Connect([&](const ImportedAsset& asset, bool success)
		//			{
		//				RLS_VERIFY(success, "[OutlinerPanel] Error importing UI texture asset.");
		//				handleToSet = asset.Handle;
		//			});
		//
		//		AssetImportTask& importTask = importTasks.emplace_back();
		//		importTask.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, srcPath);
		//		importTask.pFactory = pTextureFactory;
		//	};
		//
		//CreateUIImportTask("Textures\\Icons\\searchicon.png", UI::SearchIconTextureHandle);
		//CreateUIImportTask("Textures\\Icons\\cancelicon.png", UI::CancelIconTextureHandle);
		//CreateUIImportTask("Textures\\Icons\\arrowdownicon.png", UI::ArrowDownIconTextureHandle);

		//Importer::RequestAsyncLoad(importTasks).Wait();

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		
		{
			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\searchicon.png");
			Ref<TextureFactory> pTextureFactory = RLS_NEW TextureFactory();
			pTextureFactory->SetImportAsSRGB(true);
			task.pFactory = pTextureFactory;
		}
		{
			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\cancelicon.png");
			Ref<TextureFactory> pTextureFactory = RLS_NEW TextureFactory();
			pTextureFactory->SetImportAsSRGB(true);
			task.pFactory = pTextureFactory;
		}
		{
			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\arrowdownicon.png");
			Ref<TextureFactory> pTextureFactory = RLS_NEW TextureFactory();
			pTextureFactory->SetImportAsSRGB(true);
			task.pFactory = pTextureFactory;
		}

		const std::vector<AssetImportResult> importResults = assetToolsModule.Import(importTasks);
		UI::SearchIconTextureHandle = importResults[0].Handle;
		UI::CancelIconTextureHandle = importResults[1].Handle;
		UI::ArrowDownIconTextureHandle = importResults[2].Handle;
	}

	std::string UI::Utility::ShortenStringToFitClipRect(const std::string& originalString, const ImVec2& topLeft, const ImVec2& bottomRight) noexcept
	{
		// Calculate available width from the provided clip rect
		const float availableHeight = bottomRight.y - topLeft.y;

		// Measure the original string
		const ImVec2 textSize = ImGui::CalcTextSize(originalString.c_str());
		const uint32 possibleNrOfRows = static_cast<uint32>(std::floor(availableHeight / textSize.y));
		const float availableWidth = (bottomRight.x - topLeft.x) * possibleNrOfRows;

		// If the string fits within the available width, no need to shorten
		if (textSize.x <= availableWidth) 
		{
			return originalString;
		}

		constexpr const char* ellipsis = "...";
		const ImVec2 ellipsisSize = ImGui::CalcTextSize(ellipsis);

		// Ensure available width can at least fit the ellipsis
		if (availableWidth <= ellipsisSize.x) 
		{
			// If available width is too small even for ellipsis, return as much as we can fit
			return "";
		}

		std::string modified = originalString;
		// Iteratively remove characters until the string plus ellipsis fits
		while (!modified.empty() && (ImGui::CalcTextSize((modified + ellipsis).c_str()).x > availableWidth)) 
		{
			modified.pop_back(); // Remove one character from the end
		}

		return modified + ellipsis;
	}

	void UI::Utility::DrawTitledSeparator(const std::string& title, const ImVec2& begin, const ImVec2&) noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;
		
		const float windowWidth = ImGui::GetWindowWidth();

		ImGui::SetWindowFontScale(0.8f);
		const ImVec2 textSize = ImGui::CalcTextSize(title.c_str());

		constexpr ImU32 textColor = IM_COL32(255.0f, 255.0f, 255.0f, 128.0f);
		pDrawList->AddText(ImVec2(begin.x + 10.0f, begin.y - (textSize.y / 2.0f)), textColor, title.c_str());
		
		ImGui::SetWindowFontScale(1.0f);

		float remainingWidth = windowWidth - textSize.x;

		ImGui::SetCursorScreenPos(ImVec2(begin.x + 10.0f + textSize.x + 20.0f, begin.y));

		constexpr const ImVec4 splitterColor = ImVec4(200.0f / 255, 200.0f / 255, 200.0f / 255, 128.0f / 255.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, splitterColor);
		
		constexpr float splitterThickness = 1.0f;
		ImGui::Button("##Splitter2", ImVec2(remainingWidth - 30.0f - 15.0f, splitterThickness));
		
		ImGui::PopStyleColor();
	}

	void UI::Utility::DrawTooltip(const char* tooltip) noexcept
	{
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::SetTooltip(tooltip);
		ImGui::PopStyleColor(2);
	}

	float UI::Utility::CalculateTextHeight(const std::string& text, float wrapWidth) noexcept
	{
		const float paddingTop = GImGui->Style.FramePadding.y;
		const float paddingBottom = GImGui->Style.FramePadding.y;

		return ImGui::CalcTextSize(text.c_str(), nullptr, false, wrapWidth).y + paddingTop + paddingBottom;
	}

}