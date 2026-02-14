#pragma once

#include "IWidget.h"

namespace std {
	template <>
	struct hash<std::pair<uint32, uint32>>
	{
		std::size_t operator()(const std::pair<uint32_t, uint32_t>& p) const noexcept
		{
			std::size_t h1 = std::hash<uint32_t>{}(p.first);
			std::size_t h2 = std::hash<uint32_t>{}(p.second);
			return h1 ^ (h2 << 1); // Better distribution than simple bitpacking
		}
	};
}

namespace Relentless
{
	class Table : public IStylableWidget<Table>
	{
	public:
		Table() noexcept;

		template<typename T>
		T* Add(Ref<T> pWidget, uint32 column, uint32 row) noexcept;

		template<typename T>
		T* Add(T* pWidget, uint32 column, uint32 row) noexcept;

		NO_DISCARD bool HasWidget(Ref<IBaseWidget> pWidget) const noexcept;

		void SetCellPadding(const Vector2& padding) noexcept;
		void SetBorderLightColor(const Color& color) noexcept;
		void SetSeparatorColor(const Color& color) noexcept;
		void SetSeparatorHoverColor(const Color& color) noexcept;
		void SetSeparatorActiveColor(const Color& color) noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		std::unordered_map<std::pair<uint32, uint32>, std::vector<Ref<IBaseWidget>>> m_Cells;

		uint32 m_NumColumns = 0u;
		uint32 m_NumRows = 0u;
	};

	template<typename T>
	T* Table::Add(T* pWidget, uint32 column, uint32 row) noexcept
	{
		static_assert(std::is_base_of_v<IBaseWidget, T>, "[Table::Add]: Can only Add widgets derived from IWidget");

		Ref<T> widgetRef(pWidget);
		return Add(widgetRef, column, row);
	}

	template<typename T>
	T* Table::Add(Ref<T> pWidget, uint32 column, uint32 row) noexcept
	{
		RLS_ASSERT(!HasWidget(pWidget), "[Table::Add] Widget already assigned as child.");
		m_Cells[{column, row}].push_back(pWidget);

		m_NumRows = Math::Max(m_NumRows, row + 1);
		m_NumColumns = Math::Max(m_NumColumns, column + 1);

		return pWidget.Get();
	}

}