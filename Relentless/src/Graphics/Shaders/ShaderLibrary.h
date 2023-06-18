#pragma once
#include "Shader.h"
namespace Relentless
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary() noexcept = default;
		~ShaderLibrary() noexcept = default;
		DELETE_COPY_MOVE_CONSTRUCTOR(ShaderLibrary);
		void Initialize() noexcept;
		void Add(const std::shared_ptr<Shader>& pShader) noexcept;
		[[nodiscard]] std::shared_ptr<Shader> Get(const std::string& shaderName) noexcept;
		[[nodiscard]] constexpr const std::unordered_map<std::string, std::shared_ptr<Shader>>& GetLibrary() const noexcept { return m_Shaders; }
	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> m_Shaders;
	};
}