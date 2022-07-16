#pragma once
namespace Relentless
{
	class D3D12Command
	{
	public:
		D3D12Command() noexcept;
		~D3D12Command() noexcept = default;
		void Initialize(const D3D12_COMMAND_LIST_TYPE commandType, const uint8_t nrOfBufferedFrames) noexcept;
	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList7> m_pCommandList;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_CommandAllocators;
		D3D12_COMMAND_LIST_TYPE m_CommandType;
	};
}