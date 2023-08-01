#include "ShaderReflector.h"
#include "../D3D12Core.h"
#include "../Renderer/Properties.h"
namespace Relentless
{

	static uint32_t GetVariableSize(const std::string& variableName, const Microsoft::WRL::ComPtr<ID3D12ShaderReflection>& pShaderReflection) noexcept
	{
		D3D12_SHADER_DESC shaderDesc;
		pShaderReflection->GetDesc(&shaderDesc);

		for (UINT i = 0; i < shaderDesc.ConstantBuffers; i++) {
			ID3D12ShaderReflectionConstantBuffer* pConstantBuffer = pShaderReflection->GetConstantBufferByIndex(i);

			D3D12_SHADER_BUFFER_DESC bufferDesc;
			pConstantBuffer->GetDesc(&bufferDesc);

			// Now we can inspect the variables within the constant buffer
			for (UINT j = 0; j < bufferDesc.Variables; j++) {
				ID3D12ShaderReflectionVariable* pVariable = pConstantBuffer->GetVariableByIndex(j);

				D3D12_SHADER_VARIABLE_DESC variableDesc;
				pVariable->GetDesc(&variableDesc);

				// variableDesc now contains information about this variable
				if (variableDesc.Name == variableName)
				{
					return variableDesc.Size;
				}
			}
		}
		return 0;
	}

	static D3D12_SHADER_VISIBILITY RLSShaderTypeToD3D12ShaderType(ShaderType shaderType) noexcept
	{
		switch (shaderType)
		{
		case ShaderType::VERTEX:
			return D3D12_SHADER_VISIBILITY_VERTEX;
			break;
		case ShaderType::PIXEL:
			return D3D12_SHADER_VISIBILITY_PIXEL;
			break;
		}

		RLS_ASSERT(false, "Unknown shader type encountered.");
		return D3D12_SHADER_VISIBILITY_ALL;
	}

