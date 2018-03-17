#pragma once


#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <vulkan\vulkan.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <chrono>