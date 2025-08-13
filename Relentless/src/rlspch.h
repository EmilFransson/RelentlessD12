#pragma once

#pragma warning(push, 0)

#include "Core/MinWindows.h"
#include <Windowsx.h>
#include <dxgi1_6.h>

#include "../vendor/includes/D3D12/d3d12.h"
#include "../vendor/includes/D3D12/d3d12sdklayers.h"
#define D3DX12_NO_STATE_OBJECT_HELPERS
#include "../vendor/includes/D3D12/d3dx12.h"

#include "../vendor/includes/DirectXShaderCompiler/dxcapi.h"
#include "../vendor/includes/DirectXShaderCompiler/d3d12shader.h"

#include "../vendor/includes/FontAwesome/IconsFontAwesome6.h"

using ID3D12DeviceX = ID3D12Device14;
using ID3D12GraphicsCommandListX = ID3D12GraphicsCommandList10;
using ID3D12ResourceX = ID3D12Resource2;

using IDXGIFactoryX = IDXGIFactory7;
using IDXGISwapChainX = IDXGISwapChain4;

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <concepts>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <execution>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <span>
#include <stdint.h>
#include <sstream>
#include <stdlib.h>
#include <stack>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <queue>

#include <comdef.h>
#include <commdlg.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <rpc.h>
#include <shlobj.h>
#include <wincodec.h>
#include <wrl/client.h>
#pragma warning(pop)

//Relentless-specific files:
#include "Core/Log.h"
#include "Core/Profiler.h"
#include "Core/Core.h"
#include "Core/CoreTypes.h"
#include "Core/Utility.h"
#include "Core/Ref.h"
#include "Core/Span.h"
#include "Math/CommonMath.h"
#include "Math/MathTypes.h"