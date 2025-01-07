#include <cassert>
#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    struct FormatMapping
    {
        RHI::Format rhiFormat;
        VkFormat vkFormat;
    };

    static const std::array<FormatMapping, size_t(Format::COUNT)> c_FormatMap = { {
        { Format::UNKNOWN,           VK_FORMAT_UNDEFINED                },
        { Format::R8_UINT,           VK_FORMAT_R8_UINT                  },
        { Format::R8_SINT,           VK_FORMAT_R8_SINT                  },
        { Format::R8_UNORM,          VK_FORMAT_R8_UNORM                 },
        { Format::R8_SNORM,          VK_FORMAT_R8_SNORM                 },
        { Format::RG8_UINT,          VK_FORMAT_R8G8_UINT                },
        { Format::RG8_SINT,          VK_FORMAT_R8G8_SINT                },
        { Format::RG8_UNORM,         VK_FORMAT_R8G8_UNORM               },
        { Format::RG8_SNORM,         VK_FORMAT_R8G8_SNORM               },
        { Format::R16_UINT,          VK_FORMAT_R16_UINT                 },
        { Format::R16_SINT,          VK_FORMAT_R16_SINT                 },
        { Format::R16_UNORM,         VK_FORMAT_R16_UNORM                },
        { Format::R16_SNORM,         VK_FORMAT_R16_SNORM                },
        { Format::R16_FLOAT,         VK_FORMAT_R16_SFLOAT               },
        { Format::BGRA4_UNORM,       VK_FORMAT_B4G4R4A4_UNORM_PACK16    },
        { Format::B5G6R5_UNORM,      VK_FORMAT_B5G6R5_UNORM_PACK16      },
        { Format::B5G5R5A1_UNORM,    VK_FORMAT_B5G5R5A1_UNORM_PACK16    },
        { Format::RGBA8_UINT,        VK_FORMAT_R8G8B8A8_UINT            },
        { Format::RGBA8_SINT,        VK_FORMAT_R8G8B8A8_SINT            },
        { Format::RGBA8_UNORM,       VK_FORMAT_R8G8B8A8_UNORM           },
        { Format::RGBA8_SNORM,       VK_FORMAT_R8G8B8A8_SNORM           },
        { Format::BGRA8_UNORM,       VK_FORMAT_B8G8R8A8_UNORM           },
        { Format::SRGBA8_UNORM,      VK_FORMAT_R8G8B8A8_SRGB            },
        { Format::SBGRA8_UNORM,      VK_FORMAT_B8G8R8A8_SRGB            },
        { Format::R10G10B10A2_UNORM, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
        { Format::R11G11B10_FLOAT,   VK_FORMAT_B10G11R11_UFLOAT_PACK32  },
        { Format::RG16_UINT,         VK_FORMAT_R16G16_UINT              },
        { Format::RG16_SINT,         VK_FORMAT_R16G16_SINT              },
        { Format::RG16_UNORM,        VK_FORMAT_R16G16_UNORM             },
        { Format::RG16_SNORM,        VK_FORMAT_R16G16_SNORM             },
        { Format::RG16_FLOAT,        VK_FORMAT_R16G16_SFLOAT            },
        { Format::R32_UINT,          VK_FORMAT_R32_UINT                 },
        { Format::R32_SINT,          VK_FORMAT_R32_SINT                 },
        { Format::R32_FLOAT,         VK_FORMAT_R32_SFLOAT               },
        { Format::RGBA16_UINT,       VK_FORMAT_R16G16B16A16_UINT        },
        { Format::RGBA16_SINT,       VK_FORMAT_R16G16B16A16_SINT        },
        { Format::RGBA16_FLOAT,      VK_FORMAT_R16G16B16A16_SFLOAT      },
        { Format::RGBA16_UNORM,      VK_FORMAT_R16G16B16A16_UNORM       },
        { Format::RGBA16_SNORM,      VK_FORMAT_R16G16B16A16_SNORM       },
        { Format::RG32_UINT,         VK_FORMAT_R32G32_UINT              },
        { Format::RG32_SINT,         VK_FORMAT_R32G32_SINT              },
        { Format::RG32_FLOAT,        VK_FORMAT_R32G32_SFLOAT            },
        { Format::RGB32_UINT,        VK_FORMAT_R32G32B32_UINT           },
        { Format::RGB32_SINT,        VK_FORMAT_R32G32B32_SINT           },
        { Format::RGB32_FLOAT,       VK_FORMAT_R32G32B32_SFLOAT         },
        { Format::RGBA32_UINT,       VK_FORMAT_R32G32B32A32_UINT        },
        { Format::RGBA32_SINT,       VK_FORMAT_R32G32B32A32_SINT        },
        { Format::RGBA32_FLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT      },
        { Format::D16,               VK_FORMAT_D16_UNORM                },
        { Format::D24S8,             VK_FORMAT_D24_UNORM_S8_UINT        },
        { Format::X24G8_UINT,        VK_FORMAT_D24_UNORM_S8_UINT        },
        { Format::D32,               VK_FORMAT_D32_SFLOAT               },
        { Format::D32S8,             VK_FORMAT_D32_SFLOAT_S8_UINT       },
        { Format::X32G8_UINT,        VK_FORMAT_D32_SFLOAT_S8_UINT       },
        { Format::BC1_UNORM,         VK_FORMAT_BC1_RGBA_UNORM_BLOCK     },
        { Format::BC1_UNORM_SRGB,    VK_FORMAT_BC1_RGBA_SRGB_BLOCK      },
        { Format::BC2_UNORM,         VK_FORMAT_BC2_UNORM_BLOCK          },
        { Format::BC2_UNORM_SRGB,    VK_FORMAT_BC2_SRGB_BLOCK           },
        { Format::BC3_UNORM,         VK_FORMAT_BC3_UNORM_BLOCK          },
        { Format::BC3_UNORM_SRGB,    VK_FORMAT_BC3_SRGB_BLOCK           },
        { Format::BC4_UNORM,         VK_FORMAT_BC4_UNORM_BLOCK          },
        { Format::BC4_SNORM,         VK_FORMAT_BC4_SNORM_BLOCK          },
        { Format::BC5_UNORM,         VK_FORMAT_BC5_UNORM_BLOCK          },
        { Format::BC5_SNORM,         VK_FORMAT_BC5_SNORM_BLOCK          },
        { Format::BC6H_UFLOAT,       VK_FORMAT_BC6H_UFLOAT_BLOCK        },
        { Format::BC6H_SFLOAT,       VK_FORMAT_BC6H_SFLOAT_BLOCK        },
        { Format::BC7_UNORM,         VK_FORMAT_BC7_UNORM_BLOCK          },
        { Format::BC7_UNORM_SRGB,    VK_FORMAT_BC7_SRGB_BLOCK           },

    } };

    VkFormat convertFormat(RHI::Format format)
    {
        assert(format < RHI::Format::COUNT);
        assert(c_FormatMap[uint32_t(format)].rhiFormat == format);

        return c_FormatMap[uint32_t(format)].vkFormat;
    }

    VkSamplerAddressMode convertSamplerAddressMode(SamplerAddressMode mode)
    {
	    switch (mode)
	    {
	    case SamplerAddressMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    case SamplerAddressMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	    case SamplerAddressMode::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	    case SamplerAddressMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	    case SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	    }
    }

    struct ImageLayoutMapping
    {
        RHI::ImageLayout rhiImageLayout;
        VkImageLayout vkImageLayout;
    };

    static const std::array<ImageLayoutMapping, size_t(ImageLayout::COUNT)> c_ImageLayoutMapping =
    {
    {
        {ImageLayout::UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED},
        {ImageLayout::GENERAL, VK_IMAGE_LAYOUT_GENERAL},
        {ImageLayout::COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        {ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
        {ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL},
        {ImageLayout::SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
        {ImageLayout::TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
        {ImageLayout::TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
        {ImageLayout::PREINITIALIZED, VK_IMAGE_LAYOUT_PREINITIALIZED},
        {ImageLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL},
        {ImageLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL},
        {ImageLayout::DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL},
        {ImageLayout::DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL},
        {ImageLayout::STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL},
        {ImageLayout::STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL},
        {ImageLayout::READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL},
        {ImageLayout::ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL},
        }
    };

    VkImageLayout convertImageLayout(ImageLayout imageLayout)
    {
        assert(imageLayout < RHI::ImageLayout::COUNT);
        assert(c_ImageLayoutMapping[uint32_t(imageLayout)].rhiImageLayout == imageLayout);

        return c_ImageLayoutMapping[uint32_t(imageLayout)].vkImageLayout;
    }

    VkMemoryPropertyFlags pickMemoryProperties(const MemoryPropertiesBits& memoryProperties)
    {
        VkMemoryPropertyFlags ret = 0;

        if ((memoryProperties & MemoryPropertiesBits::DEVICE_LOCAL_BIT) != 0)
        {
            ret |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }
        if ((memoryProperties & MemoryPropertiesBits::HOST_VISIBLE_BIT) != 0)
        {
            ret |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        }
        if ((memoryProperties & MemoryPropertiesBits::HOST_CACHED_BIT) != 0)
        {
            ret |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        if ((memoryProperties & MemoryPropertiesBits::HOST_COHERENT_BIT) != 0)
        {
            ret |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        }

        return ret;
    }

    VkDescriptorType convertDescriptorType(DescriptorType type)
    {
        assert(type < RHI::DescriptorType::MAX_ENUM);

        switch (type)
        {
        case DescriptorType::SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SAMPLED_IMAGE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::STORAGE_IMAGE: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UNIFORM_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::STORAGE_TEXEL_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::UNIFORM_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::STORAGE_BUFFER_DYNAMIC: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::INPUT_ATTACHMENT: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        default:
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    VkShaderStageFlags pickShaderStage(ShaderStageFlagBits stages)
    {
        assert(stages < RHI::ShaderStageFlagBits::MAX_ENUM);

        VkShaderStageFlags ret = 0;

        if((stages & ShaderStageFlagBits::VERTEX_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if ((stages & ShaderStageFlagBits::TESSELLATION_CONTROL_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }
        if ((stages & ShaderStageFlagBits::TESSELLATION_EVALUATION_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }
        if ((stages & ShaderStageFlagBits::GEOMETRY_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_GEOMETRY_BIT;
        }
        if ((stages & ShaderStageFlagBits::FRAGMENT_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        if ((stages & ShaderStageFlagBits::COMPUTE_BIT) != 0)
        {
            ret |= VK_SHADER_STAGE_COMPUTE_BIT;
        }
        if ((stages & ShaderStageFlagBits::ALL_GRAPHICS) != 0)
        {
            ret |= VK_SHADER_STAGE_ALL_GRAPHICS;
        }
        if ((stages & ShaderStageFlagBits::ALL) != 0)
        {
            ret |= VK_SHADER_STAGE_ALL;
        }

        return ret;
    }

    VkBlendFactor convertBlendFactor(const BlendState& blendState)
    {
	    switch (blendState)
	    {
        case BlendState::ZERO: return VK_BLEND_FACTOR_ZERO;
        case BlendState::ONE: return VK_BLEND_FACTOR_ONE;
        case BlendState::SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendState::ONE_MINUS_SRC_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendState::DST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
        case BlendState::ONE_MINUS_DST_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendState::SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendState::ONE_MINUS_SRC_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendState::DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendState::ONE_MINUS_DST_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendState::CONSTANT_COLOR: return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendState::ONE_MINUS_CONSTANT_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendState::CONSTANT_ALPHA: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendState::ONE_MINUS_CONSTANT_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendState::SRC_ALPHA_SATURATE: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendState::SRC1_COLOR: return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendState::ONE_MINUS_SRC1_COLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendState::SRC1_ALPHA: return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendState::ONE_MINUS_SRC1_ALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	    default:
	    	return VK_BLEND_FACTOR_ZERO;
	    }
    }

    VkBlendOp convertBlendOp(const BlendOp& blendOp)
    {
	    switch (blendOp)
	    {
        case BlendOp::ADD: return VK_BLEND_OP_ADD;
        case BlendOp::SUBTRACT: return VK_BLEND_OP_SUBTRACT;
        case BlendOp::REVERSE_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::MIN: return VK_BLEND_OP_MIN;
        case BlendOp::MAX: return VK_BLEND_OP_MAX;
	    default:
	    	return VK_BLEND_OP_ADD;
	    }
    }

    VkPolygonMode convertFillMode(const FillMode& mode)
    {
	    switch (mode)
	    {
        case FillMode::FILL: return VK_POLYGON_MODE_FILL;
        case FillMode::LINE: return VK_POLYGON_MODE_LINE;
        case FillMode::POINT: return VK_POLYGON_MODE_POINT;
        default:
            return VK_POLYGON_MODE_FILL;
	    }
    }
}
