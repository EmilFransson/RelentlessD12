#pragma once
namespace Relentless
{
	enum class ShaderType : uint8 { VERTEX = 0u, PIXEL, Compute, Max };

	class Shader
	{
	public:
		Shader(const ShaderType shaderType, const String& name, const char* pEntryPoint, const std::vector<String>& defines, Microsoft::WRL::ComPtr<IDxcBlob> pBuffer) noexcept;
		~Shader() noexcept = default;
		static std::shared_ptr<Shader> Create(const ShaderType type, const String& fileName, const char* pEntryPoint, Span<String> defines = {}) noexcept;
		NO_DISCARD ShaderType GetType() const noexcept { return m_Type; }
		NO_DISCARD const Microsoft::WRL::ComPtr<IDxcBlob>& GetBuffer() const noexcept { return m_pBuffer; }
		NO_DISCARD const String& GetEntryPoint() const noexcept { return m_EntryPoint; }
		NO_DISCARD const String& GetName() const noexcept { return m_Name; }
		NO_DISCARD const std::vector<String>& GetDefines() const noexcept { return m_Defines; }

	private:
		ShaderType m_Type;
		String m_Name;
		String m_EntryPoint;
		Microsoft::WRL::ComPtr<IDxcBlob> m_pBuffer;
		std::vector<String> m_Defines;
	};
}