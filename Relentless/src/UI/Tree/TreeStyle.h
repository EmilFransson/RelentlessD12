#include "UI/UI.h"

namespace Relentless
{
	struct TableRowStyle
	{
		UI::Alignment Alignment = UI::Alignment::Left;
		ImVec4 LabelColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 IconTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		float IconWeight = 0.6f;
		float Spacing = -1.0f;
	};

	class TreeStyle
	{
	public:
		TreeStyle() noexcept = default;
		virtual ~TreeStyle() noexcept = default;

		void SetUseAlternatingRowColors(bool useState) noexcept;
		[[nodiscard]] bool IsUsingAlternateRowColors() const noexcept;

		//virtual [[nodiscard]] const TableRowStyle GetRowStyle([[maybe_unused]] const std::shared_ptr<TableData>& pTableData, [[maybe_unused]] uint32_t column) const noexcept { return m_DefaultRowStyle; }
	private:
		bool m_UseAlternatingColors = true;
	};
}