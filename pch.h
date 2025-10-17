#pragma once

// Standard Libraries
#include <windows.h>
#include <wrl.h>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <cstdio>
#include <d3d11.h>
#include <d3d11_1.h>

// Included Libraries
#include "cereal.h"
#include "fbxsdk.h"

#ifdef _DEBUG
// Debug Libraries
#include "imgui/imgui.h"
#endif

// Personal Libraries
#include "render_lock.h"
#include "verify.h"
#include "high_resolution_timer.h"
#include "math.h"
#include "slider.h"
#include "string.h"
#include "device.h"
