#pragma once

#include <RHICommon.hpp>

#define VK_NO_PROTOTYPES
#ifdef _WIN32
#include <Volk/volk.h>
#else
#include <volk.h>
#endif

namespace RHI::Vulkan
{

    class IDevice : public RHI::IDevice
    {
    public:
        // Additional Vulkan-specific public methods
        virtual VkSemaphore getQueueSemaphore(CommandQueue queueID) = 0;
        virtual void queueWaitForSemaphore(CommandQueue waitQueueID, VkSemaphore semaphore, uint64_t value) = 0;
        virtual void queueSignalSemaphore(CommandQueue executionQueueID, VkSemaphore semaphore, uint64_t value) = 0;

        virtual TextureHandle createTextureForNative(VkImage image, VkImageView imageView, ImageAspectFlagBits aspectFlags, const TextureDesc& desc) = 0;
    };

    typedef std::shared_ptr<IDevice> DeviceHandle;

    VkFormat convertFormat(RHI::Format format);

    VkSamplerAddressMode convertSamplerAddressMode(SamplerAddressMode mode);

    VkImageLayout convertImageLayout(ImageLayout imageLayout);

    VkMemoryPropertyFlags pickMemoryProperties(const MemoryPropertiesBits& memoryProperties);

    VkDescriptorType convertDescriptorType(DescriptorType type);

    VkShaderStageFlags pickShaderStage(ShaderStageFlagBits stages);

    VkBlendFactor convertBlendFactor(const BlendFactor& blendState);

    VkBlendOp convertBlendOp(const BlendOp& blendOp);

    VkPolygonMode convertFillMode(const FillMode& mode);

    VkCompareOp convertCompareOp(const CompareOp& op);

    VkStencilOp convertStencilOp(const StencilOp &op);

    VkStencilOpState convertStencilState(
            const DepthStencilState &depthStencilState, const DepthStencilState::StencilFaceState &stencilFaceState
        );

    VkPipelineColorBlendAttachmentState convertBlendState(const ColorBlendState::RenderTargetBlendState &blendState);

}
