#include <VulkanBackend.hpp>
#include <array>
#include <format>

namespace RHI::Vulkan
{
    IRenderPass* Device::addFullScreenPass(const RenderPassCreateInfo ci)
    {
        RenderPass* result = new RenderPass(ci);
        //m_Resources.allRenderPasses.push_back(result->handle);
        return result;
    }

    IRenderPass* Device::createRenderPass(const FramebufferDesc& framebufferDesc, const RenderPassCreateInfo& ci)
    {
        RenderPass* rp = new RenderPass(ci);

        VkRenderPass renderPass;

        if (framebufferDesc.colorAttachments.empty() && framebufferDesc.depthAttachment.texture == nullptr)
        {
            printf("Empty list of output attachments for RenderPass\n");
            exit(EXIT_FAILURE);
        }

        if(!framebufferDesc.colorAttachments.empty() && framebufferDesc.depthAttachment.texture)
        {
            printf("Creating color/depth render pass\n");
            // TODO: update create...RenderPass to support multiple color attachments
            if (!createColorAndDepthRenderPass(&renderPass, ci, framebufferDesc))
            {
                printf("Unable to create offscreen render pass\n");
                exit(EXIT_FAILURE);
            }
        }

        //m_Resources.allRenderPasses.push_back(renderPass);
        rp->info = ci;
        rp->handle = renderPass;
        return rp;
    }

    IRenderPass* Device::addDepthRenderPass(const RenderPassCreateInfo ci)
    {
        RenderPass* rp = new RenderPass(ci);
        VkRenderPass renderPass;

        if (!createDepthOnlyRenderPass(&renderPass, ci))
        {
            printf("Unable to create offscreen render pass\n");
            exit(EXIT_FAILURE);
        }

        m_Resources.allRenderPasses.push_back(renderPass);
        rp->info = ci;
        rp->handle = renderPass;
        return rp;
    }

    bool Device::createColorAndDepthRenderPass(VkRenderPass* renderPass, const RenderPassCreateInfo& ci, const FramebufferDesc& framebufferDesc)
    {
        const bool offscreenInt = ci.flags & eRenderPassBit_OffscreenInternal;
        const bool first = ci.flags & eRenderPassBit_First;
        const bool last = ci.flags & eRenderPassBit_Last;

        uint32_t attachmentIdx = 0;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorAttachmentRefs;

        for(uint32_t i = 0; i < framebufferDesc.colorAttachments.size(); i++)
        {
            Texture* tex = dynamic_cast<Texture*>(framebufferDesc.colorAttachments[i].texture);
            if(!tex)
            {
                printf("Empty color attachment");
                exit(EXIT_FAILURE);
            }

            VkAttachmentDescription colorAttachment{};
            colorAttachment.flags = 0;
            colorAttachment.format = convertFormat(tex->getDesc().format);
            colorAttachment.samples = (VkSampleCountFlagBits)tex->getDesc().sampleCount;
            colorAttachment.loadOp = offscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ci.clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD);
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = first ? VK_IMAGE_LAYOUT_UNDEFINED : (offscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            colorAttachment.finalLayout = last ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            if (ci.flags & eRenderPassBit_Offscreen)
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            attachments.push_back(colorAttachment);

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = attachmentIdx;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentRefs.push_back(colorAttachmentRef);
        }

        bool useDepth = false;
        VkAttachmentReference depthAttachmentRef{};
        if(Texture* depthTex = dynamic_cast<Texture*>(framebufferDesc.depthAttachment.texture))
        {
            useDepth = true;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.flags = 0;
            depthAttachment.format = convertFormat(findDepthFormat());
            depthAttachment.samples = (VkSampleCountFlagBits)depthTex->getDesc().sampleCount;
            depthAttachment.loadOp = offscreenInt ? VK_ATTACHMENT_LOAD_OP_LOAD : (ci.clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD);
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = ci.clearDepth ? VK_IMAGE_LAYOUT_UNDEFINED : (offscreenInt ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            if (ci.flags & eRenderPassBit_Offscreen)
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            attachments.push_back(depthAttachment);
            
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        std::vector<VkSubpassDependency> dependencies;
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dependencyFlags = 0;

        if (ci.flags & eRenderPassBit_Offscreen)
        {
        	// Use subpass dependencies for layout transitions
            dependencies.resize(2);

            VkSubpassDependency subpassDependency1{};
            subpassDependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency1.dstSubpass = 0;
            subpassDependency1.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            subpassDependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency1.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            subpassDependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            subpassDependency1.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependencies[0] = subpassDependency1;

            VkSubpassDependency subpassDependency2{};
            subpassDependency2.srcSubpass = 0;
            subpassDependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
            subpassDependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpassDependency2.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            subpassDependency2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            subpassDependency2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            subpassDependency2.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            dependencies[1] = subpassDependency2;
        }

        VkSubpassDescription subpass{};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
        subpass.pColorAttachments = colorAttachmentRefs.data();
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = useDepth ? &depthAttachmentRef : nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.flags = 0;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        return (vkCreateRenderPass(m_Context.device, &renderPassInfo, nullptr, renderPass) == VK_SUCCESS);
    }

    bool Device::createDepthOnlyRenderPass(VkRenderPass* renderPass, const RenderPassCreateInfo& ci)
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.flags = 0;
        depthAttachment.format = convertFormat(findDepthFormat());
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = ci.clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = ci.clearDepth ? VK_IMAGE_LAYOUT_UNDEFINED : (ci.flags & eRenderPassBit_OffscreenInternal ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::vector<VkSubpassDependency> dependencies;

        if (ci.flags & eRenderPassBit_Offscreen)
        {
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // Use subpass dependencies for layout transitions
            //dependencies.resize(2);

            //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            //dependencies[0].dstSubpass = 0;
            //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; //VK_PIPELINE_STAGE_DEPTH_STENCIL_ATTACHMENT_OUTPUT_BIT;
            //dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            //dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            //dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            //dependencies[1].srcSubpass = 0;
            //dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_DEPTH_STENCIL_ATTACHMENT_OUTPUT_BIT;
            //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            //dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            //dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            //dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        VkSubpassDescription subpass{};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = 0;
        subpass.pColorAttachments = nullptr;
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pNext = nullptr;
        renderPassInfo.flags = 0;
        renderPassInfo.attachmentCount = 1u;
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1u;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        return (vkCreateRenderPass(m_Context.device, &renderPassInfo, nullptr, renderPass) == VK_SUCCESS);
    }
}
