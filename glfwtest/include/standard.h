#pragma once

#define MULTITHREADED_RENDERING 1

#define MAX_BONES 64
#define MAX_BONES_PER_VERTEX 4

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <vulkan\vulkan.h>
#include <GLFW/glfw3.h>

#include <json/json.hpp>
using json = nlohmann::json;

#include <array>
#include <assert.h>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <string>
#include <thread>
#include <vector>

#include "helpers/Utility.h"