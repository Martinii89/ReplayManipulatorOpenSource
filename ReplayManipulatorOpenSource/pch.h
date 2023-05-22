#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "bakkesmod/plugin/bakkesmodplugin.h"
//#include "C:\Users\Martin\AppData\Roaming\bakkesmod\bakkesmod\bakkesmodsdk\include\bakkesmod/plugin/bakkesmodplugin.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>

#define IMGUI_USER_CONFIG "imgui_user_config.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_stdlib.h"
#include "IMGUI/imgui_searchablecombo.h"
#include "IMGUI/imgui_rangeslider.h"

#include "logging.h"

#include "nlohmann/json.hpp"
using namespace nlohmann;