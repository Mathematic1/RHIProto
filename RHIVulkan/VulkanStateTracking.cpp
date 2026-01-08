#include <VulkanBackend.hpp>

#include <assert.h>

namespace RHI::Vulkan
{
    void CommandList::setResourceStatesForBindingSet(IBindingSet *bindingSet) {
        if (!bindingSet) {
            return;
        }

        BindingSet *binding = static_cast<BindingSet *>(bindingSet);
        for (auto &textureIdx : binding->texturesWithoutPermanentState) {
            TextureAttachment &textureAttachment = binding->desc.textures[textureIdx];

            switch (textureAttachment.dInfo.type) {
            case DescriptorType::COMBINED_IMAGE_SAMPLER:
                requireTextureState(
                    textureAttachment.texture, textureAttachment.dInfo.subresource, ResourceStates::ShaderResource
                );
            }
        }
    }

    void CommandList::trackResourcesAndBarriers(const GraphicsState &state) {
        assert(m_EnableAutoBarriers);

        for (size_t i = 0; i < state.bindingSets.size(); i++) {
            setResourceStatesForBindingSet(state.bindingSets[i]);
        }

        if (m_CurrentGraphicsState.framebuffer != state.framebuffer) {
            setTextureStatesForFramebuffer(state.framebuffer);
        }
    }

    void CommandList::transitionBufferLayout(IBuffer *buffer, ImageLayout oldLayout, ImageLayout newLayout) {
        Buffer *buf = dynamic_cast<Buffer *>(buffer);
        transitionBufferLayoutCmd(buf->buffer, convertFormat(buf->desc.format), VkImageLayout{}, VkImageLayout{}, 0, 0);
    }

    void CommandList::transitionBufferLayoutCmd(
        VkBuffer buffer, VkFormat format, VkAccessFlags oldAccess, VkAccessFlags newAccess, uint32_t offset, uint32_t size
    ) {
        VkBufferMemoryBarrier barrier{};

        VkPipelineStageFlags sourceStage{};
        VkPipelineStageFlags destinationStage{};

        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.offset = offset;
        barrier.size = size;

        vkCmdPipelineBarrier(
            m_CurrentCommandBuffer->commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 1, &barrier, 0, nullptr
        );
    }

    void CommandList::requireTextureState(ITexture *texture, const TextureSubresource &subresource, ResourceStates requiredState) {
        Texture *tex = dynamic_cast<Texture *>(texture);

        m_StateTracker.requireTextureState(tex, subresource, requiredState);
    }

    void CommandList::beginTrackingTextureState(
        ITexture *texture, TextureSubresource subresources, ResourceStates stateBits
    ) {
        Texture *tex = dynamic_cast<Texture *>(texture);

        m_StateTracker.beginTrackingTextureState(tex, subresources, stateBits);
    }

    void CommandList::setTextureState(ITexture *texture, TextureSubresource subresource, ResourceStates states) {
        Texture *tex = dynamic_cast<Texture *>(texture);

        m_StateTracker.requireTextureState(tex, subresource, states);

        if (m_CurrentCommandBuffer) {
            m_CurrentCommandBuffer->referencedResources.push_back(tex);
        }
    }

    void CommandList::setPermanentTextureState(ITexture *texture, ResourceStates states) {
        Texture *tex = dynamic_cast<Texture *>(texture);

        m_StateTracker.setPermanentTextureState(
            tex, TextureSubresource{ 0, tex->desc.mipLevels, 0, tex->desc.layerCount }, states
        );

        if (m_CurrentCommandBuffer) {
            m_CurrentCommandBuffer->referencedResources.push_back(tex);
        }
    }

    void CommandList::commitBarriers() {
        const auto &barriers = m_StateTracker.getTextureBarriers();
        if (barriers.empty()) {
            return;
        }

        endRenderPass();

        std::vector<VkImageMemoryBarrier> imageBarriers;
        //imageBarriers.reserve(barriers.size());

        VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        for (const TextureBarrier &barrier : barriers) {
            ResourceStateMapping before = convertResourceState(barrier.stateBefore);
            ResourceStateMapping after = convertResourceState(barrier.stateAfter);

            if ((before.stages != srcStages || after.stages != dstStages) && !imageBarriers.empty()) {
                vkCmdPipelineBarrier(
                    m_CurrentCommandBuffer->commandBuffer,
                    srcStages,
                    dstStages,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    imageBarriers.size(),
                    imageBarriers.data()
                );

                imageBarriers.clear();
            }

            srcStages = before.stages;
            dstStages = after.stages;

            Texture *texture = static_cast<Texture *>(barrier.texture);
            const VkFormat format = convertFormat(texture->desc.format);

            VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            if (after.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || (format == VK_FORMAT_D16_UNORM) ||
                (format == VK_FORMAT_X8_D24_UNORM_PACK32) || (format == VK_FORMAT_D32_SFLOAT) ||
                (format == VK_FORMAT_S8_UINT) || (format == VK_FORMAT_D16_UNORM_S8_UINT) ||
                (format == VK_FORMAT_D24_UNORM_S8_UINT)) {
                aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                if (hasStencilComponent(format)) {
                    aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            } else {
                aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            VkImageMemoryBarrier imageBarrier{};
            imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageBarrier.pNext = nullptr;
            imageBarrier.srcAccessMask = before.accessMask;
            imageBarrier.dstAccessMask = after.accessMask;
            imageBarrier.oldLayout = before.layout;
            imageBarrier.newLayout = after.layout;
            imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageBarrier.image = texture->image;
            imageBarrier.subresourceRange.aspectMask = aspectMask;
            imageBarrier.subresourceRange.baseMipLevel = barrier.entireTexture ? 0 : barrier.mipLevel;
            imageBarrier.subresourceRange.levelCount = barrier.entireTexture ? texture->getDesc().mipLevels : 1;
            imageBarrier.subresourceRange.baseArrayLayer = barrier.entireTexture ? 0 : barrier.arraySlice;
            imageBarrier.subresourceRange.layerCount = barrier.entireTexture ? texture->getDesc().layerCount : 1;

            imageBarriers.push_back(imageBarrier);

            texture->currentLayout = after.layout;
        }

        if (!imageBarriers.empty()) {
            vkCmdPipelineBarrier(
                m_CurrentCommandBuffer->commandBuffer,
                srcStages,
                dstStages,
                0,
                0,
                nullptr,
                0,
                nullptr,
                imageBarriers.size(),
                imageBarriers.data()
            );
        }
        imageBarriers.clear();

        m_StateTracker.clearBarriers();
    }

    void CommandList::setTextureStatesForFramebuffer(IFramebuffer *framebuffer) {
        const FramebufferDesc &desc = framebuffer->getDesc();

        for (const auto &attachment : desc.colorAttachments) {
            setTextureState(attachment.texture, attachment.subresource, ResourceStates::RenderTarget);
        }

        if (desc.depthAttachment.texture) {
            setTextureState(desc.depthAttachment.texture, desc.depthAttachment.subresource, ResourceStates::DepthWrite);
        }
    }

}
