#include <RHICommon.hpp>

namespace RHI {
TextureSubresource TextureSubresource::ResolveTextureSubresource(const TextureDesc &desc) const {
    TextureSubresource ret = *this;

    if (ret.mipLevel >= desc.mipLevels) {
        ret.mipLevel = 0;
    }

    if (ret.mipLevelCount == 0 || ret.mipLevelCount == std::numeric_limits<uint32_t>::max()) {
        uint32_t lastMipPlusOne = std::min(ret.mipLevel + desc.mipLevels, desc.mipLevels);

        ret.mipLevelCount = lastMipPlusOne - ret.mipLevel;
    } else {
        uint32_t lastMipPlusOne = std::min(ret.mipLevel + ret.mipLevelCount, desc.mipLevels);

        ret.mipLevelCount = lastMipPlusOne - ret.mipLevel;
    }

    switch (desc.dimension)
    {
    case TextureDimension::Texture1DArray:
    case TextureDimension::Texture2DArray:
    case TextureDimension::TextureCube:
    case TextureDimension::TextureCubeArray:
    case TextureDimension::Texture2DMSArray: {
        if (ret.baseArrayLayer >= desc.layerCount) {
            ret.baseArrayLayer = 0;
        }

        if (ret.layerCount == 0 || ret.layerCount == std::numeric_limits<uint32_t>::max()) {
            uint32_t lastLayerPlusOne = std::min(ret.baseArrayLayer + desc.layerCount, desc.layerCount);

            ret.layerCount = lastLayerPlusOne - ret.baseArrayLayer;
        } else {
            uint32_t lastLayerPlusOne = std::min(ret.baseArrayLayer + ret.layerCount, desc.layerCount);

            ret.layerCount = lastLayerPlusOne - ret.baseArrayLayer;
        }
        break;
    }

    default:
        ret.baseArrayLayer = 0;
        ret.layerCount = 1;
        break;
    }

    return ret;
}
}
