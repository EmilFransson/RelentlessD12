#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Outliner : public Tree
	{
	public:
		Outliner(const std::shared_ptr<TreeDataView>& dataView) noexcept;
		virtual ~Outliner() noexcept override = default;
	private:
		virtual [[nodiscard]] const char* GetID() const noexcept override;
	};
}