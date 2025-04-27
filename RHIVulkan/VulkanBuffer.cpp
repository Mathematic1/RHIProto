#include <VulkanBackend.hpp>

//#include <EasyProfilerWrapper.hpp>

namespace RHI::Vulkan
{
    static VkBufferUsageFlags pickBufferUsage(const BufferDesc& desc)
    {
        VkBufferUsageFlags ret = 0;

        if (desc.usage.isTransferSrc)
            ret |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        if (desc.usage.isTransferDst)
            ret |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    	if (desc.usage.isVertexBuffer)
            ret |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        if (desc.usage.isIndexBuffer)
            ret |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (desc.usage.isUniformBuffer)
            ret |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        if (desc.usage.isStorageBuffer)
            ret |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        if (desc.usage.isDrawIndirectBuffer)
            ret |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        return ret;
    }

	BufferHandle Device::createBuffer(const BufferDesc& desc)
    {
        Buffer* buffer = new Buffer(m_Context);

        buffer->desc = desc;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = desc.size;
        bufferInfo.usage = pickBufferUsage(desc);
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;

        VK_CHECK(vkCreateBuffer(m_Context.device, &bufferInfo, nullptr, &buffer->buffer));

        m_Context.setVkObjectName(buffer->buffer, VkObjectType::VK_OBJECT_TYPE_BUFFER, desc.debugName.c_str());

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Context.device, buffer->buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pickMemoryProperties(desc.memoryProperties));

        VK_CHECK(vkAllocateMemory(m_Context.device, &allocInfo, nullptr, &buffer->memory));

        vkBindBufferMemory(m_Context.device, buffer->buffer, buffer->memory, 0);

        return BufferHandle(buffer);
    }

    BufferHandle Device::createSharedBuffer(const BufferDesc& desc)
    {
        uint32_t familyCount = static_cast<uint32_t>(m_DeviceQueueIndices.size());

        if (familyCount < 2)
            return createBuffer(desc);

        Buffer* buffer = new Buffer(m_Context);

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = desc.size;
        bufferInfo.usage = pickBufferUsage(desc);
        bufferInfo.sharingMode = (familyCount > 1) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(m_DeviceQueueIndices.size());
        bufferInfo.pQueueFamilyIndices = (familyCount > 1) ? m_DeviceQueueIndices.data() : nullptr;

        VK_CHECK(vkCreateBuffer(m_Context.device, &bufferInfo, nullptr, &buffer->buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Context.device, buffer->buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pickMemoryProperties(desc.memoryProperties));

        VK_CHECK(vkAllocateMemory(m_Context.device, &allocInfo, nullptr, &buffer->memory));

        vkBindBufferMemory(m_Context.device, buffer->buffer, buffer->memory, 0);

        return BufferHandle(buffer);
    }

    BufferHandle Device::createUniformBuffer(VkDeviceSize bufferSize)
    {
        BufferDesc desc = BufferDesc{}
    		.setSize(bufferSize)
            .setIsUniformBuffer(true)
            .setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT);
        return createBuffer(desc);
    }

    void Device::uploadBufferData(IBuffer* buffer, size_t deviceOffset, const void* data, const size_t dataSize)
    {
        //EASY_FUNCTION()

    	Buffer* buf = dynamic_cast<Buffer*>(buffer);

    	void* mappedData = nullptr;
        vkMapMemory(m_Context.device, buf->memory, deviceOffset, dataSize, 0, &mappedData);
        memcpy(mappedData, data, dataSize);
        vkUnmapMemory(m_Context.device, buf->memory);
    }

    void Device::uploadVertexIndexBufferData(IBuffer* buffer, size_t deviceOffset, size_t vertexDataSize, const void* vertexData,
        size_t indexDataSize, const void* indexData, const size_t dataSize)
    {
        //EASY_FUNCTION()

    	Buffer* buf = dynamic_cast<Buffer*>(buffer);

        void* mappedData = nullptr;
        vkMapMemory(m_Context.device, buf->memory, deviceOffset, dataSize, 0, &mappedData);
        memcpy(mappedData, vertexData, vertexDataSize);
        memcpy((unsigned char*)mappedData + vertexDataSize, indexData, indexDataSize);
        vkUnmapMemory(m_Context.device, buf->memory);
    }

    void Device::downloadBufferData(const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, void* outData, size_t dataSize)
    {
        //EASY_FUNCTION()

    	void* mappedData = nullptr;
        vkMapMemory(m_Context.device, bufferMemory, deviceOffset, dataSize, 0, &mappedData);
        memcpy(outData, mappedData, dataSize);
        vkUnmapMemory(m_Context.device, bufferMemory);
    }

    void* Device::mapBufferMemory(IBuffer* buffer, size_t offset, size_t size)
    {
        Buffer* buf = dynamic_cast<Buffer*>(buffer);

        void* mappedData = nullptr;
        vkMapMemory(m_Context.device, buf->memory, offset, size, 0, &mappedData);

        return mappedData;
    }

    void Device::unmapBufferMemory(IBuffer* buffer)
    {
        Buffer* buf = dynamic_cast<Buffer*>(buffer);

        vkUnmapMemory(m_Context.device, buf->memory);
    }

    BufferHandle Device::addBuffer(const BufferDesc& desc, bool createMapping)
    {
		Buffer* buffer = dynamic_cast<Buffer*>(createSharedBuffer(desc).get());
        if (!buffer)
        {
            printf("Cannot allocate buffer\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            buffer->size = desc.size;
            //m_Resources.allBuffers.push_back(buffer);
        }

        if (createMapping)
            vkMapMemory(m_Context.device, buffer->memory, 0, VK_WHOLE_SIZE, 0, &buffer->ptr);

        return BufferHandle(buffer);
    }

    void CommandList::writeBuffer(IBuffer* srcBuffer, size_t size, const void* data)
    {
    	BufferDesc stagingDesc = BufferDesc{}
            .setSize(size)
            .setIsTransferSrc(true)
            .setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT);
        BufferHandle stagingBuffer = m_Device->createBuffer(stagingDesc);

        m_Device->uploadBufferData(stagingBuffer.get(), 0, data, size);

        m_CurrentCommandBuffer->referencedStagingBuffers.push_back(stagingBuffer);

        copyBuffer(stagingBuffer.get(), srcBuffer, size);
    }

    Buffer::~Buffer()
	{
        vkDestroyBuffer(m_Context.device, buffer, nullptr);
        vkFreeMemory(m_Context.device, memory, nullptr);
	}
}
