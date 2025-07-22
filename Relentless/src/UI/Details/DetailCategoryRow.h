#pragma once
#include "UI/IWidget.h"

namespace Relentless
{
	class DetailCategoryRow : public ICompoundWidget<DetailCategoryRow>
	{
	public:
		friend class DetailCategoryNode;

		DetailCategoryRow() noexcept;
		virtual ~DetailCategoryRow() noexcept override = default;

		bool IsRenderingTable() const { return m_IsRenderingTable; }

		void Finish() noexcept; 

		void SetHeaderFlags(int flags) noexcept;
		void SetBorderLightColor(const Color& color) noexcept;

		void SetCellPadding(const Vector2& padding) noexcept;
		void SetSeparatorColor(const Color& color) noexcept;
		void SetSeparatorHoverColor(const Color& color) noexcept;
		void SetSeparatorActiveColor(const Color& color) noexcept;

		void SetTableFlags(int flags) noexcept;
	protected:
		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override { return 0.0f; };
		virtual void OnRender() noexcept override;
	private:
		bool m_IsRenderingTable = false;

		int m_TableFlags = 0;
		int m_HeaderFlags = 0;
	};
}