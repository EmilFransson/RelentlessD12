#include "MetricsPanel.h"
#include "ImGui/ImGuiLayer.h"

namespace Relentless
{
	void MetricsPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		PROFILE_FUNC;
		ImGui::Begin("Metrics");

		ImGuiTabBarFlags flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("MyTabBar", flags))
		{
			if (ImGui::BeginTabItem("Performance"))
			{
				ImGui::Text("FPS: %d", Time::GetFramesPerSecond());
				ImGui::Separator();

				static bool sortDurations = false;

				ImGui::Checkbox("Sorted", &sortDurations);
				if (sortDurations)
				{
					std::sort(ProfilerManager::ProfilerMetrics.begin(), ProfilerManager::ProfilerMetrics.end(), [&](const ProfilerMetrics& lhs, const ProfilerMetrics& rhs)
						{
							return (lhs.durationInMilliSeconds > rhs.durationInMilliSeconds);
						});
				}

				for (auto& metric : ProfilerManager::ProfilerMetrics)
				{
					ImGui::Text("%.3fms", metric.durationInMilliSeconds);
					ImGui::SameLine();
					ImGui::Text("%s", metric.ContextName);
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

		ProfilerManager::ClearData();
	}
}