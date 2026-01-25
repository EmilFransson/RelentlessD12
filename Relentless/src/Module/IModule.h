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

		NO_DISCARD virtual bool SupportsAutomaticShutdown() const { return true; }
	};
}