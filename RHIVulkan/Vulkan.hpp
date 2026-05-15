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
    struct NativeContext
    {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue computeQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        uint32_t graphicsQueueFamily = uint32_t(-1);
        uint32_t computeQueueFamily = uint32_t(-1);
        uint32_t transferQueueFamily = uint32_t(-1);

        std::vector<const char *> instanceExtensions;
        std::vector<const char *> layers;
        std::vector<const char *> deviceExtensions;
    };

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
