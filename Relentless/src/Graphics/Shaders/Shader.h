#pragma once
namespace Relentless
{
	enum class ShaderType : uint8_t { VERTEX, PIXEL };

	class Shader
	{
	public:
		Shader(const ShaderType shaderType, const std::string& name, Microsoft::WRL::ComPtr<IDxcBlob> pBuffer) noexcept;
		~Shader() noexcept = default;
		static std::shared_ptr<Shader> Create(const ShaderType type, const std::string& fileName) noexcept;
		[[nodiscard]] constexpr const ShaderType GetType() const noexcept { return m_Type; }
		[[nodiscard]] constexpr const Microsoft::WRL::ComPtr<IDxcBlob>& GetBuffer() const noexcept { return m_pBuffer; }
		[[nodiscard]] constexpr const std::string& GetName() const noexcept { return m_Name; }
	private:
		ShaderType m_Type;
		std::string m_Name;
		Microsoft::WRL::ComPtr<IDxcBlob> m_pBuffer;
	};
}