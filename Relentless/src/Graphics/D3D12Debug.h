#pragma once
namespace Relentless
{
	class D3D12Debug
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12InfoQueue>& GetInfoQueue() noexcept { return m_pInfoQueue; }
	private:
		D3D12Debug() noexcept = default;
		~D3D12Debug() noexcept = default;
	private:
		static Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_pInfoQueue;
	};
}

#ifndef NAME_D12_OBJECT
	#define NAME_D12_OBJECT(obj, name) DXCall(obj->SetName(name)); 
#endif //NAME_D12_OBJECT
#ifndef NAME_D12_OBJECT_INDEXED
	#define NAME_D12_OBJECT_INDEXED(obj, name, index)	\
			{	\
				std::wstring finalName = std::wstring(name) + std::wstring(L"[") + std::to_wstring(index) + std::wstring(L"]");	\
				DXCall(obj->SetName(finalName.c_str()));	\
			}
#endif //NAME_D12_OBJECT_INDEXED

#if defined(RLS_DEBUG)
	#ifndef DXCall
		#define DXCall(function)	\
		{	\
			if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					D3D12Debug::GetInfoQueue()->ClearStoredMessages();	\
				}	\
			HRESULT hr = (function);	\
			if (FAILED(hr))	\
			{	\
				std::stringstream ss;	\
				ss << "Direct3D12 has encountered a critical error.\n";	\
				ss << "File: " << __FILE__ << "\n";	\
				ss << "Function: " << __FUNCTION__ << "\n";	\
				ss << "Line: " << __LINE__ << "\n";	\
				if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					for (uint32_t messageIndex{0u}; messageIndex < D3D12Debug::GetInfoQueue()->GetNumStoredMessages(); messageIndex++)	\
						{	\
							SIZE_T messageLength{0u};	\
							D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, NULL, &messageLength);	\
							std::unique_ptr<D3D12_MESSAGE> pMessage = std::unique_ptr<D3D12_MESSAGE>(RLS_NEW D3D12_MESSAGE[messageLength]);	\
							D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, pMessage.get(), &messageLength);	\
							ss << "D3D12 Message [" << std::to_string(messageIndex + 1) << "]: ";	\
							ss << pMessage->pDescription;	\
							ss << "\n\n";	\
						}	\
				}	\
				RLS_ASSERT(false, ss.str());	\
			}	\
		}
	#endif //DXCall

	#ifndef DXCall_STD
		#define DXCall_STD(function2)	\
		{	\
			D3D12Debug::GetInfoQueue()->ClearStoredMessages();	\
			(function2);	\
			if (D3D12Debug::GetInfoQueue()->GetNumStoredMessages() > 0)	\
			{	\
				std::stringstream ss2;	\
				ss2 << "Direct3D12 has encountered a critical error.\n";	\
				ss2 << "File: " << __FILE__ << "\n";	\
				ss2 << "Function: " << __FUNCTION__ << "\n";	\
				ss2 << "Line: " << __LINE__ << "\n";	\
				for (uint32_t messageIndex2{0u}; messageIndex2 < D3D12Debug::GetInfoQueue()->GetNumStoredMessages(); messageIndex2++)	\
				{	\
					SIZE_T messageLength2{0u};	\
					D3D12Debug::GetInfoQueue()->GetMessage(messageIndex2, NULL, &messageLength2);	\
					std::unique_ptr<D3D12_MESSAGE> pMessage2 = std::unique_ptr<D3D12_MESSAGE>(RLS_NEW D3D12_MESSAGE[messageLength2]);	\
					D3D12Debug::GetInfoQueue()->GetMessage(messageIndex2, pMessage2.get(), &messageLength2);	\
					ss2 << "D3D12 Message [" << std::to_string(messageIndex2 + 1) << "]: ";	\
					ss2 << pMessage2->pDescription;	\
				}	\
				RLS_ASSERT(false, ss2.str());	\
			}	\
		}
	#endif //DXCall_STD

	#ifndef SERIALIZE_ROOT_SIGNATURE
		#define SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob)	\
		{	\
			Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob{nullptr};	\
			if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					D3D12Debug::GetInfoQueue()->ClearStoredMessages();	\
				}	\
			HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1_0, &pRootSignatureBlob, &pErrorBlob);	\
			if (FAILED(hr))	\
			{	\
				std::stringstream ss;	\
				ss << "Direct3D12 has encountered a critical error.\n";	\
				ss << "File: " << __FILE__ << "\n";	\
				ss << "Function: " << __FUNCTION__ << "\n";	\
				ss << "Line: " << __LINE__ << "\n";	\
				if (D3D12Debug::GetInfoQueue() != nullptr)	\
				{	\
					for (uint32_t messageIndex{ 0u }; messageIndex < D3D12Debug::GetInfoQueue()->GetNumStoredMessages(); messageIndex++)	\
					{	\
						SIZE_T messageLength{ 0u };	\
						D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, NULL, &messageLength);	\
						std::unique_ptr<D3D12_MESSAGE> pMessage = std::unique_ptr<D3D12_MESSAGE>(RLS_NEW D3D12_MESSAGE[messageLength]);	\
						D3D12Debug::GetInfoQueue()->GetMessage(messageIndex, pMessage.get(), &messageLength);	\
						ss << "D3D12 Message [" << std::to_string(messageIndex + 1) << "]: ";	\
						ss << pMessage->pDescription;	\
						ss << "\n\n";	\
					}	\
				}	\
				if (pErrorBlob)	\
				{	\
					ss << "Root Signature error blob message: ";	\
					ss << (char*)pErrorBlob->GetBufferPointer();	\
				}	\
				RLS_ASSERT(false, ss.str());	\
			}	\
		}
	#endif //SERIALIZE_ROOT_SIGNATURE

	#ifndef COMPILE_SHADER_FROM_FILE
		#define COMPILE_SHADER_FROM_FILE(shaderName, entryPoint, version, shaderBlob)	\
		{	\
			Microsoft::WRL::ComPtr<IDxcUtils> pUtils{ nullptr };	\
			DXCall(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));	\
				\
			Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource{ nullptr };	\
			std::wstring fullShaderName = ConvertStringToWstring(SHADER_DIRECTORY) + std::wstring(shaderName);	\
			RLS_ASSERT(std::filesystem::exists(fullShaderName), "Shader file does not exist.");	\
			RLS_ASSERT(fullShaderName.ends_with(".hlsl"), "Shader file extension is not supported.");	\
			uint32_t codePage = CP_UTF8;	\
			DXCall(pUtils->LoadFile(fullShaderName.c_str(), &codePage, &pSource));	\
				\
			std::vector<LPCWSTR> shaderArguments{};	\
				\
			shaderArguments.push_back(L"-E");	\
			shaderArguments.push_back(entryPoint);	\
				\
			shaderArguments.push_back(L"-T");	\
			shaderArguments.push_back(version);	\
				\
			shaderArguments.push_back(L"-Od");	\
			shaderArguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);	\
			shaderArguments.push_back(DXC_ARG_DEBUG);	\
				\
			DxcBuffer sourceBuffer = {};	\
			sourceBuffer.Ptr = pSource->GetBufferPointer();	\
			sourceBuffer.Size = pSource->GetBufferSize();	\
			sourceBuffer.Encoding = 0;	\
				\
			Microsoft::WRL::ComPtr<IDxcResult> pCompileResults{ nullptr };	\
				\
			Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler{ nullptr };	\
			DXCall(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));	\
			DXCall(pCompiler->Compile(&sourceBuffer, shaderArguments.data(), (uint32_t)shaderArguments.size(), nullptr, IID_PPV_ARGS(&pCompileResults)));	\
				\
			DXCall(pCompileResults->GetResult(&shaderBlob));	\
				\
			Microsoft::WRL::ComPtr<IDxcBlobUtf8> pErrorBlob{nullptr};	\
			DXCall(pCompileResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorBlob), nullptr));	\
			if (pErrorBlob && pErrorBlob->GetStringLength() > 0)	\
			{	\
				RLS_CORE_ERROR((char*)pErrorBlob->GetBufferPointer());	\
				__debugbreak();	\
			}	\
		}
	#endif //COMPILE_SHADER_FROM_FILE

