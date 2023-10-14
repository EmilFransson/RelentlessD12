#include "ShaderCompiler.h"
namespace Relentless
{
	std::shared_ptr<Shader> ShaderCompiler::CompileFromFile(const ShaderType shaderType, const std::string& fileName) noexcept
	{
		std::wstring fullShaderName = ConvertStringToWstring(SHADER_DIRECTORY) + ConvertStringToWstring(fileName);
		RLS_ASSERT(std::filesystem::exists(fullShaderName), "Shader file does not exist.");
		RLS_ASSERT(fullShaderName.ends_with(L".hlsl"), "Shader file extension is not supported.");

		Microsoft::WRL::ComPtr<IDxcBlob> pShaderBlob{ nullptr };
		Microsoft::WRL::ComPtr<IDxcUtils> pUtils{ nullptr };
		DXCall(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource{ nullptr };

		uint32_t codePage = CP_UTF8;
		DXCall(pUtils->LoadFile(fullShaderName.c_str(), &codePage, &pSource));

		std::vector<LPCWSTR> shaderArguments{};
		shaderArguments.push_back(L"-E");
		shaderArguments.push_back(shaderType == ShaderType::VERTEX ? L"vs_main" : L"ps_main");
		shaderArguments.push_back(L"-T");
		shaderArguments.push_back(shaderType == ShaderType::VERTEX ? L"vs_6_6" : L"ps_6_6");
#if defined RLS_DEBUG
		shaderArguments.push_back(L"-Od");
		shaderArguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
		shaderArguments.push_back(DXC_ARG_DEBUG);
#else
		shaderArguments.push_back(L"-O3");
#endif


		DxcBuffer sourceBuffer = {};
		sourceBuffer.Ptr = pSource->GetBufferPointer();
		sourceBuffer.Size = pSource->GetBufferSize();
		sourceBuffer.Encoding = 0;

		Microsoft::WRL::ComPtr<IDxcResult> pCompileResults{ nullptr };

		Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler{ nullptr };
		DXCall(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));
		DXCall(pCompiler->Compile(&sourceBuffer, shaderArguments.data(), (uint32_t)shaderArguments.size(), nullptr, IID_PPV_ARGS(&pCompileResults)));

		DXCall(pCompileResults->GetResult(&pShaderBlob));

#if defined RLS_DEBUG
		Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrorBlob{ nullptr };
		DXCall(pCompileResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorBlob), nullptr));
		if (pErrorBlob && pErrorBlob->GetStringLength() > 0)
		{
			RLS_CORE_CRITICAL("{}", (char*)pErrorBlob->GetBufferPointer());
			RLS_ASSERT(false, (char*)pErrorBlob->GetBufferPointer());
		}
#endif

		RLS_CORE_INFO("Compiled shader: '{0}'", fileName);

		return std::make_shared<Shader>(shaderType, fileName.substr(0, fileName.find_first_of(".")), std::move(pShaderBlob));
	}
}