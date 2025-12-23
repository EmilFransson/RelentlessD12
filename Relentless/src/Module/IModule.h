#pragma once

namespace Relentless
{
	class IModule
	{
	public:
		virtual ~IModule() = default;
		
	protected:
		friend class ModuleManager;

		virtual void OnLoad() {};
		virtual void OnUnload() {};

		virtual NO_DISCARD bool SupportsAutomaticShutdown() const { return true; }
	};
}