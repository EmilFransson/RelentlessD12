#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IDetailsView : public IWidget<IDetailsView>
	{
	public:
		virtual ~IDetailsView() noexcept = default;
		NO_DISCARD bool IsLocked() const noexcept;
		
		void RequestRefresh() noexcept;
	protected:
		bool m_ShouldRefresh = true;
	private:
		bool m_IsLocked = false;
	};
}