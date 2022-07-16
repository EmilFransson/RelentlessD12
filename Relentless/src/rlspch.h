#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <bitset>
#include <memory>
#include <string>
#include <sstream>
#include <stdint.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

//Windows and D3D12:
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <wrl/client.h>
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_6.h>

//Relentless-specific files:
#include "Relentless/Log.h"
#include "Relentless/Graphics/D3D12Debug.h"