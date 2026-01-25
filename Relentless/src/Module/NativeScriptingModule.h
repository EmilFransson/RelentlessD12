#pragma once

#include "Module/IModule.h"

namespace Relentless
{
	struct ProcessResult
	{
		int ExitCode = -1;
		String Output;
	};
	
	struct ScriptEntry
	{
		HMODULE ModuleHandle = nullptr;
	};

	class NativeScriptingModule : public IModule
	{
	protected:
		virtual void OnLoad() override;
	private:
		bool EnsureConfigured(const Path& projectDir,
			const Path& scriptsDir,
			const Path& buildDir,
			const Path& stageDir,
			const Path& engineInclude,
			const Path& engineLibDir,
			const String& engineLibsSemicolonList, // "RelentlessCore;RelentlessEngine"
			bool forceReconfigure);

		ProcessResult RunProcess(const std::wstring& commandLine, const std::wstring& workingDirectory);
	};
}