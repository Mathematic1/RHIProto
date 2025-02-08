#include "VulkanBackend.hpp"
#include <array>

namespace RHI::Vulkan
{
	bool Device::createColorAndDepthFramebuffers(VkRenderPass renderPass, VkImageView depthImageView, std::vector<VkFramebuffer>& swapchainFramebuffers)
    {
        swapchainFramebuffers.resize(m_Resources.swapchainImageViews.size());

        for (size_t i = 0; i < m_Resources.swapchainImageViews.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                    m_Resources.swapchainImageViews[i],
                    depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.flags = 0;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>((depthImageView == VK_NULL_HANDLE) ? 1 : 2);
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_DeviceDesc.framebufferWidth;
            framebufferInfo.height = m_DeviceDesc.framebufferHeight;
            framebufferInfo.layers = 1;

            VK_CHECK(vkCreateFramebuffer(m_Context.device, &framebufferInfo, nullptr, &swapchainFramebuffers[i]));
        }

        return true;
    }

    FramebufferHandle Device::createFramebuffer(IRenderPass* renderPass, const FramebufferDesc& desc)
    {
        Framebuffer* fb = new Framebuffer(m_Context);
        RenderPass* rp = dynamic_cast<RenderPass*>(renderPass);

        fb->desc = desc;

        fb->renderPass = rp->handle;

        if(desc.depthAttachment.texture)
        {
            fb->framebufferWidth = desc.depthAttachment.texture->getDesc().width >> desc.depthAttachment.subresource.mipLevel;
            fb->framebufferHeight = desc.depthAttachment.texture->getDesc().height >> desc.depthAttachment.subresource.mipLevel;
            fb->sampleCount = fb->desc.depthAttachment.texture->getDesc().sampleCount;
        }
        else if(!desc.colorAttachments.empty() && desc.colorAttachments[0].texture)
        {
            fb->framebufferWidth = desc.colorAttachments[0].texture->getDesc().width >> desc.colorAttachments[0].subresource.mipLevel;
            fb->framebufferHeight = desc.colorAttachments[0].texture->getDesc().height >> desc.colorAttachments[0].subresource.mipLevel;
            fb->sampleCount = fb->desc.colorAttachments[0].texture->getDesc().sampleCount;
        }

        std::vector<VkImageView> attachments(desc.colorAttachments.size());
        for (uint32_t i = 0; i < desc.colorAttachments.size(); i++)
        {
            Texture* texture = dynamic_cast<Texture*>(desc.colorAttachments[i].texture);
            attachments[i] = texture->imageView;
        }
        if(desc.depthAttachment.texture)
        {
            Texture* texture = dynamic_cast<Texture*>(desc.depthAttachment.texture);
            attachments.push_back(texture->imageView);
        }

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.pNext = nullptr;
        fbInfo.flags = 0;
        fbInfo.renderPass = rp->handle;
        fbInfo.attachmentCount = (uint32_t)attachments.size();
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = fb->framebufferWidth;
        fbInfo.height = fb->framebufferHeight;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(m_Context.device, &fbInfo, nullptr, &fb->framebuffer) != VK_SUCCESS)
        {
            printf("Unable to create offscreen framebuffer\n");
            exit(EXIT_FAILURE);
        }

        // TODO:fix allocation issue
        //m_Resources.allFramebuffers.push_back(fb->framebuffer);

        return FramebufferHandle(fb);
    }

    std::vector<VkFramebuffer> Device::addFramebuffers(VkRenderPass renderPass, VkImageView depthView)
    {
        std::vector<VkFramebuffer> framebuffers;
        createColorAndDepthFramebuffers(renderPass, depthView, framebuffers);
        // TODO:fix allocation issue
        //for (auto f : framebuffers)
            //m_Resources.allFramebuffers.push_back(f);
        return framebuffers;
    }
}
