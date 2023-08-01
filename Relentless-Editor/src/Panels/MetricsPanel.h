#pragma once
#include <Relentless.h>
namespace Relentless
{
	class MetricsPanel
	{
	public:
		explicit MetricsPanel() noexcept = default;
		~MetricsPanel() noexcept = default;
		void OnImGuiRender(const bool show) noexcept;
	};
}