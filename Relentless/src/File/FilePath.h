#pragma once

namespace Relentless
{
	class FilePath
	{
	public:
		using path = std::filesystem::path;

		NO_DISCARD static std::filesystem::path GetEngineWorkingDirectory()
		{
			return std::filesystem::path(MAIN_ENGINE_DIRECTORY);
		}

		NO_DISCARD static bool IsSubpath(const std::filesystem::path& childIn, const std::filesystem::path& parentIn)
		{
			std::filesystem::path child = childIn.lexically_normal();
			std::filesystem::path parent = parentIn.lexically_normal();

			if (child == parent) return false;

			// Root names (drive letters) must match ignoring case
			const auto sa = child.root_name().native();
			const auto sb = parent.root_name().native();
			if (sa.size() != sb.size()) return false;
			for (size_t i = 0; i < sa.size(); ++i)
				if (std::towlower(sa[i]) != std::towlower(sb[i])) return false;

			// Compare path segments ignoring case
			auto ci = child.begin(), ce = child.end();
			auto pi = parent.begin(), pe = parent.end();

			for (; pi != pe; ++pi, ++ci)
			{
				if (ci == ce) return false; // parent longer than child
				const auto segC = ci->native();
				const auto segP = pi->native();

				if (segC.size() != segP.size()) return false;
				for (size_t i = 0; i < segC.size(); ++i)
					if (std::towlower(segC[i]) != std::towlower(segP[i])) return false;
			}

			// parent exhausted, child has extra segments → subpath
			return ci != ce;
		}

	private:

	};

}