#include "D3D12Command.h"
#include "D3D12Core.h"
namespace Relentless
{
	D3D12Command::D3D12Command() noexcept
		: m_pCommandList{nullptr}, 
		  m_pCommandQueue{nullptr},
		  m_CommandAllocators{nullptr},
		  m_CommandType{D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT}
	{
	}

	void D3D12Command::Initialize(const D3D12_COMMAND_LIST_TYPE commandType, const uint8_t nrOfBufferedFrames) noexcept
	{
		RLS_ASSERT(nrOfBufferedFrames > 0, "Number of buffered frames must exceed 0.");
		m_CommandType = commandType;

		D3D12_COMMAND_QUEUE_DESC commandQueueDescriptor{};
		commandQueueDescriptor.Type = m_CommandType;
		commandQueueDescriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDescriptor.NodeMask = 0u;
		DXCall(D3D12Core::GetDevice()->CreateCommandQueue(&commandQueueDescriptor, IID_PPV_ARGS(&m_pCommandQueue)));
		NAME_D12_OBJECT(m_pCommandQueue, L"Main Command Queue");

		if (m_CommandAllocators.size() > 0)
			m_CommandAllocators.clear();

		m_CommandAllocators.reserve(nrOfBufferedFrames);
		for (uint8_t i{ 0u }; i < nrOfBufferedFrames; ++i)
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator{ nullptr };
			DXCall(D3D12Core::GetDevice()->CreateCommandAllocator(m_CommandType, IID_PPV_ARGS(&pCommandAllocator)));
			m_CommandAllocators.emplace_back(std::move(pCommandAllocator));
			NAME_D12_OBJECT_INDEXED(m_CommandAllocators[i], L"Command Allocator", i);
		}

		DXCall(D3D12Core::GetDevice()->CreateCommandList(0u, m_CommandType, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_pCommandList)));
		NAME_D12_OBJECT(m_pCommandList, L"Main Command List");
	}
}