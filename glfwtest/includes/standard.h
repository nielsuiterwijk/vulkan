#pragma once

#define MULTITHREADED_RENDERING 1

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

#include <vulkan\vulkan.h>
#include <GLFW/glfw3.h>

#include <array>
#include <assert.h>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>