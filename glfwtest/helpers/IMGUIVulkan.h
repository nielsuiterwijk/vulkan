#pragma once

#include "standard.h"

#include "imgui/imgui.h"

#include "graphics/PipelineStateObject.h"

class CommandBuffer;
class TextureSampler;
class VulkanBuffer;
class VertexShader;
class FragmentShader;
class Material;

//Adopted from: https://github.com/ocornut/imgui/tree/master/examples/vulkan_example
class IMGUIVulkan
{
private:
	struct ScaleTranslateUBO
	{
		glm::vec2 scale;
		glm::vec2 translate;
	};

public:
	IMGUIVulkan();
	~IMGUIVulkan();

	bool Init( GLFWwindow* window, bool installCallbacks );
	void Shutdown();
	void NewFrame( float deltaTime );
	void Render( std::shared_ptr<CommandBuffer> commandBuffer );

	bool IsReady() const { return !psoBasic2D.IsDirty(); }

private:
	void MouseButtonCallback( int button, int action, int mods );
	void ScrollCallback( double xoffset, double yoffset );
	void KeyCallback( int key, int scancode, int action, int mods );
	void CharCallback( unsigned int c );

	static const char* GetClipboardText( void* user_data )
	{
		return glfwGetClipboardString( (GLFWwindow*)user_data );
	}

	static void SetClipboardText( void* user_data, const char* text )
	{
		glfwSetClipboardString( (GLFWwindow*)user_data, text );
	}

private:
	TextureSampler* sampler;
	Texture2D* imguiFont;
	UniformBuffer* vulkanUbo;
	ScaleTranslateUBO ubo;

	std::shared_ptr<Material> material;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	PipelineStateObject psoBasic2D;

	VulkanBuffer* indexBuffer;
	VulkanBuffer* vertexBuffer;

	ImDrawVert* cpuVertex; // = new ImDrawVert[draw_data->TotalVtxCount];
	size_t sizeOfVertexBuffer;
	ImDrawIdx* cpuIndex; // = new ImDrawIdx[draw_data->TotalIdxCount];
	size_t sizeOfIndexBuffer;

	std::vector<bool> mousePressed;
	std::vector<GLFWcursor*> mouseCursors;

	bool showDemoWindow;

	bool didRender;

	GLFWwindow* window;

	VkVertexInputBindingDescription binding_desc = {};
	std::vector<VkVertexInputAttributeDescription> attribute_desc;
};
