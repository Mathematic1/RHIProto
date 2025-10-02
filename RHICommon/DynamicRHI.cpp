#include <RHICommon.hpp>

namespace RHI
{
	bool IDynamicRHI::CreateDevice()
	{
		createDeviceInternal();

		return true;
	}

	void IDynamicRHI::BackBufferResizing()
	{
		m_SwapChainFramebuffers.clear();
	}

	void IDynamicRHI::BackBufferResized()
	{
		uint32_t backBufferCount = GetBackBufferCount();
		m_SwapChainFramebuffers.resize(backBufferCount);
		for(uint32_t index = 0; index < backBufferCount; index++)
		{
		    FramebufferDesc framebufferDesc = {};
		    framebufferDesc.addColorAttachment({GetBackBuffer(index)}).setDepthAttachment({GetDepthBuffer()});
		    RenderPassCreateInfo ci{};
		    ci.clearColor = true;
		    ci.clearDepth = true;
		    ci.flags = eRenderPassBit_First | eRenderPassBit_Last;
		    IRenderPass* renderPass = getDevice()->createRenderPass(framebufferDesc, ci);
		    m_SwapChainFramebuffers[index] = getDevice()->createFramebuffer(renderPass, framebufferDesc);
		}
	}

}
