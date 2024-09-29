#pragma once

#pragma warning(push, 0)
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
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <queue>

//Windows and D3D12:
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <comdef.h>
#include <commdlg.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <rpc.h>
#include <shlobj.h>
#include <Windows.h>
#include <Windowsx.h>
#include <wincodec.h>
#include <wrl/client.h>
#pragma warning(pop)

//Relentless-specific files:
#include "Core/Log.h"
#include "Core/Profiler.h"
#include "Core/Core.h"
#include "Core/Utility.h"
#include "Graphics/D3D12Debug.h"
#include "Math/CommonMath.h"
#include "Math/Vector3.h"

#include "../vendor/includes/directxtex/d3dx12.h"