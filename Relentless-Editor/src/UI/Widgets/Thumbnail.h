#pragma once
#include "UI/Brush/Brush.h"
#include "UI/Widgets/IStylableWidget.h"

namespace Relentless
{
	class Thumbnail : public IStylableWidget<Thumbnail>
	{
	public:
		virtual ~Thumbnail() noexcept = default;
	
		NO_DISCARD Vector2 ReportSize() const noexcept;

		void SetBrush(const ThumbnailBrush& aBrush) noexcept;
		void SetSize(const Vector2& aSize) noexcept;
	protected:
		void OnRender() noexcept override;
	private:
		ThumbnailBrush m_Brush;
		Vector2 m_Size = Vector2::Zero;
	};
}