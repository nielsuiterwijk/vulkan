#include "IMGUIVulkan.h"

#include "graphics/buffers/CommandBuffer.h"
#include "graphics/buffers/UniformBuffer.h"
#include "graphics/buffers/VulkanBuffer.h"
#include "graphics/shaders/FragmentShader.h"
#include "graphics/shaders/ShaderCache.h"
#include "graphics/shaders/VertexShader.h"
#include "graphics/shaders/material.h"
#include "graphics/textures/TextureSampler.h"
#include "io/InputEvent.h"

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

IMGUIVulkan::IMGUIVulkan( GLFWwindow* pWindow ) :
	psoBasic2D(),
	_MousePressed(),
	mouseCursors(),
	cpuVertex( nullptr ),
	cpuIndex( nullptr ),
	showDemoWindow( true ),
	didRender( true ),
	sampler( nullptr ),
	imguiFont( nullptr ),
	vulkanUbo( nullptr ),
	_pWindow( pWindow ),
	sizeOfVertexBuffer( 0 ),
	sizeOfIndexBuffer( 0 ), 
	indexBuffer( nullptr ),
	vertexBuffer( nullptr )

{
}

IMGUIVulkan::~IMGUIVulkan()
{
	Shutdown();
}

bool IMGUIVulkan::Initialize(  )
{
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)

	io.KeyMap[ ImGuiKey_Tab ] = GLFW_KEY_TAB;
	io.KeyMap[ ImGuiKey_LeftArrow ] = GLFW_KEY_LEFT;
	io.KeyMap[ ImGuiKey_RightArrow ] = GLFW_KEY_RIGHT;
	io.KeyMap[ ImGuiKey_UpArrow ] = GLFW_KEY_UP;
	io.KeyMap[ ImGuiKey_DownArrow ] = GLFW_KEY_DOWN;
	io.KeyMap[ ImGuiKey_PageUp ] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ ImGuiKey_PageDown ] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ ImGuiKey_Home ] = GLFW_KEY_HOME;
	io.KeyMap[ ImGuiKey_End ] = GLFW_KEY_END;
	io.KeyMap[ ImGuiKey_Insert ] = GLFW_KEY_INSERT;
	io.KeyMap[ ImGuiKey_Delete ] = GLFW_KEY_DELETE;
	io.KeyMap[ ImGuiKey_Backspace ] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ ImGuiKey_Space ] = GLFW_KEY_SPACE;
	io.KeyMap[ ImGuiKey_Enter ] = GLFW_KEY_ENTER;
	io.KeyMap[ ImGuiKey_Escape ] = GLFW_KEY_ESCAPE;
	io.KeyMap[ ImGuiKey_A ] = GLFW_KEY_A;
	io.KeyMap[ ImGuiKey_C ] = GLFW_KEY_C;
	io.KeyMap[ ImGuiKey_V ] = GLFW_KEY_V;
	io.KeyMap[ ImGuiKey_X ] = GLFW_KEY_X;
	io.KeyMap[ ImGuiKey_Y ] = GLFW_KEY_Y;
	io.KeyMap[ ImGuiKey_Z ] = GLFW_KEY_Z;

	io.SetClipboardTextFn = IMGUIVulkan::SetClipboardText;
	io.GetClipboardTextFn = IMGUIVulkan::GetClipboardText;
	io.ClipboardUserData = _pWindow;
#ifdef _WIN32
	io.ImeWindowHandle = glfwGetWin32Window( _pWindow );
