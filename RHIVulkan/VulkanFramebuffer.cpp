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

    IFramebuffer* Device::createFramebuffer(IRenderPass* renderPass, const std::vector<ITexture*>& images)
    {
        Framebuffer* fb = new Framebuffer(m_Context);
        RenderPass* rp = dynamic_cast<RenderPass*>(renderPass);

        std::vector<VkImageView> attachments;
        for (const auto& image : images)
        {
            Texture* texture = dynamic_cast<Texture*>(image);
            attachments.push_back(texture->imageView);

            if(texture->desiredLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
            {
                fb->desc.colorAttachments.push_back(texture);
            }
            else if(texture->desiredLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
            {
                fb->desc.depthAttachment = texture;
            }
        }

        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.pNext = nullptr;
        fbInfo.flags = 0;
        fbInfo.renderPass = rp->handle;
        fbInfo.attachmentCount = (uint32_t)attachments.size();
        fbInfo.pAttachments = attachments.data();
        fbInfo.width = images[0]->getDesc().width;
        fbInfo.height = images[0]->getDesc().height;
        fbInfo.layers = 1;

        if (vkCreateFramebuffer(m_Context.device, &fbInfo, nullptr, &fb->framebuffer) != VK_SUCCESS)
        {
            printf("Unable to create offscreen framebuffer\n");
            exit(EXIT_FAILURE);
        }

        // TODO:fix allocation issue
        //m_Resources.allFramebuffers.push_back(fb->framebuffer);
        fb->renderPass = rp->handle;
        fb->framebufferWidth = images[0]->getDesc().width;
        fb->framebufferHeight = images[0]->getDesc().height;
        return fb;
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
