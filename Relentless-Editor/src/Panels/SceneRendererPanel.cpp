#include "SceneRendererPanel.h"

namespace Relentless
{
	void SceneRendererPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		PROFILE_FUNC;

		ImGui::Begin("Scene Renderer");

		DrawColumnSection("MSAA", 120u, [&]()
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

		DrawColumnSection("VSync", 120u, [&]()
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

		DrawColumnSection("Contact Shadows", 120u, [&]()
			{
				ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const char* ContactShadowsStrings[] = { "Off", "HBAO+" };
		const char* currentContactShadowsString = ContactShadowsStrings[(int)m_pContext->GetOptions().ContactShadowType];

		if (ImGui::BeginCombo("##ContactShadows", currentContactShadowsString))
		{
			for (uint8_t i = 0u; i < ARRAYSIZE(ContactShadowsStrings); ++i)
			{
				bool isSelected = currentContactShadowsString == ContactShadowsStrings[i];
				if (ImGui::Selectable(ContactShadowsStrings[i], isSelected))
				{
					if (!isSelected)
					{
						ContactShadows contactShadowsType{};
						switch (i)
						{
						case (uint8_t)ContactShadows::OFF:
							contactShadowsType = ContactShadows::OFF;
							break;
						case (uint8_t)ContactShadows::HBAO_PLUS:
							contactShadowsType = ContactShadows::HBAO_PLUS;
							break;
						}

						m_DeferredFunctionCalls.push([this, contactShadowsType]()
							{
								m_pContext->SetContactShadowsType(contactShadowsType);
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

		if (m_pContext->GetOptions().ContactShadowType == ContactShadows::HBAO_PLUS)
		{
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
			

			GFSDK_SSAO_Parameters& parameters = m_pContext->GetHBAOPlusParameters();
			DrawColumnSection("Radius", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					ImGui::DragFloat("##HBAO_PLUS_RADIUS", &parameters.Radius, 0.06f, 0.0f, FLT_MAX);

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Bias", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					ImGui::DragFloat("##HBAO_PLUS_BIAS", &parameters.Bias, 0.006f, 0.0f, 0.5f);

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Small scale AO", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

			ImGui::DragFloat("##HBAO_PLUS_SmallScaleAO", &parameters.SmallScaleAO, 0.006f, 0.0f, 2.0f);

			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Large scale AO", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					ImGui::DragFloat("##HBAO_PLUS_LargeScaleAO", &parameters.LargeScaleAO, 0.006f, 0.0f, 2.0f);

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Power", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					ImGui::DragFloat("##HBAO_PLUS_Power", &parameters.PowerExponent, 0.006f, 1.0f, 4.0f);

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Steps", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					const char* StepsStrings[] = { "4", "8" };
					const char* currentStepsString = StepsStrings[parameters.StepCount == GFSDK_SSAO_STEP_COUNT_4 ? 0 : 1];

					if (ImGui::BeginCombo("##StepsCombo", currentStepsString))
					{
						for (uint8_t i = 0u; i < ARRAYSIZE(StepsStrings); ++i)
						{
							bool isSelected = currentStepsString == StepsStrings[i];
							if (ImGui::Selectable(StepsStrings[i], isSelected))
							{
								if (!isSelected)
								{
									parameters.StepCount = i == 0 ? GFSDK_SSAO_STEP_COUNT_4 : GFSDK_SSAO_STEP_COUNT_8;
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Precision", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					const char* PrecisionStrings[] = { "16 bit", "32 bit" };
					const char* currentPrecisionString = PrecisionStrings[parameters.DepthStorage == GFSDK_SSAO_FP16_VIEW_DEPTHS ? 0 : 1];

					if (ImGui::BeginCombo("##Precision", currentPrecisionString))
					{
						for (uint8_t i = 0u; i < ARRAYSIZE(PrecisionStrings); ++i)
						{
							bool isSelected = currentPrecisionString == PrecisionStrings[i];
							if (ImGui::Selectable(PrecisionStrings[i], isSelected))
							{
								if (!isSelected)
								{
									parameters.DepthStorage = i == 0 ? GFSDK_SSAO_FP16_VIEW_DEPTHS : GFSDK_SSAO_FP32_VIEW_DEPTHS;
								}
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			DrawColumnSection("Blur", 120u, [&]()
				{
					ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

					bool enabled = (bool)parameters.Blur.Enable;
					if (ImGui::Checkbox("##HBAO_BLUR", &enabled))
					{
						parameters.Blur.Enable = enabled;
					}
			
					ImGui::PopItemWidth();
					ImGui::PopStyleVar();
				}, false);

			if (parameters.Blur.Enable)
			{
				DrawColumnSection("Blur sharpness", 120u, [&]()
					{
						ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

						ImGui::DragFloat("##HBAO_PLUS_BlurSharpness", &parameters.Blur.Sharpness, 0.06f, 0.0f, 16.0f);
	
						ImGui::PopItemWidth();
						ImGui::PopStyleVar();
					}, false);

				DrawColumnSection("Blur radius", 120u, [&]()
					{
						ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

						const char* RadiusStrings[] = { "2", "4" };
						const char* currentRadiusString = RadiusStrings[parameters.Blur.Radius == GFSDK_SSAO_BLUR_RADIUS_2 ? 0 : 1];

						if (ImGui::BeginCombo("##HBAO_PLUS_BlurRadius", currentRadiusString))
						{
							for (uint8_t i = 0u; i < ARRAYSIZE(RadiusStrings); ++i)
							{
								bool isSelected = currentRadiusString == RadiusStrings[i];
								if (ImGui::Selectable(RadiusStrings[i], isSelected))
								{
									if (!isSelected)
									{
										parameters.Blur.Radius = i == 0 ? GFSDK_SSAO_BLUR_RADIUS_2 : GFSDK_SSAO_BLUR_RADIUS_4;
									}
								}
								if (isSelected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						ImGui::PopItemWidth();
						ImGui::PopStyleVar();
					}, false);
			}

		}
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
	void SceneRendererPanel::DrawColumnSection(const char* label, uint32_t columnWidth, const LambdaFunction&& invocable, bool border) noexcept
	{
		ImGui::Columns(2, (const char*)0, border);
		ImGui::SetColumnWidth(0, static_cast<float>(columnWidth));
		ImGui::Text(label);
		ImGui::NextColumn();

		std::invoke(invocable);

		ImGui::Columns(1);
	}
}