#include <cassert>
#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    CommandList::CommandList(Device *device, VulkanContext &context, const CommandListParameters &parameters)
        : m_Device(device), m_Context(context), m_CommandListParameters(parameters)
    {
    }

    CommandList::~CommandList()
    {
    }

    void CommandList::beginSingleTimeCommands()
    {
        m_CurrentCommandBuffer = m_Device->getQueue(m_CommandListParameters.queueType)->getOrCreateCommandBuffer();

        //vkResetCommandBuffer(m_CurrentCommandBuffer->commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(m_CurrentCommandBuffer->commandBuffer, &beginInfo);

        clearState();
    }

    void CommandList::endSingleTimeCommands()
    {
        endRenderPass();

        m_StateTracker.keepTextureInitialStates();
        commitBarriers();

        vkEndCommandBuffer(m_CurrentCommandBuffer->commandBuffer);

        clearState();

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

    void CommandList::clearState()
    {
        endRenderPass();

        m_CurrentPipelineLayout = VkPipelineLayout();
        m_CurrentPushConstantsVisibility = VkShaderStageFlags();

        m_CurrentGraphicsState = GraphicsState();
    }

    void CommandList::queueWaitIdle()
    {
        vkQueueWaitIdle(m_Device->getQueue(m_CommandListParameters.queueType)->getVkQueue());

        if (m_CurrentCommandBuffer) {
            vkFreeCommandBuffers(m_Context.device, m_CurrentCommandBuffer->commandPool, 1,
                                 &m_CurrentCommandBuffer->commandBuffer);
        }
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

    void CommandList::copyBufferToImage(IBuffer* buffer, ITexture* texture, uint32_t mipLevel, uint32_t baseArrayLayer)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);
        Buffer* buf = dynamic_cast<Buffer*>(buffer);

        const uint32_t mipWidth = std::max(tex->desc.width >> mipLevel, 1u);
        const uint32_t mipHeight = std::max(tex->desc.height >> mipLevel, 1u);
        const uint32_t mipDepth = std::max(tex->desc.depth >> mipLevel, 1u);

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
            mipWidth = std::max(mipWidth >> 1, 1u);
            mipHeight = std::max(mipHeight >> 1, 1u);
            mipDepth = std::max(mipDepth >> 1, 1u);

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

        m_StateTracker.commandListSubmitted();
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
