#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Outliner : public Tree
	{
	public:
		Outliner(const char* id, const std::shared_ptr<TreeDataView>& dataView) noexcept;
		virtual ~Outliner() noexcept override = default;
	private:
	};
}