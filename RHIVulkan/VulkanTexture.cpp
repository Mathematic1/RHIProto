#include <VulkanBackend.hpp>

#include <assert.h>

namespace RHI::Vulkan
{
    Texture::~Texture()
    {
        for (auto &viewPair : subresourceViews) {
            VkImageView &view = viewPair.second.imageView;
            vkDestroyImageView(m_Context.device, view, nullptr);
            view = VkImageView();
        }

        if (managed)
        {
            if (image)
            {
                vkDestroyImage(m_Context.device, image, nullptr);
            }
            if (memory)
            {
                vkFreeMemory(m_Context.device, memory, nullptr);
            }
        }
    }

    Sampler::~Sampler()
    {
        vkDestroySampler(m_Context.device, sampler, nullptr);
    }

    static VkImageCreateInfo fillImageInfo(const TextureDesc& desc)
    {
	    
    }

    VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_Context.physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        printf("failed to find supported format!\n");
        exit(0);
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_Context.physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        return 0xFFFFFFFF;
    }

    Format Device::findDepthFormat()
    {
        VkFormat vkFormat = findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        switch (vkFormat)
        {
        case VK_FORMAT_D32_SFLOAT: return Format::D32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return Format::D32S8;
        case VK_FORMAT_D24_UNORM_S8_UINT: return Format::D24S8;
        default:
            return Format::UNKNOWN;
        }

        return Format::UNKNOWN;
    }

    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    bool hasDepthComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    static VkImageUsageFlags pickImageUsage(const TextureDesc& desc)
    {
        VkImageUsageFlags ret = 0;

        if(desc.usage.isTransferSrc)
        {
            ret |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if(desc.usage.isTransferDst)
        {
            ret |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (desc.usage.isShaderResource)
            ret |= VK_IMAGE_USAGE_SAMPLED_BIT;

        const VkFormat format = convertFormat(desc.format);

        if (desc.usage.isRenderTarget)
        {
            if (hasDepthComponent(format) || hasStencilComponent(format))
            {
                ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
            else {
                ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }
        }

        if (desc.usage.isUAV)
            ret |= VK_IMAGE_USAGE_STORAGE_BIT;

        return ret;
    }

    static VkImageCreateFlags pickImageFlag(const TextureDesc& desc)
    {
        VkImageCreateFlags ret = 0;

        if(desc.dimension == TextureDimension::TextureCube ||
            desc.dimension == TextureDimension::TextureCubeArray)
        {
            ret |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        return ret;
    }

    inline VkImageAspectFlags pickImageAspect(Format format) {
        switch (format) {
        case Format::D16:
        case Format::D32:
            return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;

        case Format::D24S8:
        case Format::D32S8:
            return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT;

        default:
            return VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }

    static VkImageType textureDimensionToImageType(TextureDimension dimension)
    {
        switch (dimension)
        {
        case TextureDimension::Texture1D:
        case TextureDimension::Texture1DArray:
            return VK_IMAGE_TYPE_1D;

        case TextureDimension::Texture2D:
        case TextureDimension::Texture2DMS:
        case TextureDimension::Texture2DArray:
        case TextureDimension::Texture2DMSArray:
        case TextureDimension::TextureCube:
        case TextureDimension::TextureCubeArray:
            return VK_IMAGE_TYPE_2D;

        case TextureDimension::Texture3D:
            return VK_IMAGE_TYPE_3D;

        case TextureDimension::Unknown:
        default:
            return VK_IMAGE_TYPE_2D;
        }
    }

    static VkImageViewType textureDimensionToImageViewType(TextureDimension dimension)
    {
        switch (dimension)
        {
        case TextureDimension::Texture1D:
            return VK_IMAGE_VIEW_TYPE_1D;

        case TextureDimension::Texture1DArray:
            return VK_IMAGE_VIEW_TYPE_1D_ARRAY;

        case TextureDimension::Texture2D:
        case TextureDimension::Texture2DMS:
            return VK_IMAGE_VIEW_TYPE_2D;

        case TextureDimension::Texture2DArray:
        case TextureDimension::Texture2DMSArray:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

        case TextureDimension::TextureCube:
            return VK_IMAGE_VIEW_TYPE_CUBE;

        case TextureDimension::TextureCubeArray:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

        case TextureDimension::Texture3D:
            return VK_IMAGE_VIEW_TYPE_3D;

        case TextureDimension::Unknown:
        default:
            return VK_IMAGE_VIEW_TYPE_2D;
        }
    }

    TextureHandle Device::createImage(const TextureDesc& desc)
    {
        Texture* tex = new Texture(m_Context);
        tex->desc = desc;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.flags = pickImageFlag(desc);
        imageInfo.imageType = textureDimensionToImageType(desc.dimension);
        imageInfo.format = convertFormat(desc.format);
        imageInfo.extent = VkExtent3D{ desc.width, desc.height, desc.depth };
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = (uint32_t)((pickImageFlag(desc) == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? 6 : 1);
        imageInfo.samples = (VkSampleCountFlagBits)desc.sampleCount;
        imageInfo.tiling = desc.isLinearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = pickImageUsage(desc);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = nullptr;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        checkSuccess(vkCreateImage(m_Context.device, &imageInfo, nullptr, &tex->image));

        m_Context.setVkImageName(tex->image, desc.debugName.c_str());

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Context.device, tex->image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pickMemoryProperties(desc.memoryProperties));

        checkSuccess(vkAllocateMemory(m_Context.device, &allocInfo, nullptr, &tex->memory));

        vkBindImageMemory(m_Context.device, tex->image, tex->memory, 0);
        return TextureHandle(tex);
    }

    TextureView *Texture::GetOrCreateSubresourceView(const TextureSubresource &subresource) {
        auto it = subresourceViews.find(subresource);
        if (it != subresourceViews.end()) {
            return &it->second;
        }
        
        auto [insertIt, inserted] = subresourceViews.emplace(subresource, *this);

        auto &view = insertIt->second;

        view.subresource = subresource;

        view.subresourceRange.aspectMask = pickImageAspect(desc.format);
        view.subresourceRange.baseMipLevel = subresource.mipLevel;
        view.subresourceRange.levelCount = subresource.mipLevelCount;
        view.subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        view.subresourceRange.layerCount = subresource.layerCount;

        VkImageViewCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.image = image;
        ci.viewType = textureDimensionToImageViewType(desc.dimension);
        ci.format = convertFormat(desc.format);
        ci.subresourceRange = view.subresourceRange;

        VkResult res = vkCreateImageView(m_Context.device, &ci, nullptr, &view.imageView);
        assert(res == VK_SUCCESS);

        return &insertIt->second;
    }

    SamplerHandle Device::createTextureSampler(const SamplerDesc& desc)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.pNext = nullptr;
        samplerInfo.flags = 0;
        samplerInfo.magFilter = desc.magFilter == SamplerFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerInfo.minFilter = desc.minFilter == SamplerFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerInfo.mipmapMode = desc.mipFilter == SamplerFilter::LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.addressModeU = convertSamplerAddressMode(desc.addressU); // VK_SAMPLER_ADDRESS_MODE_REPEAT,
        samplerInfo.addressModeV = convertSamplerAddressMode(desc.addressV); // VK_SAMPLER_ADDRESS_MODE_REPEAT,
        samplerInfo.addressModeW = convertSamplerAddressMode(desc.addressW); // VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE VK_SAMPLER_ADDRESS_MODE_REPEAT,
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = desc.anisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = desc.maxAnisotropy;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = std::numeric_limits<float>::max();
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        Sampler* sampler = new Sampler(m_Context);

        if(vkCreateSampler(m_Context.device, &samplerInfo, nullptr, &sampler->sampler) != VK_SUCCESS)
        {
            printf("Cannot create texture sampler\n");
            exit(EXIT_FAILURE);
        }

        return SamplerHandle(sampler);
    }

    SamplerHandle Device::createDepthSampler()
    {
        VkSamplerCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        si.pNext = nullptr;
        si.magFilter = VK_FILTER_LINEAR;
        si.minFilter = VK_FILTER_LINEAR;
        si.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        si.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        si.mipLodBias = 0.0f;
        si.maxAnisotropy = 1.0f;
        si.minLod = 0.0f;
        si.maxLod = 1.0f;
        si.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        Sampler* sampler = new Sampler(m_Context);

        if(vkCreateSampler(m_Context.device, &si, nullptr, &sampler->sampler) != VK_SUCCESS)
        {
            printf("Cannot create depth sampler\n");
            exit(EXIT_FAILURE);
        }

        return SamplerHandle(sampler);
    }

    bool CommandList::updateTextureImage(
        ITexture *texture, uint32_t mipLevel, uint32_t baseArrayLayer, const void *imageData, size_t rowPitch,
        size_t depthPitch
    ) {
        endRenderPass();

        Texture *tex = dynamic_cast<Texture *>(texture);

        const FormatInfo formatInfo = getFormatInfo(tex->desc.format);

        const uint32_t mipWidth = std::max(tex->desc.width >> mipLevel, uint32_t(1));
        const uint32_t mipHeight = std::max(tex->desc.height >> mipLevel, uint32_t(1));
        const uint32_t mipDepth = std::max(tex->desc.depth >> mipLevel, uint32_t(1));

        const uint32_t deviceNumColumns = (mipWidth + formatInfo.blockSize - 1) / formatInfo.blockSize;
        const uint32_t deviceNumRows = (mipHeight + formatInfo.blockSize - 1) / formatInfo.blockSize;
        const uint32_t deviceRowSize = deviceNumColumns * formatInfo.bytesPerBlock;
        VkDeviceSize layerSize = deviceRowSize * deviceNumRows * mipDepth;
        VkDeviceSize imageSize = layerSize * tex->desc.layerCount;

        if (rowPitch == 0) {
            rowPitch = deviceRowSize;
        }

        if (depthPitch == 0) {
            depthPitch = rowPitch * deviceNumRows;
        }

        BufferDesc stagingDesc = BufferDesc{}.setSize(imageSize).setIsTransferSrc(true).setMemoryProperties(
            MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT
        );
        BufferHandle stagingBuffer = m_Device->createBuffer(stagingDesc);
        m_CurrentCommandBuffer->referencedStagingBuffers.push_back(stagingBuffer);

        // m_Device->uploadBufferData(stagingBuffer.get(), 0, imageData, imageSize);
        m_Device->uploadMipLevelToStagingBuffer(
            stagingBuffer.get(),
            0,
            imageData,
            imageSize,
            deviceNumRows,
            deviceNumColumns,
            mipDepth,
            tex->desc.layerCount,
            rowPitch,
            depthPitch,
            deviceRowSize
        );

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                tex, { mipLevel, 1, baseArrayLayer, 1 }, ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        copyBufferToImage(stagingBuffer.get(), tex, mipLevel, baseArrayLayer);

        return true;
    }

    void* Device::mapStagingTextureMemory(ITexture* texture, size_t offset, size_t size)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);

        void* mappedData = nullptr;
        vkMapMemory(m_Context.device, tex->memory, offset, size, 0, &mappedData);

        return mappedData;
    }

    void Device::unmapStagingTextureMemory(ITexture* texture)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);

        vkUnmapMemory(m_Context.device, tex->memory);
    }

    TextureHandle Device::createTextureForNative(VkImage image, VkImageView imageView, const TextureDesc& desc)
    {
        Texture* tex = new Texture(m_Context);
        tex->desc = desc;
        tex->image = image;
        tex->managed = false;

        return TextureHandle(tex);
    }

    void CommandList::copyTexture(
        ITexture *srcTexture, const TextureSubresource &srcSubresource, const TextureRegion &srcRegion,
        ITexture *dstTexture, const TextureSubresource &dstSubresource, const TextureRegion &dstRegion
    )
    {
        endRenderPass();

        Texture* srcTex = dynamic_cast<Texture*>(srcTexture);
        Texture* dstTex = dynamic_cast<Texture*>(dstTexture);

        auto resolvedSrcRegion = srcRegion.resolveRegion(srcTex->desc);
        auto resolvedDstRegion = dstRegion.resolveRegion(dstTex->desc);

        m_CurrentCommandBuffer->referencedResources.push_back(srcTex);
        m_CurrentCommandBuffer->referencedResources.push_back(dstTex);

        VkImageCopy imageCopyRegion{};
        imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.srcSubresource.mipLevel = srcSubresource.mipLevel;
        imageCopyRegion.srcSubresource.baseArrayLayer = srcSubresource.baseArrayLayer;
        imageCopyRegion.srcSubresource.layerCount = srcSubresource.layerCount;
        imageCopyRegion.srcOffset = VkOffset3D(resolvedSrcRegion.x, resolvedSrcRegion.y, resolvedSrcRegion.z);
        imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopyRegion.dstSubresource.mipLevel = dstSubresource.mipLevel;
        imageCopyRegion.dstSubresource.baseArrayLayer = dstSubresource.baseArrayLayer;
        imageCopyRegion.dstSubresource.layerCount = dstSubresource.layerCount;
        imageCopyRegion.dstOffset = VkOffset3D(resolvedDstRegion.x, resolvedDstRegion.y, resolvedDstRegion.z);
        imageCopyRegion.extent.width = std::min<uint32_t>(resolvedSrcRegion.width, resolvedDstRegion.width);
        imageCopyRegion.extent.height = std::min<uint32_t>(resolvedSrcRegion.height, resolvedDstRegion.height);
        imageCopyRegion.extent.depth = std::min<uint32_t>(resolvedSrcRegion.depth, resolvedDstRegion.depth);

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                srcTex,
                { srcSubresource.mipLevel, 1, srcSubresource.baseArrayLayer, 1 },
                ResourceStates::CopySource
            );
            m_StateTracker.requireTextureState(
                dstTex,
                { dstSubresource.mipLevel, 1, dstSubresource.baseArrayLayer, 1 },
                ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        vkCmdCopyImage(
            m_CurrentCommandBuffer->commandBuffer,
            srcTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageCopyRegion);
    }

    void CommandList::blitTexture(
        ITexture *srcTexture, const TextureSubresource &srcSubresource, const TextureRegion &srcRegion,
        ITexture *dstTexture, const TextureSubresource &dstSubresource, const TextureRegion &dstRegion,
        RHI::SamplerFilter filter
    ) {
        endRenderPass();

        Texture *srcTex = dynamic_cast<Texture *>(srcTexture);
        Texture *dstTex = dynamic_cast<Texture *>(dstTexture);

        auto resolvedSrcRegion = srcRegion.resolveRegion(srcTex->desc);
        auto resolvedDstRegion = dstRegion.resolveRegion(dstTex->desc);

        m_CurrentCommandBuffer->referencedResources.push_back(srcTex);
        m_CurrentCommandBuffer->referencedResources.push_back(dstTex);

        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.mipLevel = srcSubresource.mipLevel;
        imageBlitRegion.srcSubresource.baseArrayLayer = srcSubresource.baseArrayLayer;
        imageBlitRegion.srcSubresource.layerCount = srcSubresource.layerCount;
        imageBlitRegion.srcOffsets[0] = VkOffset3D( resolvedSrcRegion.x, resolvedSrcRegion.y, resolvedSrcRegion.z );
        imageBlitRegion.srcOffsets[1] = VkOffset3D(
            resolvedSrcRegion.x + resolvedSrcRegion.width,
            resolvedSrcRegion.y + resolvedSrcRegion.height,
            resolvedSrcRegion.z + resolvedSrcRegion.depth
        );
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.mipLevel = dstSubresource.mipLevel;
        imageBlitRegion.dstSubresource.baseArrayLayer = dstSubresource.baseArrayLayer;
        imageBlitRegion.dstSubresource.layerCount = dstSubresource.layerCount;
        imageBlitRegion.dstOffsets[0] = VkOffset3D( resolvedDstRegion.x, resolvedDstRegion.y, resolvedDstRegion.z );
        imageBlitRegion.dstOffsets[1] = VkOffset3D(
            resolvedDstRegion.x + resolvedDstRegion.width,
            resolvedDstRegion.y + resolvedDstRegion.height,
            resolvedDstRegion.z + resolvedDstRegion.depth
        );

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                srcTex, { srcSubresource.mipLevel, 1, srcSubresource.baseArrayLayer, 1 }, ResourceStates::CopySource
            );
            m_StateTracker.requireTextureState(
                dstTex,
                { dstSubresource.mipLevel, 1, dstSubresource.baseArrayLayer, 1 },
                ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        vkCmdBlitImage(
            m_CurrentCommandBuffer->commandBuffer,
            srcTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlitRegion,
            filter == SamplerFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST
        );
    }

    void CommandList::resolveTexture(
        ITexture *srcTexture, const TextureSubresource &srcSubresource, ITexture *dstTexture,
        const TextureSubresource dstSubresource
    ) {
        endRenderPass();

        Texture *srcTex = dynamic_cast<Texture *>(srcTexture);
        Texture *dstTex = dynamic_cast<Texture *>(dstTexture);

        m_CurrentCommandBuffer->referencedResources.push_back(srcTex);
        m_CurrentCommandBuffer->referencedResources.push_back(dstTex);

        VkImageResolve imageResolveRegion{};
        imageResolveRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResolveRegion.srcSubresource.mipLevel = srcSubresource.mipLevel;
        imageResolveRegion.srcSubresource.baseArrayLayer = srcSubresource.baseArrayLayer;
        imageResolveRegion.srcSubresource.layerCount = srcSubresource.layerCount;
        imageResolveRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResolveRegion.dstSubresource.mipLevel = dstSubresource.mipLevel;
        imageResolveRegion.dstSubresource.baseArrayLayer = dstSubresource.baseArrayLayer;
        imageResolveRegion.dstSubresource.layerCount = dstSubresource.layerCount;
        imageResolveRegion.extent.width = srcTex->getDesc().width;
        imageResolveRegion.extent.height = srcTex->getDesc().height;
        imageResolveRegion.extent.depth = srcTex->getDesc().depth;

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                srcTex, { srcSubresource.mipLevel, 1, srcSubresource.baseArrayLayer, 1 }, ResourceStates::CopySource
            );
            m_StateTracker.requireTextureState(
                dstTex,
                { dstSubresource.mipLevel, 1, dstSubresource.baseArrayLayer, 1 },
                ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        vkCmdResolveImage(
            m_CurrentCommandBuffer->commandBuffer,
            srcTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            dstTex->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageResolveRegion
        );
    }

    void CommandList::clearColorTexture(ITexture *texture, const TextureSubresource &subresource, const Color &color) {
        Texture *tex = dynamic_cast<Texture *>(texture);

        m_CurrentCommandBuffer->referencedResources.push_back(tex);

        VkClearColorValue clearColorValue{};
        clearColorValue.float32[0] = color.r;
        clearColorValue.float32[1] = color.g;
        clearColorValue.float32[2] = color.b;
        clearColorValue.float32[3] = color.a;

        VkImageSubresourceRange imageSubresourceRange{};
        imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageSubresourceRange.baseMipLevel = subresource.mipLevel;
        imageSubresourceRange.levelCount = subresource.mipLevelCount;
        imageSubresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        imageSubresourceRange.layerCount = subresource.layerCount;

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                tex, { subresource.mipLevel, 1, subresource.baseArrayLayer, 1 }, ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        vkCmdClearColorImage(
            m_CurrentCommandBuffer->commandBuffer,
            tex->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            &clearColorValue,
            1,
            &imageSubresourceRange
        );
    }

    void CommandList::clearDepthStencilTexture(
        ITexture *texture, const TextureSubresource &subresource, bool clearDepth, bool clearStencil, float depthValue,
        uint32_t stencilValue
    ) {
        if (!clearDepth && !clearStencil) {
            return;
        }

        Texture *tex = dynamic_cast<Texture *>(texture);

        m_CurrentCommandBuffer->referencedResources.push_back(tex);

        VkClearDepthStencilValue clearDepthStencilValue{};
        clearDepthStencilValue.depth = depthValue;
        clearDepthStencilValue.stencil = stencilValue;

        VkImageAspectFlags aspectFlags = {};

        if (clearDepth) {
            aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        if (clearStencil) {
            aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        VkImageSubresourceRange imageSubresourceRange{};
        imageSubresourceRange.aspectMask = aspectFlags;
        imageSubresourceRange.baseMipLevel = subresource.mipLevel;
        imageSubresourceRange.levelCount = subresource.mipLevelCount;
        imageSubresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        imageSubresourceRange.layerCount = subresource.layerCount;

        if (m_EnableAutoBarriers) {
            m_StateTracker.requireTextureState(
                tex, { subresource.mipLevel, 1, subresource.baseArrayLayer, 1 }, ResourceStates::CopyDestination
            );
        }
        commitBarriers();

        vkCmdClearDepthStencilImage(
            m_CurrentCommandBuffer->commandBuffer,
            tex->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            &clearDepthStencilValue,
            1,
            &imageSubresourceRange
        );
    }

    void CommandList::clearAttachments(
        std::vector<ITexture *> colorAttachments, ITexture *depthAttachment, const std::vector<Rect> &rects
    ) {
        std::vector<VkClearAttachment> clearAttachments = {};

        VkClearColorValue clearColorValue{};
        clearColorValue.float32[0] = 0.0f;
        clearColorValue.float32[1] = 0.0f;
        clearColorValue.float32[2] = 0.0f;
        clearColorValue.float32[3] = 1.0f;

        for (ITexture *attachment : colorAttachments) {
            if (attachment) {
                VkClearAttachment clearAttachment = {};
                clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                clearAttachment.colorAttachment = 0;
                clearAttachment.clearValue.color = clearColorValue;
                clearAttachments.push_back(clearAttachment);
            }
        }

        if (depthAttachment) {
            VkClearDepthStencilValue clearDepthStencilValue{};
            clearDepthStencilValue.depth = 0.0f;
            clearDepthStencilValue.stencil = 0x00;

            VkClearAttachment clearAttachment = {};
            clearAttachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            clearAttachment.clearValue.depthStencil = clearDepthStencilValue;
            clearAttachments.push_back(clearAttachment);
        }

        const size_t rectsCount = rects.size();
        std::vector<VkClearRect> clearRects = {};
        clearRects.reserve(rectsCount);
        for (auto &rect : rects) {
            VkClearRect clearRect = {};
            clearRect.layerCount = 1;
            clearRect.rect.offset = { 0, 0 };
            clearRect.rect.extent = { static_cast<uint32_t>(rect.getWidth()), static_cast<uint32_t>(rect.getHeight()) };
            clearRects.push_back(clearRect);
        }

        vkCmdClearAttachments(
            m_CurrentCommandBuffer->commandBuffer,
            clearAttachments.size(),
            clearAttachments.data(),
            rectsCount,
            clearRects.data()
        );
    }

}
