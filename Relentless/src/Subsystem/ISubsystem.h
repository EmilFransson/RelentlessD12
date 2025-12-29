#pragma once

namespace Relentless
{
	class ISystemManager;

	class ISubsystem
	{
	public:
		virtual ~ISubsystem() noexcept = default;

		virtual NO_DISCARD bool OnLoad(ISystemManager*) noexcept { return false; }
		virtual void OnUnload() noexcept {}
		static bool ShouldCreateSubsystem(ISystemManager*) noexcept { return true; }
	};
}