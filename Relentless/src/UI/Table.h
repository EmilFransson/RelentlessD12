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
	class Table : public IWidget
	{
	public:
		Table(std::string_view id) noexcept;

		void Add(Ref<IWidget> pWidget, uint32 column, uint32 row) noexcept;

		[[nodiscard]] bool HasWidget(Ref<IWidget> pWidget) const noexcept;
	protected:
		virtual void OnRender() noexcept override;
	private:
		void SetColorsAndStyles() noexcept;
	private:
		std::unordered_map<std::pair<uint32, uint32>, std::vector<Ref<IWidget>>> m_Cells;

		uint32 m_NumColumns = 0u;
		uint32 m_NumRows = 0u;
	};
}