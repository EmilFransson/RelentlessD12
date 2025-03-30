#pragma once
namespace Relentless
{
	enum class ShaderType : uint8_t { VERTEX = 0u, PIXEL, Compute, Max };

	class Shader
	{
	public:
		Shader(const ShaderType shaderType, const std::string& name, const char* pEntryPoint, const std::vector<std::string>& defines, Microsoft::WRL::ComPtr<IDxcBlob> pBuffer) noexcept;
		~Shader() noexcept = default;
		static std::shared_ptr<Shader> Create(const ShaderType type, const std::string& fileName, const char* pEntryPoint, Span<std::string> defines = {}) noexcept;
		[[nodiscard]] const ShaderType GetType() const noexcept { return m_Type; }
		[[nodiscard]] const Microsoft::WRL::ComPtr<IDxcBlob>& GetBuffer() const noexcept { return m_pBuffer; }
		[[nodiscard]] const std::string& GetEntryPoint() const noexcept { return m_EntryPoint; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::vector<std::string>& GetDefines() const noexcept { return m_Defines; }

	private:
		ShaderType m_Type;
		std::string m_Name;
		std::string m_EntryPoint;
		Microsoft::WRL::ComPtr<IDxcBlob> m_pBuffer;
		std::vector<std::string> m_Defines;
	};
}