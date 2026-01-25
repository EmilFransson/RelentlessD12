#pragma once
#include <Relentless.h>

namespace Relentless
{
	//Light weight marker interface for editor subsystems
	class IEditorSubsystem : public ISubsystem
	{
	public:
		virtual ~IEditorSubsystem() noexcept override = default;
	};
}