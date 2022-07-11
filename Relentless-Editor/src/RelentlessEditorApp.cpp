#include <Relentless.h>
#include "Relentless/EntryPoint.h"

namespace Relentless
{
	class RelentlessEditor : public Application 
	{
	public:
		RelentlessEditor() noexcept = default;
		virtual ~RelentlessEditor() override = default;
	};

	const std::unique_ptr<Application> CreateApplication() noexcept
	{
		return std::unique_ptr<RelentlessEditor>(new RelentlessEditor());
	}
}