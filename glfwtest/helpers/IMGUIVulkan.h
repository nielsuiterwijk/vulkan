#pragma once

#include "standard.h"

#include "imgui/imgui.h"

#include "graphics/PipelineStateObject.h"

class CommandBuffer;
class TextureSampler;
class VulkanBuffer;


//Adopted from: https://github.com/ocornut/imgui/tree/master/examples/vulkan_example
class IMGUIVulkan
{
public:
	IMGUIVulkan();
	~IMGUIVulkan();

	bool Init(GLFWwindow* window, bool installCallbacks);
	void Shutdown();
	void NewFrame(float deltaTime);
	std::shared_ptr<CommandBuffer> Render(uint32_t frameIndex);

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
	TextureSampler* sampler;
	Texture2D* imguiFont;
	std::shared_ptr<Material> material2d;
	PipelineStateObject psoBasic2D;

	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer; 
	
	ImDrawVert* cpuVertex;// = new ImDrawVert[draw_data->TotalVtxCount];
	ImDrawIdx* cpuIndex;// = new ImDrawIdx[draw_data->TotalIdxCount];

	std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	std::vector<bool> mousePressed;
	std::vector<GLFWcursor*> mouseCursors;

	bool show_demo_window;
	

	GLFWwindow* window;

};