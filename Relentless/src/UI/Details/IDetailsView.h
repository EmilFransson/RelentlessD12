#pragma once
#include "src/UI/IWidget.h"

namespace Relentless
{
	class IDetailsCustomizer;

	class IDetailsView : public ICompoundWidget<IDetailsView>
	{
	public:
		IDetailsView() noexcept = default;

		virtual ~IDetailsView() noexcept override = default;
	};
}