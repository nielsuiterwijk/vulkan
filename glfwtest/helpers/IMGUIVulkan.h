#pragma once

#include "standard.h"

#include "graphics/PipelineStateObject.h"

class CommandBuffer;

//Adopted from: https://github.com/ocornut/imgui/tree/master/examples/vulkan_example
class IMGUIVulkan
{
public:
	IMGUIVulkan();
	~IMGUIVulkan();

	bool Init(GLFWwindow* window, bool installCallbacks);
	void Shutdown();
	void NewFrame();
	void Render();

private:
	void MouseButtonCallback(int button, int action, int mods);
	void ScrollCallback(double xoffset, double yoffset);
	void KeyCallback(int key, int scancode, int action, int mods);
	void CharCallback(unsigned int c);

	static const char* GetClipboardText(void* user_data)
	{
		return glfwGetClipboardString((GLFWwindow*)user_data);
	}

	static void SetClipboardText(void* user_data, const char* text)
	{
		glfwSetClipboardString((GLFWwindow*)user_data, text);
	}

private:
	std::shared_ptr<Material> material2d;
	PipelineStateObject psoBasic2D;

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	std::vector<bool> mousePressed;
	std::vector<GLFWcursor*> mouseCursors;

};