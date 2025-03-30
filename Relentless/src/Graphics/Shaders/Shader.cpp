#include "Shader.h"
#include "ShaderCompiler.h"
#include "Graphics/D3D12Debug.h"

namespace Relentless
{
	Shader::Shader(const ShaderType shaderType, const std::string& name, const char* pEntryPoint, const std::vector<std::string>& defines, Microsoft::WRL::ComPtr<IDxcBlob> pBuffer) noexcept
		: m_Type{ shaderType },
		  m_Name{ name },
		  m_EntryPoint{ pEntryPoint },
		  m_pBuffer{ std::move(pBuffer) },
		  m_Defines{ defines }
	{}

	std::shared_ptr<Shader> Shader::Create(const ShaderType type, const std::string& fileName, const char* pEntryPoint, Span<std::string> defines) noexcept
	{
		return ShaderCompiler::CompileFromFile(type, fileName, pEntryPoint, defines);
	}
}