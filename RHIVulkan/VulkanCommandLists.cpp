#include <cassert>
#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
	CommandList::CommandList(Device* device, VulkanContext& context, const CommandListParameters& parameters)
		: m_Device(device)
		, m_Context(context)
		, m_CommandListParameters(parameters)
	{
		
	}

	CommandList::~CommandList()
	{
		
	}

    void CommandList::beginSingleTimeCommands()
    {
        m_CurrentCommandBuffer = m_Device->getQueue(m_CommandListParameters.queueType)->getOrCreateCommandBuffer();

        vkResetCommandBuffer(m_CurrentCommandBuffer->commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_CurrentCommandBuffer->commandBuffer, &beginInfo);
    }

    void CommandList::endSingleTimeCommands()
    {
        endRenderPass();

        vkEndCommandBuffer(m_CurrentCommandBuffer->commandBuffer);

        /*VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CurrentCommandBuffer->commandBuffer;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;

        VkQueue submitQueue = m_Device->getQueue(m_CommandListParameters.queueType)->getVkQueue();
        VkResult resultQueueSubmit =  vkQueueSubmit(submitQueue, 1, &submitInfo, VK_NULL_HANDLE);
        VkResult resultQueueWaidIdle = vkQueueWaitIdle(submitQueue);

        vkFreeCommandBuffers(m_Context.device, m_CurrentCommandBuffer->commandPool, 1, &m_CurrentCommandBuffer->commandBuffer);*/
    }

    void CommandList::queueWaitIdle()
    {
        vkQueueWaitIdle(m_Device->getQueue(m_CommandListParameters.queueType)->getVkQueue());

        vkFreeCommandBuffers(m_Context.device, m_CurrentCommandBuffer->commandPool, 1, &m_CurrentCommandBuffer->commandBuffer);
    }

    void CommandList::copyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size)
    {
        Buffer* srcBuf = dynamic_cast<Buffer*>(srcBuffer);
        Buffer* dstBuf = dynamic_cast<Buffer*>(dstBuffer);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        vkCmdCopyBuffer(m_CurrentCommandBuffer->commandBuffer, srcBuf->buffer, dstBuf->buffer, 1, &copyRegion);
    }

    void CommandList::transitionImageLayout(ITexture* texture, ImageLayout oldLayout, ImageLayout newLayout)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);
        transitionImageLayoutCmd(
            tex->image,
            convertFormat(tex->getDesc().format),
            convertImageLayout(oldLayout),
            convertImageLayout(newLayout),
            tex->getDesc().layerCount,
            tex->getDesc().mipLevels);
        tex->desiredLayout = convertImageLayout(newLayout);
    }

    void CommandList::transitionImageLayoutCmd(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount, uint32_t mipLevels)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
            (format == VK_FORMAT_D16_UNORM) ||
            (format == VK_FORMAT_X8_D24_UNORM_PACK32) ||
            (format == VK_FORMAT_D32_SFLOAT) ||
            (format == VK_FORMAT_S8_UINT) ||
            (format == VK_FORMAT_D16_UNORM_S8_UINT) ||
            (format == VK_FORMAT_D24_UNORM_S8_UINT))
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (hasStencilComponent(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        /* Convert back from read-only to updateable */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        /* Convert from updateable texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        /* Convert depth texture from undefined state to depth-stencil buffer */
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        /* Wait for render pass to complete */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = 0; // VK_ACCESS_SHADER_READ_BIT
            barrier.dstAccessMask = 0;
            /*
                    sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ///		destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                    destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            */
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        /* Convert back from read-only to color attachment */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        /* Convert from updateable texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        /* Convert back from read-only to depth attachment */
        else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            destinationStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        }
        /* Convert from updateable depth texture to shader read-only */
        else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(
            m_CurrentCommandBuffer->commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void CommandList::transitionBufferLayout(IBuffer* buffer, ImageLayout oldLayout, ImageLayout newLayout)
    {
        Buffer* buf = dynamic_cast<Buffer*>(buffer);
        transitionBufferLayoutCmd(buf->buffer, convertFormat(buf->desc.format), VkImageLayout{}, VkImageLayout{}, 0, 0);
    }

    void CommandList::transitionBufferLayoutCmd(VkBuffer buffer, VkFormat format, VkAccessFlags oldAccess, VkAccessFlags newAccess, uint32_t offset, uint32_t size)
    {
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
            m_CurrentCommandBuffer->commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            1, &barrier,
            0, nullptr);
    }

    void CommandList::copyBufferToImage(IBuffer* buffer, ITexture* texture, uint32_t mipLevel, uint32_t baseArrayLayer)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);
        Buffer* buf = dynamic_cast<Buffer*>(buffer);

        const uint32_t mipWidth = std::max(tex->desc.width >> mipLevel, uint32_t(1));
        const uint32_t mipHeight = std::max(tex->desc.height >> mipLevel, uint32_t(1));
        const uint32_t mipDepth = std::max(tex->desc.depth >> mipLevel, uint32_t(1));

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mipLevel;
        region.imageSubresource.baseArrayLayer = baseArrayLayer;
        region.imageSubresource.layerCount = tex->getDesc().layerCount;
        region.imageOffset = VkOffset3D{ 0, 0, 0 };
        region.imageExtent = VkExtent3D{mipWidth, mipHeight, mipDepth};

        vkCmdCopyBufferToImage(m_CurrentCommandBuffer->commandBuffer, buf->buffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void CommandList::copyMIPBufferToImage(IBuffer* buffer, ITexture* texture)
    {
        Buffer* buf = dynamic_cast<Buffer*>(buffer);
        Texture* tex = dynamic_cast<Texture*>(texture);

        FormatInfo formatInfo = getFormatInfo(tex->getDesc().format);
        uint32_t mipWidth = tex->getDesc().width, mipHeight = tex->getDesc().height, mipDepth = tex->getDesc().depth;
        uint32_t offset = 0;
        std::vector<VkBufferImageCopy> regions(tex->getDesc().mipLevels);
        for (uint32_t i = 0; i < tex->getDesc().mipLevels; i++)
        {
            mipWidth = std::max(mipWidth >> 1, uint32_t(1));
            mipHeight = std::max(mipHeight >> 1, uint32_t(1));
            mipDepth = std::max(mipDepth >> 1, uint32_t(1));

            const uint32_t numColumns = (mipWidth + formatInfo.blockSize - 1) / formatInfo.blockSize;
            const uint32_t numRows = (mipHeight + formatInfo.blockSize - 1) / formatInfo.blockSize;
            const uint32_t rowSize = numColumns * formatInfo.bytesPerBlock;
            size_t layerSize = rowSize * numRows * mipDepth;

            VkImageSubresourceLayers subresource{};
            subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresource.mipLevel = i;
            subresource.baseArrayLayer = 0;
            subresource.layerCount = tex->getDesc().layerCount;
            VkBufferImageCopy region{};
            region.bufferOffset = offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource = subresource;
            region.imageOffset = VkOffset3D{ 0, 0, 0 };
            region.imageExtent = VkExtent3D{ mipWidth, mipHeight, 1 };

            offset += layerSize * tex->getDesc().layerCount;

            regions[i] = region;
        }

        vkCmdCopyBufferToImage(m_CurrentCommandBuffer->commandBuffer, buf->buffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)regions.size(), regions.data());
    }

    void CommandList::copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount)
    {
        VkImageSubresourceLayers imageSubresourceLayers{};
        imageSubresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageSubresourceLayers.mipLevel = 0;
        imageSubresourceLayers.baseArrayLayer = 0;
        imageSubresourceLayers.layerCount = layerCount;

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource = imageSubresourceLayers;
        region.imageOffset = VkOffset3D{ 0, 0, 0 };
        region.imageExtent = VkExtent3D{ width, height, 1 };

        vkCmdCopyImageToBuffer(m_CurrentCommandBuffer->commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &region);
    }

    void CommandList::setPushConstants(const void* data, size_t byteSize)
    {
        assert(m_CurrentCommandBuffer);

        vkCmdPushConstants(m_CurrentCommandBuffer->commandBuffer, m_CurrentPipelineLayout, m_CurrentPushConstantsVisibility, 0, byteSize, data);
    }

    void CommandList::executed(Queue &queue, const uint64_t submissionID)
    {
        assert(m_CurrentCommandBuffer);

        m_CurrentCommandBuffer->submissionID = submissionID;

        m_CurrentCommandBuffer = nullptr;
    }

    void CommandList::draw(const DrawArguments& args)
	{
        assert(m_CurrentCommandBuffer);

        vkCmdDraw(m_CurrentCommandBuffer->commandBuffer,
            args.vertexCount,
            args.instanceCount,
            args.startVertexLocation,
            args.startInstanceLocation);
	}

    void CommandList::drawIndexed(const DrawArguments& args)
    {
        assert(m_CurrentCommandBuffer);

        vkCmdDrawIndexed(m_CurrentCommandBuffer->commandBuffer,
            args.vertexCount,
            args.instanceCount,
            args.startIndexLocation,
            args.startVertexLocation,
            args.startInstanceLocation);
    }
}
