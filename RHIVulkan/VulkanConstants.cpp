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

    static const ResourceStateMapping c_ResourceStateMappings[] = {
        {          ResourceStates::Common,VK_IMAGE_LAYOUT_UNDEFINED,                                                                          0,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT                                                                                                                                                       },

        {  ResourceStates::ShaderResource,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,                                                  VK_ACCESS_SHADER_READ_BIT,
         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT                                                                                                                                              },

        { ResourceStates::UnorderedAccess,
         VK_IMAGE_LAYOUT_GENERAL,                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT                                                                                                       },

        {    ResourceStates::RenderTarget,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT                                                                                                                                      },

        {      ResourceStates::DepthWrite,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT                                                                                                                                          },

        {       ResourceStates::DepthRead,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT                                                                                             },

        {      ResourceStates::CopySource,
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,                                                VK_ACCESS_TRANSFER_READ_BIT,
         VK_PIPELINE_STAGE_TRANSFER_BIT                                                                                                                                                     },

        { ResourceStates::CopyDestination,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,                                               VK_ACCESS_TRANSFER_WRITE_BIT,
         VK_PIPELINE_STAGE_TRANSFER_BIT                                                                                                                                                     },

        {         ResourceStates::Present, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                                                                          0, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT }
    };

    ResourceStateMapping convertResourceState(ResourceStates state) {
        if (state == ResourceStates::Unknown) {
            return { ResourceStates::Unknown, VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
        }

        for (const ResourceStateMapping &mapping : c_ResourceStateMappings) {
            if ((state & mapping.state) == mapping.state) {
                return mapping;
            }
        }

        return c_ResourceStateMappings[0];
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

    VkBlendFactor convertBlendFactor(const BlendFactor &blendState) {
        switch (blendState) {
        case BlendFactor::ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::ONE:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::ONE_MINUS_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::ONE_MINUS_DST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::ONE_MINUS_DST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::CONSTANT_COLOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        case BlendFactor::SRC_ALPHA_SATURATE:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendFactor::SRC1_COLOR:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendFactor::ONE_MINUS_SRC1_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendFactor::SRC1_ALPHA:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendFactor::ONE_MINUS_SRC1_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            return VK_BLEND_FACTOR_ZERO;
        }
    }

    VkBlendOp convertBlendOp(const BlendOp &blendOp) {
        switch (blendOp) {
        case BlendOp::ADD:
            return VK_BLEND_OP_ADD;
        case BlendOp::SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::REVERSE_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::MIN:
            return VK_BLEND_OP_MIN;
        case BlendOp::MAX:
            return VK_BLEND_OP_MAX;
        default:
            return VK_BLEND_OP_ADD;
        }
    }

    VkPolygonMode convertFillMode(const RasterizerFillMode &mode) {
        switch (mode) {
        case RasterizerFillMode::FILL:
            return VK_POLYGON_MODE_FILL;
        case RasterizerFillMode::LINE:
            return VK_POLYGON_MODE_LINE;
        case RasterizerFillMode::POINT:
            return VK_POLYGON_MODE_POINT;
        default:
            return VK_POLYGON_MODE_FILL;
        }
    }

    VkCullModeFlags convertCullMode(const RasterizerCullMode &mode) {
        switch (mode) {
        case RasterizerCullMode::None:
            return VK_CULL_MODE_NONE;
        case RasterizerCullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case RasterizerCullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        default:
            return VK_CULL_MODE_NONE;
        }
    }

    VkCompareOp convertCompareOp(const CompareOp &op) {
        switch (op) {
        case NEVER:
            return VK_COMPARE_OP_NEVER;
        case LESS:
            return VK_COMPARE_OP_LESS;
        case EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GREATER:
            return VK_COMPARE_OP_GREATER;
        case NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case GREATER_OR_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_ALWAYS;
        }
    }

    VkStencilOp convertStencilOp(const StencilOp &op) {
        switch (op) {
        case StencilOp::KEEP:
            return VK_STENCIL_OP_KEEP;
        case StencilOp::ZERO:
            return VK_STENCIL_OP_ZERO;
        case StencilOp::REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case StencilOp::INCR_CLAMP:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DECR_CLAMP:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::INVERT:
            return VK_STENCIL_OP_INVERT;
        case StencilOp::INCR_WRAP:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DECR_WRAP:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        }
        return VK_STENCIL_OP_KEEP;
    }

    VkStencilOpState convertStencilState(const DepthStencilState &depthStencilState, const DepthStencilState::StencilFaceState &stencilFaceState) {
        return VkStencilOpState{ .failOp = convertStencilOp(stencilFaceState.failOp),
                                 .passOp = convertStencilOp(stencilFaceState.passOp),
                                 .depthFailOp = convertStencilOp(stencilFaceState.depthFailOp),
                                 .compareOp = convertCompareOp(stencilFaceState.compareOp),
                                 .compareMask = depthStencilState.compareMask,
                                 .writeMask = depthStencilState.writeMask,
                                 .reference = depthStencilState.reference };
    }

    VkPipelineColorBlendAttachmentState convertBlendState(const ColorBlendState::RenderTargetBlendState &blendState) {
        return VkPipelineColorBlendAttachmentState{
            .blendEnable = blendState.blendEnable ? VK_TRUE : VK_FALSE,
            .srcColorBlendFactor = convertBlendFactor(blendState.srcColorBlendFactor),
            .dstColorBlendFactor = convertBlendFactor(blendState.dstColorBlendFactor),
            .colorBlendOp = convertBlendOp(blendState.colorBlendOp),
            .srcAlphaBlendFactor = convertBlendFactor(blendState.srcAlphaBlendFactor),
            .dstAlphaBlendFactor = convertBlendFactor(blendState.dstAlphaBlendFactor),
            .alphaBlendOp = convertBlendOp(blendState.alphaBlendOp),
            .colorWriteMask = static_cast<VkColorComponentFlags>(blendState.colorWriteMask)
        };
    }

}
