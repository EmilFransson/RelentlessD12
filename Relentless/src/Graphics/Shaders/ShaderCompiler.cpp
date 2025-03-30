#include "ShaderCompiler.h"

#include "File/File.h"
#include "Graphics/RHI/D3D.h"
#include "Utility/StringUtils.h"

namespace Relentless
{
	static Ref<IDxcUtils> pUtils{ nullptr };
	static Ref<IDxcCompiler3> pCompiler{ nullptr };
	static Ref<IDxcIncludeHandler> pDefaultIncludeHandler{ nullptr };
	static Ref<IDxcValidator> pValidator{ nullptr };

	std::shared_ptr<Shader> ShaderCompiler::CompileFromFile(const ShaderType shaderType, const std::string& fileName, const char* pEntryPoint, Span<std::string> defines) noexcept
	{
		std::wstring fullShaderName = StringUtils::ConvertToWide(SHADER_DIRECTORY) + StringUtils::ConvertToWide(fileName);
		RLS_ASSERT(File::Exists(fullShaderName), "Shader file does not exist.");
		RLS_ASSERT(fullShaderName.ends_with(L".hlsl"), "Shader file extension is not supported.");

		Ref<IDxcBlobEncoding> pSource{ nullptr };

		uint32_t codePage = CP_UTF8;
		VERIFY_HR(pUtils->LoadFile(fullShaderName.c_str(), &codePage, pSource.ReleaseAndGetAddressOf()));

		//auto&& GetEntryPoint = [](ShaderType type) -> LPCWSTR
		//{
		//	switch (type)
		//	{
		//	case ShaderType::VERTEX: return L"vs_main";
		//	case ShaderType::PIXEL: return L"ps_main";
		//	case ShaderType::Compute: return L"cs_main";
		//	default:
		//		RLS_ASSERT(false, "Unreachable");
		//		return L"";
		//	}
		//};

		auto&& GetProfile = [](ShaderType type) -> LPCWSTR
		{
			switch (type)
			{
			case ShaderType::VERTEX: return L"vs_6_7";
			case ShaderType::PIXEL: return L"ps_6_7";
			case ShaderType::Compute: return L"cs_6_7";
			default:
				RLS_ASSERT(false, "Unreachable");
				return L"";
			}
		};

		std::vector<LPCWSTR> shaderArguments{};
		shaderArguments.push_back(L"-E");
		const std::wstring entryPoint = StringUtils::ConvertToWide(pEntryPoint);
		shaderArguments.push_back(entryPoint.c_str());
		
		shaderArguments.push_back(L"-T");
		shaderArguments.push_back(GetProfile(shaderType));

		// Add include directory argument
		const std::wstring includePath = StringUtils::ConvertToWide(SHADER_DIRECTORY);
		shaderArguments.push_back(L"-I");
		shaderArguments.push_back(includePath.c_str());

		shaderArguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
		shaderArguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

#if defined RLS_DEBUG
		shaderArguments.push_back(DXC_ARG_DEBUG);
		shaderArguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS);
		shaderArguments.push_back(L"-Qembed_debug");  // Embed debug info into the blob
#else
		shaderArguments.push_back(L"-O3");
#endif

		shaderArguments.push_back(L"-HV");
		shaderArguments.push_back(L"2021");
		shaderArguments.push_back(L"-enable-16bit-types");

		std::vector<std::wstring> definesVector;
		for (const auto& define : defines)
		{
			shaderArguments.push_back(L"-D");
			definesVector.push_back(StringUtils::ConvertToWide(define));
			shaderArguments.push_back(definesVector.back().c_str());
		}

		DxcBuffer sourceBuffer = {};
		sourceBuffer.Ptr = pSource->GetBufferPointer();
		sourceBuffer.Size = pSource->GetBufferSize();
		sourceBuffer.Encoding = 0;

		Microsoft::WRL::ComPtr<IDxcBlob> pShaderBlob{ nullptr };
		Ref<IDxcResult> pCompileResults{ nullptr };
		VERIFY_HR(pCompiler->Compile(&sourceBuffer, shaderArguments.data(), (uint32_t)shaderArguments.size(), pDefaultIncludeHandler.Get(), IID_PPV_ARGS(pCompileResults.ReleaseAndGetAddressOf())));
		VERIFY_HR(pCompileResults->GetResult(pShaderBlob.ReleaseAndGetAddressOf()));

		Ref<IDxcBlobUtf8> pErrors;
		if (SUCCEEDED(pCompileResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(pErrors.GetAddressOf()), nullptr)))
		{
			if (pErrors && pErrors->GetStringLength())
			{
				const char* errorMessage = (char*)pErrors->GetStringPointer();
				RLS_CORE_CRITICAL("{}", errorMessage);
			}
		}

		HRESULT hrStatus;
		if (FAILED(pCompileResults->GetStatus(&hrStatus)) || FAILED(hrStatus))
		{
			//RLS_ASSERT(false, "FAILED TO COMPILE SHADER");
		}

		//Validation
		{
			Ref<IDxcOperationResult> pResult = nullptr;
			VERIFY_HR(pValidator->Validate(pShaderBlob.Get(), DxcValidatorFlags_InPlaceEdit, pResult.GetAddressOf()));
			HRESULT validationResult = S_FALSE;
			pResult->GetStatus(&validationResult);
			if (validationResult != S_OK)
			{
				Ref<IDxcBlobEncoding> pPrintBlob = nullptr;
				Ref<IDxcBlobUtf8> pPrintBlobUtf8 = nullptr;

				pResult->GetErrorBuffer(pPrintBlob.GetAddressOf());
				pUtils->GetBlobAsUtf8(pPrintBlob.Get(), pPrintBlobUtf8.GetAddressOf());

				const char* errorMessage = pPrintBlobUtf8->GetStringPointer();

				RLS_CORE_CRITICAL("{}", errorMessage);
				RLS_ASSERT(false, errorMessage);
			}
		}

		RLS_CORE_INFO("Compiled shader: '{0}'", fileName);
		return std::make_shared<Shader>(shaderType, fileName.substr(0, fileName.find_first_of(".")), pEntryPoint, defines.Copy(), std::move(pShaderBlob));
	}

	void ShaderCompiler::LoadDXC() noexcept
	{
		VERIFY_HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.ReleaseAndGetAddressOf())));
		VERIFY_HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler.ReleaseAndGetAddressOf())));
		VERIFY_HR(DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(pValidator.ReleaseAndGetAddressOf())));
		VERIFY_HR(pUtils->CreateDefaultIncludeHandler(pDefaultIncludeHandler.GetAddressOf()));
	}
}