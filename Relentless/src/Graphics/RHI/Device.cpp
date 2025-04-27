#include "Device.h"

#include "Core/Application.h"
#include "CommandContext.h"
#include "Buffer.h"
#include "PipelineState.h"
#include "RingBufferAllocator.h"
#include "ResourceViews.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "RootSignature.h"
#include "ScratchAllocator.h"
#include "Texture.h"
#include "Graphics/Shaders/ShaderCompiler.h"

namespace Relentless
{
	DeviceObject::DeviceObject(GraphicsDevice* pParent) noexcept
		: m_pParent{pParent}
	{}

	GraphicsDevice* DeviceObject::GetParent() const noexcept
	{
		return m_pParent;
	}

	GraphicsDevice::GraphicsDevice(const GraphicsDeviceOptions& options) noexcept
		:
		DeviceObject{this}, 
		m_DeferredDeleteQueue{this}
	{
		uint32_t flags = 0u;
		if (options.UseDebugDevice)
			flags = DXGI_CREATE_FACTORY_DEBUG;

		VERIFY_HR(::CreateDXGIFactory2(flags, IID_PPV_ARGS(m_pFactory.GetAddressOf())));

		if (options.UseDebugDevice)
		{
			Ref<ID3D12Debug> pDebugController = nullptr;
			if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(pDebugController.GetAddressOf()))))
			{
				pDebugController->EnableDebugLayer();
				RLS_CORE_WARN("D3D12 Debug Layer Enabled.");
			}
		}

		if (options.UseGPUValidation)
		{
			Ref<ID3D12Debug1> pDebugController = nullptr;
			if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(pDebugController.GetAddressOf()))))
			{
				pDebugController->SetEnableGPUBasedValidation(true);
				RLS_CORE_WARN("D3D12 GPU Based Validation Enabled.");
			}
		}

		Ref<ID3D12Device> pDevice = nullptr;
		Ref<IDXGIAdapter4> pAdapter = nullptr;
		if (!options.UseWarp)
		{
			RLS_CORE_INFO("Detected Adapters:");

			uint32_t adapterIndex = 0u;
			DXGI_GPU_PREFERENCE gpuPreference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
			while (m_pFactory->EnumAdapterByGpuPreference(adapterIndex++, gpuPreference, IID_PPV_ARGS(pAdapter.ReleaseAndGetAddressOf())) == S_OK)
			{
				DXGI_ADAPTER_DESC3 adapterDesc{};
				pAdapter->GetDesc3(&adapterDesc);
				RLS_CORE_INFO("\t{0} - {1} GB", UNICODE_TO_MULTIBYTE(adapterDesc.Description), (float)adapterDesc.DedicatedVideoMemory * Math::BytesToGigaBytes);
			
				uint32_t outputIndex = 0u;
				Ref<IDXGIOutput> pOutput = nullptr;
				while (pAdapter->EnumOutputs(outputIndex++, pOutput.ReleaseAndGetAddressOf()) == S_OK)
				{
					Ref<IDXGIOutput6> pOutput6 = nullptr;
					if (pOutput.As<IDXGIOutput6>(&pOutput6))
					{
						DXGI_OUTPUT_DESC1 outputDesc{};
						VERIFY_HR(pOutput6->GetDesc1(&outputDesc));

						RLS_CORE_INFO("\t\tMonitor {0} - {1}x{2} - HDR: {3} - {4} BPP - Min Lum {5} - Max Lum {6} - MaxFFL {7}",
							outputIndex,
							outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left,
							outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top,
							outputDesc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 ? "Yes" : "No",
							outputDesc.BitsPerColor,
							outputDesc.MinLuminance,
							outputDesc.MaxLuminance,
							outputDesc.MaxFullFrameLuminance);
					}
				}
			}

			VERIFY_HR(m_pFactory->EnumAdapterByGpuPreference(0, gpuPreference, IID_PPV_ARGS(pAdapter.GetAddressOf())));
			DXGI_ADAPTER_DESC3 adapterDesc{};
			VERIFY_HR(pAdapter->GetDesc3(&adapterDesc));
			RLS_CORE_INFO("Using {0}", UNICODE_TO_MULTIBYTE(adapterDesc.Description));

			constexpr D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_12_2,
				D3D_FEATURE_LEVEL_12_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};

			VERIFY_HR(::D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(pDevice.GetAddressOf())));
			D3D12_FEATURE_DATA_FEATURE_LEVELS caps{};
			caps.pFeatureLevelsRequested = featureLevels;
			caps.NumFeatureLevels = ARRAYSIZE(featureLevels);
			VERIFY_HR(pDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &caps, sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS)));
			VERIFY_HR(::D3D12CreateDevice(pAdapter.Get(), caps.MaxSupportedFeatureLevel, IID_PPV_ARGS(pDevice.ReleaseAndGetAddressOf())));
		}

		if (!pDevice)
		{
			RLS_CORE_WARN("No D3D12 Adapter Found; Falling Back To WARP.");
			VERIFY_HR(m_pFactory->EnumWarpAdapter(IID_PPV_ARGS(pAdapter.GetAddressOf())));
		}

		VERIFY_HR(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf())));

		D3D::SetObjectName(m_pDevice.Get(), "Main Device");

		Ref<ID3D12InfoQueue> pInfoQueue = nullptr;
		if (SUCCEEDED(m_pDevice->QueryInterface(IID_PPV_ARGS(pInfoQueue.GetAddressOf()))))
		{
				VERIFY_HR_EX(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE), GetDevice());
				VERIFY_HR_EX(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, TRUE), GetDevice());
				VERIFY_HR_EX(pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, TRUE), GetDevice());
				RLS_CORE_WARN("D3D Validation Break On Severity Enabled.");

				Ref<ID3D12InfoQueue1> pInfoQueue1 = nullptr;
				if (pInfoQueue.As<ID3D12InfoQueue1>(&pInfoQueue1))
				{
					auto&& MessageCallback = [](
						D3D12_MESSAGE_CATEGORY category,
						D3D12_MESSAGE_SEVERITY severity,
						D3D12_MESSAGE_ID /*id*/,
						LPCSTR pDescription,
						void* /*pContext*/)
					{
						if (category == D3D12_MESSAGE_CATEGORY_STATE_CREATION)
							return;

						switch (severity)
						{
						case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION:
							RLS_CORE_CRITICAL("D3D Validation Layer: {0}", pDescription);
							break;
						case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR:
							RLS_CORE_ERROR("D3D Validation Layer: {0}", pDescription);
							break;
						case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING:
							RLS_CORE_WARN("D3D Validation Layer: {0}", pDescription);
							break;
						case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_INFO:
						case D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_MESSAGE:
							RLS_CORE_INFO("D3D Validation Layer: {0}", pDescription);
							break;
						}
					};

					DWORD cookie = 0;
					VERIFY_HR(pInfoQueue1->RegisterMessageCallback(MessageCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, this, &cookie));
				}
		}

		if (options.UseStablePowerState)
		{
			VERIFY_HR(m_pDevice->SetStablePowerState(TRUE));
			RLS_CORE_INFO("D3D12 Enabled Stable Power State.");
		}

		m_pAdapter = pAdapter;

		m_pFrameFence = new Fence(this, "Frame Fence");

		const uint64 scratchAllocatorPageSize = 256 * Math::KilobytesToBytes;
		m_pScratchAllocationManager = new ScratchAllocationManager(this, scratchAllocatorPageSize);

		const uint32 ringBufferSize = 512 * Math::MegaBytesToBytes;
		m_pRingBufferAllocator = new RingBufferAllocator(this, ringBufferSize);

		m_CommandQueues[D3D12_COMMAND_LIST_TYPE_DIRECT] = new CommandQueue(this, D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_CommandQueues[D3D12_COMMAND_LIST_TYPE_COMPUTE] = new CommandQueue(this, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		m_CommandQueues[D3D12_COMMAND_LIST_TYPE_COPY] = new CommandQueue(this, D3D12_COMMAND_LIST_TYPE_COPY);

		m_pDescriptorManager = std::make_unique<DescriptorManager>(this);
		
		ShaderCompiler::LoadDXC();
		m_pShaderLibrary = std::make_unique<ShaderLibrary>();
		m_pShaderLibrary->Initialize();

		m_pGlobalRootSignature = new RootSignature(this);
		m_pGlobalRootSignature->AddRootCBV(BindingSlot::PerInstance, 0);
		m_pGlobalRootSignature->AddRootCBV(BindingSlot::PerPass, 0);
		m_pGlobalRootSignature->AddRootCBV(BindingSlot::PerView, 0);
		m_pGlobalRootSignature->Finalize("Root Signature - Global");
	}

	GraphicsDevice::~GraphicsDevice() noexcept
	{
		IdleGPU();
	}

	CommandContext* GraphicsDevice::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type) noexcept
	{
		const int typeIndex = (int)type;
		CommandContext* pCommandContext = nullptr;
		{
			std::lock_guard guard(m_CommandContextAllocationMutex);
			if (!m_FreeCommandContexts[typeIndex].empty())
			{
				pCommandContext = m_FreeCommandContexts[typeIndex].front();
				m_FreeCommandContexts[typeIndex].pop();
			}
			else
			{
				Ref<ID3D12CommandList> pCommandList = nullptr;
				VERIFY_HR_EX(GetDevice()->CreateCommandList1(0u, type, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(pCommandList.GetAddressOf())), GetDevice());
				D3D::SetObjectName(pCommandList, std::format("Pooled {} Command List {}", D3D::CommandListTypeToString(type), m_CommandContextPool[typeIndex].size()).c_str());
				pCommandContext = m_CommandContextPool[typeIndex].emplace_back(RLS_NEW CommandContext(this, pCommandList, m_pScratchAllocationManager, type));
			}
		}
		pCommandContext->Reset();
		return pCommandContext;
	}

	DescriptorHandle GraphicsDevice::RegisterGlobalDescriptor(DescriptorHandleType descriptorHandleType) noexcept
	{
		return m_pDescriptorManager->CreateDescriptorHandle(descriptorHandleType);
	}

	std::vector<DescriptorHandle> GraphicsDevice::RegisterGlobalDescriptorBlock(DescriptorHandleType descriptorHandleType, uint32 blockSize) noexcept
	{
		return m_pDescriptorManager->CreateDescriptorHandleBlock(descriptorHandleType, blockSize);
	}

	Ref<Buffer> GraphicsDevice::CreateBuffer(const BufferDesc& desc, const char* pName, const void* pInitData /*= (const void*)nullptr*/) noexcept
	{
		auto&& GetResourceDesc = [](const BufferDesc& desc) -> D3D12_RESOURCE_DESC
		{
			D3D12_RESOURCE_DESC d3d12Desc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size);
			if (EnumHasAnyFlags(desc.Flags, BufferFlag::UnorderedAccess))
				d3d12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			return d3d12Desc;
		};

		const D3D12_RESOURCE_DESC resourceDesc = GetResourceDesc(desc);
		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_UNKNOWN;
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;

		if (EnumHasAnyFlags(desc.Flags, BufferFlag::Upload))
		{
			RLS_ASSERT(resourceState == D3D12_RESOURCE_STATE_UNKNOWN, "Current Resource State Is Invalid.");
			heapType = D3D12_HEAP_TYPE_UPLOAD;
			resourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		if (EnumHasAnyFlags(desc.Flags, BufferFlag::ReadBack))
		{
			RLS_ASSERT(resourceState == D3D12_RESOURCE_STATE_UNKNOWN, "Current Resource State Is Invalid.");
			heapType = D3D12_HEAP_TYPE_READBACK;
			resourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		}

		if (resourceState == D3D12_RESOURCE_STATE_UNKNOWN)
			resourceState = D3D12_RESOURCE_STATE_COMMON;

		const D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(heapType);

		ID3D12Resource2* pResource = nullptr;
		VERIFY_HR_EX(GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_CREATE_NOT_ZEROED, &resourceDesc, resourceState, nullptr, IID_PPV_ARGS(&pResource)), m_pDevice);
		D3D::SetObjectName(pResource, pName);

		Ref<Buffer> pBuffer = RLS_NEW Buffer(this, desc, pResource);
		pBuffer->SetName(pName);
		pBuffer->SetResourceState(resourceState);

		if (EnumHasAnyFlags(desc.Flags, BufferFlag::Upload | BufferFlag::ReadBack))
		{
			pBuffer->Map(0u, nullptr);
			pBuffer->SetStateTracking(true);
		}

		const bool isRaw = EnumHasAnyFlags(desc.Flags, BufferFlag::ByteAddress);

		if (EnumHasAnyFlags(desc.Flags, BufferFlag::ShaderResource))
		{
			pBuffer->SetSRV(CreateSRV(pBuffer, BufferSRVDesc(desc.Format, isRaw)));
		}

		if (pInitData)
		{
			if (EnumHasAllFlags(desc.Flags, BufferFlag::Upload))
			{
				memcpy((char*)pBuffer->GetMappedData(), pInitData, desc.Size);
			}
			else
			{
				RingBufferAllocation allocation;
				m_pRingBufferAllocator->Allocate((uint32)desc.Size, allocation);
				memcpy((char*)allocation.pMappedMemory, pInitData, desc.Size);
				allocation.pContext->CopyBuffer(allocation.pBackingResource, pBuffer, desc.Size, allocation.Offset, 0);
				m_pRingBufferAllocator->Free(allocation);
			}
		}

		return pBuffer;
	}

	Ref<Texture> GraphicsDevice::CreateTexture(const TextureDesc& desc, const char* pName, Span<D3D12_SUBRESOURCE_DATA> initData) noexcept
	{
		auto&& GetResourceDesc = [](const TextureDesc& textureDesc) -> D3D12_RESOURCE_DESC
		{
			const uint32 width = textureDesc.Width;
			const uint32 height = textureDesc.Height;
			const DXGI_FORMAT format = D3D::ConvertFormat(textureDesc.Format);

			D3D12_RESOURCE_DESC d3d12Desc;
			switch (textureDesc.Type)
			{
			case TextureType::Texture2D:
				d3d12Desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, (uint16)textureDesc.DepthOrArraySize, (uint16)textureDesc.Mips, textureDesc.SampleCount, 0u, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
				break;
			case TextureType::TextureCube:
				d3d12Desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, (uint16)textureDesc.DepthOrArraySize * 6, (uint16)textureDesc.Mips, textureDesc.SampleCount, 0u, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
				break;
			default:
				RLS_ASSERT(false, "[Device] Unreachable.");
				break;
			}

			if (EnumHasAnyFlags(textureDesc.Flags, TextureFlag::RenderTarget))
				d3d12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

			if (EnumHasAnyFlags(textureDesc.Flags, TextureFlag::DepthStencil))
			{
				d3d12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				if (!EnumHasAnyFlags(textureDesc.Flags, TextureFlag::ShaderResource))
					d3d12Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
			}

			if (EnumHasAnyFlags(textureDesc.Flags, TextureFlag::UnorderedAccess))
				d3d12Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			return d3d12Desc;
		};

		RLS_ASSERT(EnumHasAllFlags(desc.Flags, TextureFlag::RenderTarget | TextureFlag::DepthStencil) == false, "[Device] Invalid Texture Flags.");

		D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

		D3D12_CLEAR_VALUE* pClearValue = nullptr;
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = desc.Format == ResourceFormat::R32_TYPELESS ? DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT : D3D::ConvertFormat(desc.Format); //TODO, CHANGE!

		if (EnumHasAnyFlags(desc.Flags, TextureFlag::RenderTarget))
		{
			RLS_ASSERT(desc.ClearBindingValue.BindingValue == ClearBinding::ClearBindingValue::Color, "[Device] Invalid Clear Binding.");
			resourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			memcpy(&clearValue.Color, &desc.ClearBindingValue.Color, sizeof(Color));
			pClearValue = &clearValue;
		}
		if (EnumHasAnyFlags(desc.Flags, TextureFlag::DepthStencil))
		{
			RLS_ASSERT(desc.ClearBindingValue.BindingValue == ClearBinding::ClearBindingValue::DepthStencil, "[Device] Invalid Clear Binding.");
			resourceState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			clearValue.DepthStencil.Depth = desc.ClearBindingValue.DepthStencil.Depth;
			clearValue.DepthStencil.Stencil = desc.ClearBindingValue.DepthStencil.Stencil;
			pClearValue = &clearValue;
		}

		D3D12_RESOURCE_DESC resourceDesc = GetResourceDesc(desc);

		D3D12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ID3D12Resource2* pResource;
		VERIFY_HR_EX(GetDevice()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, resourceState, pClearValue, IID_PPV_ARGS(&pResource)), m_pDevice);
		D3D::SetObjectName(pResource, pName);

		Ref<Texture> pTexture = new Texture(this, desc, pResource);
		pTexture->SetName(pName);
		pTexture->SetResourceState(resourceState);

		if (initData.GetSize() > 0)
		{
			RLS_ASSERT(initData.GetSize() == desc.DepthOrArraySize * desc.Mips, "[GraphicsDevice::CreateTexture] Size Mismatch.");

			uint64 requiredSize = 0;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[16];
			uint32 numRows[16];
			uint64 rowSizes[16];
			m_pDevice->GetCopyableFootprints(&resourceDesc, 0, initData.GetSize(), 0, layouts, numRows, rowSizes, &requiredSize);
			RingBufferAllocation allocation;
			RLS_VERIFY(m_pRingBufferAllocator->Allocate((uint32)requiredSize, allocation), "FAILED TO ALLOCATE");

			for (uint32 subResource = 0; subResource < initData.GetSize(); ++subResource)
			{
				const D3D12_SUBRESOURCE_DATA& srcData = initData[subResource];
				D3D12_PLACED_SUBRESOURCE_FOOTPRINT& dstLayout = layouts[subResource];

				D3D12_MEMCPY_DEST dest =
				{
					.pData = (char*)allocation.pMappedMemory + dstLayout.Offset,
					.RowPitch = dstLayout.Footprint.RowPitch,
					.SlicePitch = (uint64)dstLayout.Footprint.RowPitch * numRows[subResource]
				};

				for (uint32 z = 0; z < dstLayout.Footprint.Depth; ++z)
				{
					char* pDest = (char*)dest.pData + dest.SlicePitch * z;
					const char* pSrc = (char*)srcData.pData + srcData.SlicePitch * z;
					for (uint32 y = 0; y < numRows[subResource]; ++y)
					{
						memcpy(pDest + y * dest.RowPitch, pSrc + y * srcData.RowPitch, rowSizes[subResource]);
					}
				}

				dstLayout.Offset += allocation.Offset;

				const CD3DX12_TEXTURE_COPY_LOCATION dst(pTexture->GetResource(), subResource);
				const CD3DX12_TEXTURE_COPY_LOCATION src(allocation.pBackingResource->GetResource(), dstLayout);
				allocation.pContext->GetCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
			}

			m_pRingBufferAllocator->Free(allocation);
		}

		if (EnumHasAnyFlags(desc.Flags, TextureFlag::ShaderResource))
			pTexture->SetSRV(CreateSRV(pTexture, TextureSRVDesc(0u, (uint8)pTexture->GetMipLevels())));
		if (EnumHasAnyFlags(desc.Flags, TextureFlag::UnorderedAccess))
		{
			pTexture->SetStateTracking(true);

			for (uint8 mip = 0; mip < desc.Mips; ++mip)
				pTexture->SetUAV(CreateUAV(pTexture, TextureUAVDesc(mip)), mip);
		}
		if (EnumHasAnyFlags(desc.Flags, TextureFlag::RenderTarget))
		{
			pTexture->SetStateTracking(true);
		}
		else if (EnumHasAnyFlags(desc.Flags, TextureFlag::DepthStencil))
		{
			pTexture->SetStateTracking(true);
		}

		return pTexture;
	}

	Ref<Texture> GraphicsDevice::CreateTextureForSwapchain(ID3D12ResourceX* pResource, uint32 index) noexcept
	{
		const D3D12_RESOURCE_DESC resourceDesc = pResource->GetDesc();
		const TextureDesc desc
		{
			.Width = (uint32)resourceDesc.Width,
			.Height = (uint32)resourceDesc.Height,
			.Mips = resourceDesc.MipLevels,
			.SampleCount = resourceDesc.SampleDesc.Count,
			.Flags = TextureFlag::RenderTarget,
			.Format = ResourceFormat::Unknown,
			.ClearBindingValue = ClearBinding(Colors::Magenta),
		};

		Ref<Texture> pTexture = RLS_NEW Texture(this, desc, pResource);
		pTexture->SetImmediateDelete(true);
		pTexture->SetName(std::format("Backbuffer {}", index).c_str());
		D3D::SetObjectName(pTexture->GetResource(), std::format("Backbuffer {}", index).c_str());
		pTexture->SetResourceState(D3D12_RESOURCE_STATE_PRESENT);
		pTexture->SetStateTracking(true);

		pTexture->SetSRV(CreateSRV(pTexture, TextureSRVDesc(0, 1)));
		pTexture->SetRTV(CreateRTV(pTexture, TextureRTVDesc()));
		return pTexture;
	}

	Ref<DepthStencilView> GraphicsDevice::CreateDSV(Texture* pTexture, const TextureDSVDesc& textureDSVDesc) noexcept
	{
		RLS_ASSERT(pTexture, "[GraphicsDevice] Texture Is Invalid.");
		const TextureDesc& desc = pTexture->GetDesc();

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = desc.Format == ResourceFormat::R32_TYPELESS ? DXGI_FORMAT_D32_FLOAT : D3D::ConvertFormat(desc.Format);
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		if (EnumHasAnyFlags(textureDSVDesc.Flags, DepthTargetAccessFlags::ReadOnlyDepth))
			dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;

		if (EnumHasAnyFlags(textureDSVDesc.Flags, DepthTargetAccessFlags::ReadOnlyStencil))
			dsvDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

		switch (desc.Type)
		{
		case TextureType::Texture2D:
			dsvDesc.ViewDimension = desc.SampleCount > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
			break;
		case TextureType::TextureCube:
			dsvDesc.Texture2DArray.ArraySize = textureDSVDesc.ArraySize;
			dsvDesc.Texture2DArray.FirstArraySlice = textureDSVDesc.FirstArraySlice;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			break;
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}
		
		dsvDesc.Texture2D.MipSlice = textureDSVDesc.MipSlice;
		dsvDesc.Texture2DArray.MipSlice = textureDSVDesc.MipSlice;

		DescriptorHandle descriptorHandle = RegisterGlobalDescriptor(DescriptorHandleType::DSV);
		GetDevice()->CreateDepthStencilView(pTexture->GetResource(), &dsvDesc, descriptorHandle.CPUHandle);

		return RLS_NEW DepthStencilView(this, descriptorHandle);
	}

	Ref<PipelineState> GraphicsDevice::CreatePipeline(const PipelineStateInitializer& pipelineStateInitializer) noexcept
	{
		Ref<PipelineState> pPipelineState = new PipelineState(this, pipelineStateInitializer);
		pPipelineState->CreateInternal();
		return pPipelineState;
	}

	Ref<PipelineState> GraphicsDevice::CreateComputePipeline(RootSignature* pRootSignature, const char* pShaderName, const char* pEntryPoint) noexcept
	{
		PipelineStateInitializer desc;
		desc.SetRootSignature(pRootSignature);
		desc.SetComputeShader(pShaderName, pEntryPoint);
		desc.SetName(std::format("Compute PSO: {0}", pShaderName).c_str());
		return CreatePipeline(desc);
	}

	Ref<ShaderResourceView> GraphicsDevice::CreateSRV(Buffer* pBuffer, const BufferSRVDesc& desc) noexcept
	{
		RLS_ASSERT(pBuffer, "[GraphicsDevice::CreateSRV] Buffer Is Invalid.");
		const BufferDesc& bufferDesc = pBuffer->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

		if (desc.Raw)
		{
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			srvDesc.Buffer.StructureByteStride = 0;
			srvDesc.Buffer.FirstElement = desc.ElementOffset / 4;
			srvDesc.Buffer.NumElements = desc.NumElements > 0 ? desc.NumElements / 4 : (uint32)(bufferDesc.Size / 4);
			srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
		}
		else
		{
			srvDesc.Format = D3D::ConvertFormat(desc.Format);
			srvDesc.Buffer.StructureByteStride = desc.Format == ResourceFormat::Unknown ? bufferDesc.ElementSize : 0;
			srvDesc.Buffer.FirstElement = desc.ElementOffset;
			srvDesc.Buffer.NumElements = desc.NumElements > 0 ? desc.NumElements : bufferDesc.NumElements();
		}

		const DescriptorHandle descriptorHandle = RegisterGlobalDescriptor(DescriptorHandleType::SRV);

		m_pDevice->CreateShaderResourceView(pBuffer->GetResource(), &srvDesc, descriptorHandle.CPUHandle);

		return RLS_NEW ShaderResourceView(this, descriptorHandle);
	}

	Ref<ShaderResourceView> GraphicsDevice::CreateSRV(Texture* pTexture, const TextureSRVDesc& textureSRVDesc) noexcept
	{
		const TextureDesc& desc = pTexture->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = desc.Format == ResourceFormat::R32_TYPELESS ? DXGI_FORMAT_R32_FLOAT : D3D::ConvertFormat(desc.Format);
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (desc.Type)
		{
		case TextureType::Texture2D:
		{
			srvDesc.ViewDimension = desc.SampleCount > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = textureSRVDesc.NumMipLevels;
			srvDesc.Texture2D.MostDetailedMip = textureSRVDesc.MipLevel;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0u;
			break;
		}
		case TextureType::TextureCube:
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		}
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}

		DescriptorHandle descriptorHandle = RegisterGlobalDescriptor(DescriptorHandleType::SRV);
		GetDevice()->CreateShaderResourceView(pTexture->GetResource(), &srvDesc, descriptorHandle.CPUHandle);
		
		return RLS_NEW ShaderResourceView(this, descriptorHandle);
	}

	Ref<UnorderedAccessView> GraphicsDevice::CreateUAV(Texture* pTexture, const TextureUAVDesc& desc) noexcept
	{
		RLS_ASSERT(pTexture, "[GraphicsDevice::CreateUAV] Texture Is Invalid.");
		const TextureDesc& textureDesc = pTexture->GetDesc();

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		switch (textureDesc.Type)
		{
		case TextureType::Texture2D:
			uavDesc.Texture2D.MipSlice = desc.MipLevel;
			uavDesc.Texture2D.PlaneSlice = 0;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			break;
		case TextureType::TextureCube:
			uavDesc.Texture2DArray.ArraySize = textureDesc.DepthOrArraySize * 6;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.PlaneSlice = 0;
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			break;
		default:
			break;
		}
		uavDesc.Format = D3D::ConvertFormat(pTexture->GetFormat());

		const DescriptorHandle descriptorHandle = RegisterGlobalDescriptor(DescriptorHandleType::UAV);
		GetDevice()->CreateUnorderedAccessView(pTexture->GetResource(), nullptr, &uavDesc, descriptorHandle.CPUHandle);

		return RLS_NEW UnorderedAccessView(this, descriptorHandle);
	}

	Ref<RenderTargetView> GraphicsDevice::CreateRTV(Texture* pTexture, const TextureRTVDesc& textureRTVDesc) noexcept
	{
		RLS_ASSERT(pTexture, "[GraphicsDevice] Texture Is Invalid.");
		const TextureDesc& textureDesc = pTexture->GetDesc();

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		switch (textureDesc.Type)
		{
		case TextureType::Texture2D:
			rtvDesc.Texture2D.PlaneSlice = textureRTVDesc.PlaneSlice;
			rtvDesc.ViewDimension = textureDesc.SampleCount > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;
			break;
		case TextureType::TextureCube:
			rtvDesc.Texture2DArray.ArraySize = textureRTVDesc.ArraySize;
			rtvDesc.Texture2DArray.FirstArraySlice = textureRTVDesc.FirstArraySlice;
			rtvDesc.Texture2DArray.PlaneSlice = textureRTVDesc.PlaneSlice;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			break;
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}
		rtvDesc.Texture1D.MipSlice = textureRTVDesc.MipSlice;
		rtvDesc.Texture1DArray.MipSlice = textureRTVDesc.MipSlice;
		rtvDesc.Texture2D.MipSlice = textureRTVDesc.MipSlice;
		rtvDesc.Texture2DArray.MipSlice = textureRTVDesc.MipSlice;
		rtvDesc.Texture3D.MipSlice = textureRTVDesc.MipSlice;
		rtvDesc.Format = D3D::ConvertFormat(pTexture->GetFormat());
		
		const DescriptorHandle descriptorHandle = RegisterGlobalDescriptor(DescriptorHandleType::RTV);
		GetDevice()->CreateRenderTargetView(pTexture->GetResource(), &rtvDesc, descriptorHandle.CPUHandle);

		return RLS_NEW RenderTargetView(this, descriptorHandle);
	}

	void GraphicsDevice::DeferReleaseObject(ID3D12Object* pResource) noexcept
	{
		RLS_ASSERT(pResource, "[GraphicsDevice] Resource is invalid.");

		m_DeferredDeleteQueue.EnqueueResource(pResource, SyncPoint(m_pFrameFence.Get(), m_pFrameFence->GetCurrentValue()));
	}

	void GraphicsDevice::FreeCommandContext(CommandContext* pCommandContext) noexcept
	{
		std::lock_guard guard(m_CommandContextAllocationMutex);
		m_FreeCommandContexts[(int)pCommandContext->GetType()].push(pCommandContext);
	}

	CommandQueue* GraphicsDevice::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept
	{
		return m_CommandQueues[type];
	}

	ID3D12DeviceX* GraphicsDevice::GetDevice() const noexcept
	{
		return m_pDevice.Get();
	}

	IDXGIFactoryX* GraphicsDevice::GetFactory() const noexcept
	{
		return m_pFactory;
	}

	Fence* GraphicsDevice::GetFrameFence() const noexcept
	{
		return m_pFrameFence;
	}

	DescriptorHeap* GraphicsDevice::GetGlobalShaderBindableHeap() const noexcept
	{
		return m_pDescriptorManager->GetShaderBindableDescriptorHeap();
	}

	DescriptorHeap* GraphicsDevice::GetGlobalSamplerHeap() const noexcept
	{
		return m_pDescriptorManager->GetSamplerDescriptorHeap();
	}

	RootSignature* GraphicsDevice::GetGlobalRootSignature() const noexcept
	{
		return m_pGlobalRootSignature;
	}

	DescriptorHeap* GraphicsDevice::GetRenderTargetViewDescriptorHeap() const noexcept
	{
		return m_pDescriptorManager->GetRTVDescriptorHeap();
	}

	RingBufferAllocator* GraphicsDevice::GetRingBuffer() const noexcept
	{
		return m_pRingBufferAllocator;
	}

	ShaderLibrary* GraphicsDevice::GetShaderLibrary() const noexcept
	{
		return m_pShaderLibrary.get();
	}

	void GraphicsDevice::IdleGPU() noexcept
	{
		TickFrame();
		m_pFrameFence->CPUWait(m_pFrameFence->GetLastSignaledValue());

		for (auto& pCommandQueue : m_CommandQueues)
		{
			if (pCommandQueue)
				pCommandQueue->WaitForIdle();
		}
	}

	void GraphicsDevice::TickFrame() noexcept
	{
		m_DeferredDeleteQueue.Clean();

		const uint64_t fenceValue = m_pFrameFence->Signal(GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
		
		m_FrameFenceValues[fenceValue % NUM_BUFFERS] = fenceValue;
		++m_FrameIndex;
		m_pFrameFence->CPUWait(m_FrameFenceValues[m_FrameIndex % NUM_BUFFERS]);
	}

	void GraphicsDevice::UnregisterGlobalDescriptor(const DescriptorHandle& descriptorHandle) noexcept
	{
		m_pDescriptorManager->DeferReleaseDescriptorHandle(descriptorHandle, SyncPoint(m_pFrameFence.Get(), m_pFrameFence->GetCurrentValue()));
	}

	GraphicsDevice::DeferredDeleteQueue::DeferredDeleteQueue(GraphicsDevice* pParent) noexcept
		: DeviceObject(pParent)
	{
	}

	GraphicsDevice::DeferredDeleteQueue::~DeferredDeleteQueue() noexcept
	{
		GetParent()->IdleGPU();
		Clean();
		RLS_ASSERT(m_DeletionQueue.empty(), "[DeferredDeleteQueue] Queue was not emptied.");
	}

	void GraphicsDevice::DeferredDeleteQueue::Clean() noexcept
	{
		std::lock_guard guard(m_QueueMutex);

		while (!m_DeletionQueue.empty())
		{
			FencedObject& fencedObject = m_DeletionQueue.front();
			if (!fencedObject.Sync.IsComplete())
				break;

			RLS_VERIFY(fencedObject.pResource->Release() == 0, "RELEASE FAILED");
			fencedObject.pResource = nullptr;
			m_DeletionQueue.pop();
		}
	}

	void GraphicsDevice::DeferredDeleteQueue::EnqueueResource(ID3D12Object* pResource, const SyncPoint& syncPoint) noexcept
	{
		std::lock_guard guard(m_QueueMutex);

		FencedObject fencedObject
		{
			.pResource = pResource,
			.Sync = syncPoint
		};

		m_DeletionQueue.push(fencedObject);
	}

}
