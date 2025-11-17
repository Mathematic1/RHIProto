#include <RHICommon.hpp>

namespace RHI
{
    bool IDynamicRHI::CreateDevice() {
        createDeviceInternal();

        return true;
    }

    void IDynamicRHI::BackBufferResizing() {
        m_SwapChainFramebuffers.clear();
    }

    void IDynamicRHI::BackBufferResized() {
        uint32_t backBufferCount = GetBackBufferCount();
        m_SwapChainFramebuffers.resize(backBufferCount);
        for (uint32_t index = 0; index < backBufferCount; index++) {
            FramebufferDesc framebufferDesc = {};
            framebufferDesc.addColorAttachment({ GetBackBuffer(index) });
            if (m_DeviceParams.backBufferUseDepth) {
                framebufferDesc.setDepthAttachment({ GetDepthBuffer() });
            }

            IRenderPass *renderPass = getDevice()->createRenderPass(framebufferDesc, m_DeviceParams.renderPassCreateInfo);
            m_SwapChainFramebuffers[index] = getDevice()->createFramebuffer(renderPass, framebufferDesc);
        }
    }

}
