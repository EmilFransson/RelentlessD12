#include "ShaderLibrary.h"
#include "ShaderCompiler.h"
namespace Relentless
{
	/*Initialize sets up all basic shaders that ship with Relentless*/
	void ShaderLibrary::Initialize() noexcept
	{
		//Add(Shader::Create(ShaderType::VERTEX, "NewVertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "NewPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::Compute, "ToyComputeShader.hlsl"));

		Add(Shader::Create(ShaderType::VERTEX, "EditorGridShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "EditorGridShader.hlsl", "ps_main"));

		Add(Shader::Create(ShaderType::VERTEX, "EvolvingShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "EvolvingShader.hlsl", "ps_main", {"RED_OUTPUT"}));

		Add(Shader::Create(ShaderType::Compute, "PostProcessShader.hlsl", "cs_main"));

		//Add(Shader::Create(ShaderType::VERTEX, "EditorGridVertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "EditorGridPixelShader.hlsl"));

		//Add(Shader::Create(ShaderType::VERTEX, "VertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::VERTEX, "FullScreenTriVertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PostProcessPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PickingPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::VERTEX, "VertexShaderEditorGrid.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShaderEditorGrid.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShaderOrangeOutput.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "GeometryAndPickingPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PBRPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "CutOutPBRPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "TransparentPBRPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::VERTEX, "PickingVertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::VERTEX, "EquirectangularToCubemapVertexShader.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "EquirectangularToCubemapPixelShader.hlsl"));
		//Add(Shader::Create(ShaderType::VERTEX, "VertexShaderSkybox.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShaderSkybox.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShaderIrradianceConvolution.hlsl"));
		//Add(Shader::Create(ShaderType::PIXEL, "PixelShaderRadianceConvolution.hlsl"));
	}

	void ShaderLibrary::Add(const std::shared_ptr<Shader>& pShader) noexcept
	{
		RLS_ASSERT(pShader, "Shader pointer is invalid.");

		std::string shaderIdentifier = pShader->GetName() + pShader->GetEntryPoint();
		for (auto& define : pShader->GetDefines())
			shaderIdentifier += define;

		RLS_ASSERT(m_Shaders.find(shaderIdentifier) == m_Shaders.end(), "Shader already exists in shader library.");
		m_Shaders[shaderIdentifier] = pShader;
	}

	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& shaderName, const std::string& entryPoint, Span<std::string> defines) noexcept
	{
		std::string shaderIdentifier = shaderName + entryPoint;
		for (auto& define : defines)
			shaderIdentifier += define;

		RLS_ASSERT(m_Shaders.find(shaderIdentifier) != m_Shaders.end(), "Shader does not exist in shader library.");
		return m_Shaders[shaderIdentifier];
	}
}