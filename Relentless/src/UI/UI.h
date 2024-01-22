#include "ImGui/ImguiLayer.h"

namespace Relentless
{
	class UI
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static std::string SearchBar(const char* uniqueID, const char* hintText, float width = ImGui::GetContentRegionAvail().x) noexcept;

	};
}