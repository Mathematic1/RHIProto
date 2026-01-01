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

        virtual TextureHandle createTextureForNative(VkImage image, VkImageView imageView, const TextureDesc& desc) = 0;
    };

    typedef std::shared_ptr<IDevice> DeviceHandle;

    VkFormat convertFormat(RHI::Format format);
}
