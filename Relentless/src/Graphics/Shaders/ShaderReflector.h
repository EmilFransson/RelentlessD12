#pragma once
#include "Shader.h"
namespace Relentless
{
	struct ReflectionData
	{
		std::string Name;
		uint32_t Slot;
		uint32_t Space;
		uint32_t NrOfIntegers;
		D3D_SHADER_INPUT_TYPE shaderInputType;
		ShaderType ShaderType; 
	};

	struct ReflectionResult
	{
		Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature{ nullptr };
		std::unordered_map<uint32_t, ReflectionData> RootSignatureSlotToReflectionDataMap;
	};

	class ShaderReflector
	{
	public:
		ShaderReflector() noexcept = default;
		~ShaderReflector() noexcept = default;
		static ReflectionResult Reflect(const std::shared_ptr<Shader> & pVertexShader, const std::shared_ptr<Shader>& pPixelShader = nullptr) noexcept;
	private:
		[[nodiscard]] static std::vector<ReflectionData> ReflectShader(const std::shared_ptr<Shader>& pVertexShader) noexcept;
	};
}