#else
	#ifndef DXCall
		#define DXCall(function) function
	#endif //DXCall

	#ifndef DXCall_STD
		#define DXCall_STD(function) function
	#endif //DXCall_STD

	#ifndef SERIALIZE_ROOT_SIGNATURE
		#define SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob)	\
		{	\
			D3D12SerializeRootSignature(&rootSignatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1_0, &pRootSignatureBlob, nullptr);	\
		}
	#endif //SERIALIZE_ROOT_SIGNATURE

#ifndef COMPILE_SHADER_FROM_FILE
	#define COMPILE_SHADER_FROM_FILE(shaderName, entryPoint, version, shaderBlob)	\
		{	\
			Microsoft::WRL::ComPtr<IDxcUtils> pUtils{ nullptr };	\
			DXCall(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));	\
				\
			Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource{ nullptr };	\
			std::wstring fullShaderName = ConvertStringToWstring(SHADER_DIRECTORY) + std::wstring(shaderName);	\
			uint32_t codePage = CP_UTF8;	\
			DXCall(pUtils->LoadFile(fullShaderName.c_str(), &codePage, &pSource));	\
				\
			std::vector<LPCWSTR> shaderArguments{};	\
				\
			shaderArguments.push_back(L"-E");	\
			shaderArguments.push_back(entryPoint);	\
				\
			shaderArguments.push_back(L"-T");	\
			shaderArguments.push_back(version);	\
				\
			shaderArguments.push_back(L"-O3");	\
				\
			DxcBuffer sourceBuffer = {};	\
			sourceBuffer.Ptr = pSource->GetBufferPointer();	\
			sourceBuffer.Size = pSource->GetBufferSize();	\
			sourceBuffer.Encoding = 0;	\
				\
			Microsoft::WRL::ComPtr<IDxcResult> pCompileResults{ nullptr };	\
				\
			Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler{ nullptr };	\
			DXCall(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));	\
			DXCall(pCompiler->Compile(&sourceBuffer, shaderArguments.data(), (uint32_t)shaderArguments.size(), nullptr, IID_PPV_ARGS(&pCompileResults)));	\
				\
			DXCall(pCompileResults->GetResult(&shaderBlob));	\
		}
	#endif //COMPILE_SHADER_FROM_FILE
#endif