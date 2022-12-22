#pragma once
#pragma once
#include <Relentless.h>
namespace Relentless
{
	class ContentBrowserPanel
	{
	public:
		explicit ContentBrowserPanel() noexcept = default;
		~ContentBrowserPanel() noexcept = default;
		void OnImGuiRender() noexcept;
	};
}