#endif

	// Load cursors
	// FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert to arrow cursor for those.
	mouseCursors.resize( ImGuiMouseCursor_COUNT );

	mouseCursors[ ImGuiMouseCursor_Arrow ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
	mouseCursors[ ImGuiMouseCursor_TextInput ] = glfwCreateStandardCursor( GLFW_IBEAM_CURSOR );
	mouseCursors[ ImGuiMouseCursor_ResizeAll ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
	mouseCursors[ ImGuiMouseCursor_ResizeNS ] = glfwCreateStandardCursor( GLFW_VRESIZE_CURSOR );
	mouseCursors[ ImGuiMouseCursor_ResizeEW ] = glfwCreateStandardCursor( GLFW_HRESIZE_CURSOR );
	mouseCursors[ ImGuiMouseCursor_ResizeNESW ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
	mouseCursors[ ImGuiMouseCursor_ResizeNWSE ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );

	{
		using namespace std::placeholders;

		InputEvent::OnMouseButton.push_back( std::bind( &IMGUIVulkan::MouseButtonCallback, this, _1, _2, _3 ) );
		InputEvent::OnMouseScroll.push_back( std::bind( &IMGUIVulkan::ScrollCallback, this, _1, _2 ) );
		InputEvent::OnKey.push_back( std::bind( &IMGUIVulkan::KeyCallback, this, _1, _2, _3, _4 ) );
		InputEvent::OnChar.push_back( std::bind( &IMGUIVulkan::CharCallback, this, _1 ) );
	}

	material = std::make_shared<Material>( "imgui" );
	material->AddUniformBuffer( new UniformBuffer( { static_cast<void*>( &ubo ), sizeof( ScaleTranslateUBO ) } ) );

	binding_desc = {};
	binding_desc.stride = sizeof( ImDrawVert );
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attribute_desc.resize( 3 );
	attribute_desc[ 0 ].location = 0;
	attribute_desc[ 0 ].binding = binding_desc.binding;
	attribute_desc[ 0 ].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[ 0 ].offset = ( size_t )( &( (ImDrawVert*)0 )->pos );

	attribute_desc[ 1 ].location = 1;
	attribute_desc[ 1 ].binding = binding_desc.binding;
	attribute_desc[ 1 ].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[ 1 ].offset = ( size_t )( &( (ImDrawVert*)0 )->uv );

	attribute_desc[ 2 ].location = 2;
	attribute_desc[ 2 ].binding = binding_desc.binding;
	attribute_desc[ 2 ].format = VK_FORMAT_R8G8B8A8_UNORM;
	attribute_desc[ 2 ].offset = ( size_t )( &( (ImDrawVert*)0 )->col );

	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height );
	size_t upload_size = width * height * 4 * sizeof( char );

	//VulkanBuffer buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, BufferType::Staging, pixels, imageSize);
	VulkanBuffer buffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT, BufferType::Staging, pixels, upload_size );

	imguiFont = new Texture2D();
	imguiFont->AllocateImage( width, height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, ( VkImageUsageFlagBits )( VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT ), VMA_MEMORY_USAGE_GPU_ONLY );
	imguiFont->Transition( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ); //Setup image data so it can be transfered to.
	buffer.CopyToImageAndClear( imguiFont->GetImage(), width, height );
	imguiFont->Transition( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ); //move data to shader readable
	imguiFont->SetupView( VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

	io.Fonts->TexID = (void*)imguiFont->GetImage();

	sampler = new TextureSampler();
	sampler->Initialize( VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0 );

	return true;
}

void IMGUIVulkan::Shutdown()
{
	delete imguiFont;
	imguiFont = nullptr;

	delete sampler;
	sampler = nullptr;

	material = nullptr;

	delete indexBuffer;
	delete vertexBuffer;
	delete[] cpuVertex;
	delete[] cpuIndex;
}

void IMGUIVulkan::NewFrame( float deltaTime )
{
	if ( !didRender )
		return;

	if ( material->IsLoaded() && psoBasic2D.IsDirty() )
	{
		assert( shaderStages.size() == 0 );
		shaderStages.push_back( material->GetVertex()->GetShaderStageCreateInfo() );
		shaderStages.push_back( material->GetFragment()->GetShaderStageCreateInfo() );

		std::vector<VkDynamicState> states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		psoBasic2D.Create( material, states, false );
		psoBasic2D.SetShader( shaderStages );
		psoBasic2D.SetVertexLayout( binding_desc, attribute_desc );
		psoBasic2D.Build( material );
	}
	else if ( psoBasic2D.IsDirty() )
	{
		return;
	}

	//	ImGui::EndFrame();
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	int w, h;
	int display_w, display_h;
	glfwGetWindowSize( _pWindow, &w, &h );
	glfwGetFramebufferSize( _pWindow, &display_w, &display_h );
	io.DisplaySize = ImVec2( (float)w, (float)h );
	io.DisplayFramebufferScale = ImVec2( w > 0 ? ( (float)display_w / w ) : 0, h > 0 ? ( (float)display_h / h ) : 0 );

	// Setup time step
	io.DeltaTime = deltaTime;

	// Setup inputs
	// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
	if ( glfwGetWindowAttrib( _pWindow, GLFW_FOCUSED ) )
	{
		double mouse_x, mouse_y;
		glfwGetCursorPos( _pWindow, &mouse_x, &mouse_y );
		io.MousePos = ImVec2( (float)mouse_x, (float)mouse_y );
	}
	else
	{
		io.MousePos = ImVec2( -FLT_MAX, -FLT_MAX );
	}

	for ( int i = 0; i < _MousePressed.size(); i++ )
	{
		io.MouseDown[ i ] = _MousePressed[ i ];// || glfwGetMouseButton( window, i ) != 0; // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		//_MousePressed[ i ] = false;
	}

	// Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
	if ( ( io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange ) == 0 )
	{
		ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
		if ( io.MouseDrawCursor || cursor == ImGuiMouseCursor_None )
		{
			glfwSetInputMode( _pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
		}
		else
		{
			glfwSetCursor( _pWindow, mouseCursors[ cursor ] ? mouseCursors[ cursor ] : mouseCursors[ ImGuiMouseCursor_Arrow ] );
			glfwSetInputMode( _pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		}
	}

	ImGui::NewFrame();

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	if ( showDemoWindow )
	{
		//ImGui::ShowDemoWindow(&showDemoWindow);
	}

	didRender = false;
}

void IMGUIVulkan::Render(CommandBuffer* commandBuffer )
{
	if ( didRender )
		return;

	ImGui::Render();
	didRender = true;

	ImDrawData* draw_data = ImGui::GetDrawData();

	ImGuiIO& io = ImGui::GetIO();
	if ( draw_data->TotalVtxCount == 0 )
		return;

	// Create the Vertex Buffer:
	size_t VertexBufferSize = draw_data->TotalVtxCount * sizeof( ImDrawVert );
	if ( vertexBuffer == nullptr || vertexBuffer->GetSize() < VertexBufferSize )
	{
		delete vertexBuffer;

		constexpr int MemoryAlignment = 256;
		VkDeviceSize AlignedVertexBufferSize = ( ( VertexBufferSize - 1 ) / MemoryAlignment + 1 ) * MemoryAlignment;
		
		vertexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferType::Dynamic, nullptr, AlignedVertexBufferSize );
		assert( vertexBuffer != nullptr );
	}

	if ( sizeOfVertexBuffer < draw_data->TotalVtxCount )
	{
		delete cpuVertex;
		sizeOfVertexBuffer = draw_data->TotalVtxCount;
		cpuVertex = new ImDrawVert[ sizeOfVertexBuffer ];
		assert( cpuVertex != nullptr );
	}

	// Create the Index Buffer:
	size_t index_size = draw_data->TotalIdxCount * sizeof( ImDrawIdx );
	if ( indexBuffer == nullptr || indexBuffer->GetSize() < index_size )
	{
		delete indexBuffer;

		constexpr int memoryAlignment = 256;
		VkDeviceSize index_buffer_size = ( ( index_size - 1 ) / memoryAlignment + 1 ) * memoryAlignment;

		size_t dif = index_buffer_size - index_size;

		indexBuffer = new VulkanBuffer( VK_BUFFER_USAGE_INDEX_BUFFER_BIT, BufferType::Dynamic, nullptr, index_buffer_size );
		assert( indexBuffer != nullptr );
	}

	if ( sizeOfIndexBuffer < draw_data->TotalIdxCount )
	{
		delete cpuIndex;
		sizeOfIndexBuffer = draw_data->TotalIdxCount;
		cpuIndex = new ImDrawIdx[ sizeOfIndexBuffer ];
		assert( cpuIndex != nullptr );
	}

	// Upload Vertex and index Data:
	{
		ImDrawVert* vtx_dst = cpuVertex;
		ImDrawIdx* idx_dst = cpuIndex;

		for ( int n = 0; n < draw_data->CmdListsCount; n++ )
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[ n ];
			memcpy( vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof( ImDrawVert ) );
			memcpy( idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof( ImDrawIdx ) );
			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}

		vertexBuffer->Map( (void*)cpuVertex, VertexBufferSize );
		indexBuffer->Map( (void*)cpuIndex, index_size );
	}

	// Bind pipeline and descriptor sets:
	{
		vkCmdBindPipeline( commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, psoBasic2D.GetPipeLine() );
		VkDescriptorSet set = material->AccessDescriptorPool().RetrieveDescriptorSet( material, imguiFont, sampler );
		vkCmdBindDescriptorSets( commandBuffer->GetNative(), VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetDescriptorPool().GetPipelineLayout(), 0, 1, &set, 0, NULL );
	}

	// Bind Vertex And Index Buffer:
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers( commandBuffer->GetNative(), 0, 1, &vertexBuffer->GetNative(), offsets );
		vkCmdBindIndexBuffer( commandBuffer->GetNative(), indexBuffer->GetNative(), 0, VK_INDEX_TYPE_UINT16 );
	}

	// Setup viewport:
	{
		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = ImGui::GetIO().DisplaySize.x;
		viewport.height = ImGui::GetIO().DisplaySize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport( commandBuffer->GetNative(), 0, 1, &viewport );
	}

	// Setup scale and translation:
	{
		ubo.scale.x = 2.0f / io.DisplaySize.x;
		ubo.scale.y = 2.0f / io.DisplaySize.y;
		ubo.translate.x = -1.0f;
		ubo.translate.y = -1.0f;

		material->GetUniformBuffers()[ 0 ]->Upload();
	}

	// Render the command lists:
	int vtx_offset = 0;
	int idx_offset = 0;
	for ( int n = 0; n < draw_data->CmdListsCount; n++ )
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[ n ];
		for ( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ )
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ cmd_i ];
			if ( pcmd->UserCallback )
			{
				pcmd->UserCallback( cmd_list, pcmd );
			}
			else
			{
				VkRect2D scissor;
				scissor.offset.x = ( int32_t )( pcmd->ClipRect.x ) > 0 ? ( int32_t )( pcmd->ClipRect.x ) : 0;
				scissor.offset.y = ( int32_t )( pcmd->ClipRect.y ) > 0 ? ( int32_t )( pcmd->ClipRect.y ) : 0;
				scissor.extent.width = ( uint32_t )( pcmd->ClipRect.z - pcmd->ClipRect.x );
				scissor.extent.height = ( uint32_t )( pcmd->ClipRect.w - pcmd->ClipRect.y + 1 ); // FIXME: Why +1 here?
				vkCmdSetScissor( commandBuffer->GetNative(), 0, 1, &scissor );
				vkCmdDrawIndexed( commandBuffer->GetNative(), pcmd->ElemCount, 1, idx_offset, vtx_offset, 0 );
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

void IMGUIVulkan::MouseButtonCallback( int button, int action, int mods )
{
	if ( button >= 0 && button < _MousePressed.size() )
		_MousePressed[ button ] = action == GLFW_PRESS;
}

void IMGUIVulkan::ScrollCallback( double xoffset, double yoffset )
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void IMGUIVulkan::KeyCallback( int key, int scancode, int action, int mods )
{
	ImGuiIO& io = ImGui::GetIO();
	if ( action == GLFW_PRESS )
		io.KeysDown[ key ] = true;
	if ( action == GLFW_RELEASE )
		io.KeysDown[ key ] = false;

	(void)mods; // Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[ GLFW_KEY_LEFT_CONTROL ] || io.KeysDown[ GLFW_KEY_RIGHT_CONTROL ];
	io.KeyShift = io.KeysDown[ GLFW_KEY_LEFT_SHIFT ] || io.KeysDown[ GLFW_KEY_RIGHT_SHIFT ];
	io.KeyAlt = io.KeysDown[ GLFW_KEY_LEFT_ALT ] || io.KeysDown[ GLFW_KEY_RIGHT_ALT ];
	io.KeySuper = io.KeysDown[ GLFW_KEY_LEFT_SUPER ] || io.KeysDown[ GLFW_KEY_RIGHT_SUPER ];
}

void IMGUIVulkan::CharCallback( unsigned int c )
{
	ImGuiIO& io = ImGui::GetIO();
	if ( c > 0 && c < 0x10000 )
		io.AddInputCharacter( (unsigned short)c );
}
