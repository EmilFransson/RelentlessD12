#pragma once

namespace Relentless
{
	class ISystemManager;

	class ISubsystem
	{
	public:
		virtual ~ISubsystem() noexcept = default;

		NO_DISCARD virtual bool OnLoad(ISystemManager*) noexcept { return true; }
		virtual void OnUnload(ISystemManager*) noexcept {}
		static bool ShouldCreateSubsystem(ISystemManager*) noexcept { return true; }
	};
}