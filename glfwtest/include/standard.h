#pragma once

#define MULTITHREADED_RENDERING 1

#define MAX_BONES 64
#define MAX_BONES_PER_VERTEX 4

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <array>
#include <assert.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <vulkan\vulkan.h>

#include <json/json.hpp>
using json = nlohmann::json;

#include "helpers/DebugAssert.h"
#include "helpers/Utility.h"
#include "io/FileUtility.h"
