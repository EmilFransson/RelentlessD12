#include "SceneRendererPanel.h"

namespace Relentless
{
	void SceneRendererPanel::OnImGuiRender() noexcept
	{
		ImGui::Begin("Scene Renderer");

		DrawColumnSection("MSAA", 100u, [&]()
			{
				ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

				const char* MSAAStrings[] = { "Off", "2x", "4x", "8x" };
				const char* currentMSAAString = MSAAStrings[(int)m_MSAAOption];

				if (ImGui::BeginCombo("##MSAA", currentMSAAString))
				{
					for (uint8_t i = 0u; i < ARRAYSIZE(MSAAStrings); ++i)
					{
						bool isSelected = currentMSAAString == MSAAStrings[i];
						if (ImGui::Selectable(MSAAStrings[i], isSelected))
						{
							if (!isSelected)
							{
								uint8_t newMSAAValue = i == 0 ? 1u : static_cast<uint8_t>(std::pow(2.0f, i));
								m_DeferredFunctionCalls.push([this, newMSAAValue]()
									{
										m_MSAAOption = newMSAAValue == 1 ? MSAA::OFF : newMSAAValue == 2 ? MSAA::TWO : newMSAAValue == 4 ? MSAA::FOUR : MSAA::EIGHT;
								m_pContext->SetMSAASamples(newMSAAValue);
									});
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::PopItemWidth();
				ImGui::PopStyleVar();
			});

		ImGui::Separator();

		DrawColumnSection("VSync", 100u, [&]() 
			{
				ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

				const char* VSyncStrings[] = { "Off", "On" };
				const char* currentVSyncString = VSyncStrings[(int)Window::IsVSyncing()];

				if (ImGui::BeginCombo("##Vsync", currentVSyncString))
				{
					for (uint8_t i = 0u; i < ARRAYSIZE(VSyncStrings); ++i)
					{
						bool isSelected = currentVSyncString == VSyncStrings[i];
						if (ImGui::Selectable(VSyncStrings[i], isSelected))
						{
							if (!isSelected)
							{
								Window::ToggleVSync();
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::PopItemWidth();
				ImGui::PopStyleVar();
			});

		ImGui::Separator();

		ImGui::End();
	}

	void SceneRendererPanel::SetActiveRenderer(std::shared_ptr<SceneRenderer> pSceneRenderer) noexcept
	{
		RLS_ASSERT(pSceneRenderer, "Scene Renderer is invalid");
		m_pContext = pSceneRenderer;
		Options options = pSceneRenderer->GetOptions();
		m_MSAAOption = options.MSAASamples == 1 ? MSAA::OFF : options.MSAASamples == 2 ? MSAA::TWO : options.MSAASamples == 4 ? MSAA::FOUR : MSAA::EIGHT;
	}

	void SceneRendererPanel::OnPostRender() noexcept
	{
		for (uint32_t i{ 0u }; i < m_DeferredFunctionCalls.size(); ++i)
		{
			std::invoke(m_DeferredFunctionCalls.front());
			m_DeferredFunctionCalls.pop();
		}
	}

	template<typename LambdaFunction>
	void SceneRendererPanel::DrawColumnSection(const char* label, uint32_t columnWidth, const LambdaFunction&& invocable) noexcept
	{
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, static_cast<float>(columnWidth));
		ImGui::Text(label);
		ImGui::NextColumn();

		std::invoke(invocable);

		ImGui::Columns(1);
	}
}