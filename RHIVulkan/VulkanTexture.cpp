#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    Texture::~Texture()
    {
        vkDestroyImageView(m_Context.device, imageView, nullptr);
        vkDestroyImage(m_Context.device, image, nullptr);
        vkFreeMemory(m_Context.device, imageMemory, nullptr);
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

        const VkFormat format = convertFormat(desc.format);

        if (desc.usage.isShaderResource)
            ret |= VK_IMAGE_USAGE_SAMPLED_BIT;

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

    static VkImageAspectFlags pickImageAspect(const ImageAspectFlagBits& imageAspect)
    {
        VkImageAspectFlags ret = VK_IMAGE_ASPECT_NONE;

        if((imageAspect & ImageAspectFlagBits::COLOR_BIT) != 0)
        {
            ret |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        if ((imageAspect & ImageAspectFlagBits::DEPTH_BIT) != 0)
        {
            ret |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if ((imageAspect & ImageAspectFlagBits::STENCIL_BIT) != 0)
        {
            ret |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }

        return ret;
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
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = convertFormat(desc.format);
        imageInfo.extent = VkExtent3D{ desc.width, desc.height, desc.depth };
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = (uint32_t)((pickImageFlag(desc) == VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ? 6 : 1);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = desc.isLinearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = pickImageUsage(desc);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = nullptr;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(m_Context.device, &imageInfo, nullptr, &tex->image));

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Context.device, tex->image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, pickMemoryProperties(desc.memoryProperties));

        VK_CHECK(vkAllocateMemory(m_Context.device, &allocInfo, nullptr, &tex->imageMemory));

        vkBindImageMemory(m_Context.device, tex->image, tex->imageMemory, 0);
        return TextureHandle(tex);
    }

    bool Device::createImageView(ITexture* texture, ImageAspectFlagBits aspectFlags)
    {
        Texture* tex = dynamic_cast<Texture*>(texture);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = nullptr;
        viewInfo.flags = 0;
        viewInfo.image = tex->image;
        viewInfo.viewType = textureDimensionToImageViewType(tex->desc.dimension);
        viewInfo.format = convertFormat(tex->desc.format);
        viewInfo.subresourceRange.aspectMask = pickImageAspect(aspectFlags);
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = tex->desc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = tex->desc.layerCount;

        return (vkCreateImageView(m_Context.device, &viewInfo, nullptr, &tex->imageView) == VK_SUCCESS);
    }

    ISampler* Device::createTextureSampler(const SamplerDesc& desc)
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
        samplerInfo.maxLod = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        Sampler* sampler = new Sampler(m_Context);

        if(vkCreateSampler(m_Context.device, &samplerInfo, nullptr, &sampler->sampler) != VK_SUCCESS)
        {
            printf("Cannot create texture sampler\n");
            exit(EXIT_FAILURE);
        }

        return sampler;
    }

    ISampler* Device::createDepthSampler()
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

        return sampler;
    }

    bool CommandList::updateTextureImage(ITexture* texture, const void* imageData, ImageLayout sourceImageLayout)
    {
    	Texture* tex = dynamic_cast<Texture*>(texture);

        uint32_t bytesPerPixel = bytesPerTexFormat(tex->desc.format);

        VkDeviceSize layerSize = tex->desc.width * tex->desc.height * bytesPerPixel;
        VkDeviceSize imageSize = layerSize * tex->desc.layerCount;

        BufferDesc stagingDesc = BufferDesc{}
            .setSize(imageSize)
            .setIsTransferSrc(true)
            .setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT);
        BufferHandle stagingBuffer = m_Device->createBuffer(stagingDesc);
        m_CurrentCommandBuffer->referencedStagingBuffers.push_back(stagingBuffer);

        m_Device->uploadBufferData(stagingBuffer.get(), 0, imageData, imageSize);

        transitionImageLayout(tex, sourceImageLayout, ImageLayout::TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer.get(), tex);
        transitionImageLayout(tex, ImageLayout::TRANSFER_DST_OPTIMAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);

        // TODO: fixup memory cleanup
        //delete stagingBuffer;

        return true;
    }

    TextureHandle Device::createTextureForNative(VkImage image, VkImageView imageView, ImageAspectFlagBits aspectFlags, const TextureDesc& desc)
    {
        Texture* tex = new Texture(m_Context);
        tex->desc = desc;
        tex->image = image;
        createImageView(tex, aspectFlags);

        return TextureHandle(tex);
    }
}
