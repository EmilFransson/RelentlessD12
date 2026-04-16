#include "ShaderLibrary.h"
#include "ShaderCompiler.h"
namespace Relentless
{
	/*Initialize sets up all basic shaders that ship with Relentless*/
	void ShaderLibrary::Initialize() noexcept
	{
		Add(Shader::Create(ShaderType::VERTEX, "DepthPrePassShader.hlsl", "vs_main"));

		Add(Shader::Create(ShaderType::VERTEX, "EditorGridShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "EditorGridShader.hlsl", "ps_main"));

		Add(Shader::Create(ShaderType::VERTEX, "ForwardShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "ForwardShader.hlsl", "ps_main", {"RED_OUTPUT"}));

		Add(Shader::Create(ShaderType::VERTEX, "EntityOutputShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "EntityOutputShader.hlsl", "ps_main"));

		Add(Shader::Create(ShaderType::Compute, "PostProcessShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "GaussianBlurSeparableShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "DownsampleColorShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "AverageLuminanceShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "LuminanceHistogramShader.hlsl", "cs_main"));

		Add(Shader::Create(ShaderType::Compute, "EquirectangularToCubemapComputeShader.hlsl", "cs_main"));

		Add(Shader::Create(ShaderType::VERTEX, "SkyboxShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "SkyboxShader.hlsl", "ps_main"));
		Add(Shader::Create(ShaderType::PIXEL, "SkyboxShader.hlsl", "ps_main", {"BLEND_ENVIRONMENTS"}));

		Add(Shader::Create(ShaderType::VERTEX, "IrradianceConvolutionShader.hlsl", "vs_main"));
		Add(Shader::Create(ShaderType::PIXEL, "IrradianceConvolutionShader.hlsl", "ps_main"));
		Add(Shader::Create(ShaderType::Compute, "IrradianceConvolutionComputeShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "IrradianceConvolutionComputeShader.hlsl", "cs_main", { "LOWER_HEMISPHERE_SOLID_COLOR" }));

		Add(Shader::Create(ShaderType::Compute, "RadianceConvolutionShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "RadianceConvolutionShader.hlsl", "cs_main", { "LOWER_HEMISPHERE_SOLID_COLOR" }));

		Add(Shader::Create(ShaderType::Compute, "CubeMipGenerationShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "CubemapResampleShader.hlsl", "cs_main"));
		Add(Shader::Create(ShaderType::Compute, "CubeMapLowerHemisphereBlendShader.hlsl", "cs_main"));
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