	static D3D12_FILTER DetermineFilterType(const std::string& name) noexcept
	{
		if (name.find("_ANISOTROPIC") != std::string::npos)
		{
			return D3D12_FILTER_ANISOTROPIC;
		}
		else if (name.find("_LINEAR_CLAMP") != std::string::npos)
		{
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
		else if (name.find("_LINEAR") != std::string::npos)
		{
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
		else
		{
			RLS_ASSERT(false, "Unknown sampler filter encountered.");
			return D3D12_FILTER_ANISOTROPIC;
		}
	}


	ReflectionResult ShaderReflector::Reflect(const std::shared_ptr<Shader>& pVertexShader, const std::shared_ptr<Shader>& pPixelShader) noexcept
	{
		RLS_ASSERT(pVertexShader && (pVertexShader->GetType() == ShaderType::VERTEX), "No vertex shader submitted for shader reflection.");

		std::vector<std::vector<ReflectionData>> reflectionDataVectors;
		reflectionDataVectors.emplace_back(ReflectShader(pVertexShader));
		if (pPixelShader)
		{
			RLS_ASSERT(pPixelShader->GetType() == ShaderType::PIXEL, "Shader submitted is not of type pixel shader.");
			reflectionDataVectors.emplace_back(ReflectShader(pPixelShader));
		}

		ReflectionResult reflectionResult;

		//would have already been removed in every subsequent vectors... (quick thought)
		std::vector<D3D12_ROOT_PARAMETER> rootParameters;
		std::vector<ReflectionData> samplers;
		for (uint32_t i{ 0u }; i < reflectionDataVectors.size(); ++i)
		{
			for (uint32_t j{ 0u }; j < reflectionDataVectors[i].size(); ++j)
			{
				ReflectionData& reflectionData = reflectionDataVectors[i][j];
				if (reflectionData.shaderInputType == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER)
				{
					samplers.push_back(reflectionData);
					reflectionDataVectors[i].erase(reflectionDataVectors[i].begin() + j);
					j--;
					continue;
				}

				D3D12_ROOT_PARAMETER rootParameter;
				rootParameter.ParameterType = HLSLParameterTypeToD3D12RootParameterType(reflectionData.shaderInputType);
				if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
				{
					rootParameter.Constants.Num32BitValues = reflectionData.NrOfIntegers;
					rootParameter.Constants.ShaderRegister = reflectionData.Slot;
					rootParameter.Constants.RegisterSpace = reflectionData.Space;
				}
				else if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
				{
					rootParameter.Descriptor.ShaderRegister = reflectionData.Slot;
					rootParameter.Descriptor.RegisterSpace = reflectionData.Space;
				}
				bool sharedInMultipleShaders = false;
				for (uint32_t k = i + 1; k < reflectionDataVectors.size(); ++k)
				{
					for (int l = 0; l < reflectionDataVectors[k].size(); ++l)
					{
						if (reflectionDataVectors[k][l].Name == reflectionData.Name &&
							reflectionDataVectors[k][l].Slot == reflectionData.Slot &&
							reflectionDataVectors[k][l].Space == reflectionData.Space)
						{
							sharedInMultipleShaders = true;
							reflectionDataVectors[k].erase(reflectionDataVectors[k].begin() + l);
						}
					}
				}
				rootParameter.ShaderVisibility = sharedInMultipleShaders ? D3D12_SHADER_VISIBILITY_ALL : RLSShaderTypeToD3D12ShaderType(reflectionData.ShaderType);
				
				reflectionResult.RootSignatureSlotToReflectionDataMap[static_cast<uint32_t>(rootParameters.size())] = reflectionData;
				rootParameters.push_back(rootParameter);
			}
		}

		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplerDescriptors{};
		for (auto& data : samplers)
		{
			D3D12_STATIC_SAMPLER_DESC samplerDescriptor{};
			samplerDescriptor.Filter = DetermineFilterType(data.Name);
			samplerDescriptor.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			samplerDescriptor.ShaderRegister = data.Slot;
			samplerDescriptor.RegisterSpace = data.Space;
			samplerDescriptor.ShaderVisibility = RLSShaderTypeToD3D12ShaderType(data.ShaderType);
			samplerDescriptor.MaxLOD = D3D12_FLOAT32_MAX;
			samplerDescriptor.MinLOD = 0.0f;
			samplerDescriptor.MipLODBias = 0;
			if (samplerDescriptor.Filter == D3D12_FILTER_ANISOTROPIC)
			{
				samplerDescriptor.MaxAnisotropy = 16;
				samplerDescriptor.AddressU = samplerDescriptor.AddressV = samplerDescriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			}
			else if (samplerDescriptor.Filter == D3D12_FILTER_MIN_MAG_MIP_LINEAR)
			{
				samplerDescriptor.MaxAnisotropy = 0;
				if (data.Name == "sampler_LINEAR_CLAMP")
				{
					samplerDescriptor.AddressU = samplerDescriptor.AddressV = samplerDescriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				}
				else
				{
					samplerDescriptor.AddressU = samplerDescriptor.AddressV = samplerDescriptor.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				}

			}
			staticSamplerDescriptors.push_back(samplerDescriptor);
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
		rootSignatureDescriptor.NumParameters = static_cast<UINT>(rootParameters.size());
		rootSignatureDescriptor.pParameters = rootParameters.data();
		rootSignatureDescriptor.NumStaticSamplers = static_cast<UINT>(staticSamplerDescriptors.size());
		rootSignatureDescriptor.pStaticSamplers = staticSamplerDescriptors.data();
		rootSignatureDescriptor.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
			| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		if (!pPixelShader)
			rootSignatureDescriptor.Flags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

		Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob{ nullptr };
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{ nullptr };
		SERIALIZE_ROOT_SIGNATURE(rootSignatureDescriptor, pRootSignatureBlob);
		DXCall(D3D12Core::GetDevice()->CreateRootSignature
		(
			0u,
			pRootSignatureBlob->GetBufferPointer(),
			pRootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&reflectionResult.pRootSignature)
		));

		return reflectionResult;
	}

	std::vector<ReflectionData> ShaderReflector::ReflectShader(const std::shared_ptr<Shader>& pShader) noexcept
	{
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pShaderReflection{nullptr};

		Microsoft::WRL::ComPtr<IDxcContainerReflection> pReflection;
		DXCall(DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection)));

		// Associate the shader blob with the reflection interface
		DXCall(pReflection->Load(pShader->GetBuffer().Get()));

		// Find the index of the shader part
		UINT32 shaderIdx;
		DXCall(pReflection->FindFirstPartKind(DXC_FOURCC('D', 'X', 'I', 'L'), &shaderIdx));

		// Get the shader reflection interface
		DXCall(pReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&pShaderReflection)));

		// Now you can use pShaderReflection to inspect the shader
		D3D12_SHADER_DESC shaderDesc;
		pShaderReflection->GetDesc(&shaderDesc);

		// Loop through bound resources and print their names
		std::vector<ReflectionData> reflectionDatas;
		for (UINT i = 0; i < shaderDesc.BoundResources; i++) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			pShaderReflection->GetResourceBindingDesc(i, &bindDesc);
			
			ReflectionData reflectionData;
			reflectionData.Name = bindDesc.Name;
			reflectionData.Slot = bindDesc.BindPoint;
			reflectionData.Space = bindDesc.Space;
			reflectionData.ShaderType = pShader->GetType();
			reflectionData.shaderInputType = bindDesc.Type;
			reflectionData.NrOfIntegers = reflectionData.shaderInputType == D3D_SIT_CBUFFER ? (GetVariableSize(reflectionData.Name, pShaderReflection) / 4) : 0;

			reflectionDatas.push_back(reflectionData);
		}

		return reflectionDatas;
	}
}