#pragma once
#include "Shader.h"

namespace Relentless
{
	class ShaderCompiler
	{
	public:
		static [[nodiscard]] std::shared_ptr<Shader> CompileFromFile(const ShaderType shaderType, const std::string& fileName) noexcept;

	private:
		STATIC_CLASS(ShaderCompiler);
	};
}