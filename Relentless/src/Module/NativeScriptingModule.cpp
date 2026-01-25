#include "NativeScriptingModule.h"
#include "File/File.h"
#include "Project/Project.h"
#include "Utility/FilepathUtils.h"

namespace Relentless
{
	void NativeScriptingModule::OnLoad()
	{
		const Path projectDir = Project::GetProjectDirectory();
		const Path scriptsDir = FilepathUtils::Combine(projectDir, "Scripts");
		const Path buildDir = FilepathUtils::Combine(projectDir, "Intermediate/NativeScriptsBuild");
		const Path outDir = FilepathUtils::Combine(buildDir, "out");
		const Path stageDir = FilepathUtils::Combine(projectDir, "Intermediate/NativeScriptsStage");

		//const Path engineInclude = GetEngineIncludeDir(); // where Relentless.h lives
		//const Path engineLibDir = GetEngineLibDir();     // where .lib lives
		//const std::string engineLibs = GetEngineLibsString(); // "RelentlessCore;RelentlessEngine"
		//
		//if (!EnsureConfigured(projectDir, scriptsDir, buildDir, stageDir, engineInclude, engineLibDir, engineLibs, /*force*/false))
		//{
		//	RLS_CORE_ERROR("NativeScripts: CMake configure failed.");
		//	return;
		//}
	}

	bool NativeScriptingModule::EnsureConfigured(
		const Path& /*projectDir*/,
		const Path& scriptsDir,     // projectDir / "Scripts"
		const Path& buildDir,       // projectDir / "Intermediate/NativeScriptsBuild"
		const Path& stageDir,      // projectDir / "Intermediate/NativeScriptsStage"
		const Path& engineInclude,
		const Path& engineLibDir,
		const std::string& engineLibsSemicolonList, // "RelentlessCore;RelentlessEngine"
		bool forceReconfigure)
	{
		// Heuristic: if buildDir/CMakeCache.txt exists -> configured
		const Path cache = FilepathUtils::Combine(buildDir, "CMakeCache.txt");
		if (!forceReconfigure && File::Exists(cache))
			return true;

		FilepathUtils::CreateDirectoryTree(buildDir);
		FilepathUtils::CreateDirectoryTree(stageDir);

		// NOTE: Use Ninja if you can. If not, use your generator.
		std::wstring cmd =
			L"cmake -S \"" + scriptsDir.wstring() + L"\""
			L" -B \"" + buildDir.wstring() + L"\""
			L" -G Ninja"
			L" -DRELENTLESS_ENGINE_INCLUDE=\"" + engineInclude.wstring() + L"\""
			L" -DRELENTLESS_ENGINE_LIBDIR=\"" + engineLibDir.wstring() + L"\""
			L" -DRELENTLESS_ENGINE_LIBS=\"" + std::wstring(engineLibsSemicolonList.begin(), engineLibsSemicolonList.end()) + L"\"";

		const ProcessResult r = RunProcess(cmd, buildDir.wstring());
		return r.ExitCode == 0;
	}

	ProcessResult NativeScriptingModule::RunProcess(const std::wstring& commandLine, const std::wstring& workingDirectory)
	{
		ProcessResult result{};

		STARTUPINFOW si{};
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi{};

		// CreateProcess requires a writable command buffer
		std::wstring cmd = commandLine;

		BOOL ok = ::CreateProcessW(
			nullptr,                  // application name (null = parsed from cmd)
			cmd.data(),               // command line (must be writable!)
			nullptr,                  // process security
			nullptr,                  // thread security
			FALSE,                    // inherit handles
			CREATE_NO_WINDOW,         // flags (no console window)
			nullptr,                  // environment (inherit)
			workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
			&si,
			&pi
		);

		if (!ok)
		{
			result.ExitCode = -1;
			return result;
		}

		// Wait until process exits
		::WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 0;
		::GetExitCodeProcess(pi.hProcess, &exitCode);

		result.ExitCode = static_cast<int>(exitCode);

		::CloseHandle(pi.hThread);
		::CloseHandle(pi.hProcess);

		return result;
	}
}