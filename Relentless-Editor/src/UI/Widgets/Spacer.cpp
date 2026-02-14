#include "Spacer.h"

namespace Relentless
{
	Vector2 Spacer::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		const bool fixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool fixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		if (fixedWidth)
			size.x = GetFixedWidth();
		if (fixedHeight)
			size.y = GetFixedHeight();

		if (fixedWidth && fixedWidth)
			return size;

		size = Vector2(40.0f, 40.0f);
		return size;
	}

	void Spacer::OnRender() noexcept
	{
		const Vector2 size = GetAssignedSize();
		ImGui::Dummy(ImVec2(size.x, size.y));
	}

	bool Spacer::RequiresAssignedSize() const noexcept
	{
		return true;
	}
}