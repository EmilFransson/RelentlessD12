#pragma once

#pragma warning(push, 0)
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
#include <memory>
#include <string>
#include <sstream>
#include <stdint.h>
#include <thread>
#include <future>
#include <mutex>
#include <span>
#include <filesystem>
#include <fstream>
#include <array>
#include <queue>
#include <chrono>
#include <tuple>
#include <queue>
#include <algorithm>
#include <execution>
#include <functional>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//Windows and D3D12:
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <commdlg.h>
#include <rpc.h>
#include <wrl/client.h>
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#pragma warning(pop)

//Relentless-specific files:
#include "Core/Log.h"
#include "Core/Profiler.h"
#include "Core/Core.h"
#include "Core/Utility.h"
#include "Graphics/D3D12Debug.h"