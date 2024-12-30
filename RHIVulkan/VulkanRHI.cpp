#include <VulkanBackend.hpp>

#include <unordered_set>

#ifdef NDEBUG
const bool enableValidationLayers = false;
const bool enableValidationFeaturesEnabled = false;
const bool enableValidationFeaturesDisabled = false;
#else
const bool enableValidationLayers = true;
const bool enableValidationFeaturesEnabled = false;
const bool enableValidationFeaturesDisabled = true;
#endif

namespace RHI::Vulkan
{
    VulkanRHIModule::VulkanRHIModule()
	    : IRHIModule()
    {
        glslang_initialize_process();

        volkInitialize();
    }

    VulkanRHIModule::~VulkanRHIModule()
    {
	    
    }

	IDynamicRHI* VulkanRHIModule::createRHI(const DeviceParams& deviceParams)
	{
		VulkanDynamicRHI* VulkanRHI = new VulkanDynamicRHI(deviceParams);
		return VulkanRHI;
	}

	VulkanDynamicRHI::VulkanDynamicRHI(const DeviceParams& deviceParams)
		: IDynamicRHI(deviceParams)
	{
		createInstance();

        if (!setupDebugCallbacks(m_VulkanInstance.instance, &m_VulkanInstance.messenger, &m_VulkanInstance.reportCallback))
        {
            printf("Cannot initialize debug callbacks\n");
            exit(EXIT_FAILURE);
        }
	}

    RHI::IDevice* VulkanDynamicRHI::getDevice() const
    {
        return m_Device;
    }

    void VulkanDynamicRHI::setWindowSurface(VkSurfaceKHR surface)
    {
        m_VulkanInstance.surface = surface;
    }

	VulkanDynamicRHI::~VulkanDynamicRHI()
	{
        destroyVulkanInstance();
	}

	bool IsExtensionAvailable(const std::vector<VkExtensionProperties>& properties,
		const char* extension) noexcept {
		for (const VkExtensionProperties& p : properties) {
			if (strcmp(p.extensionName, extension) == 0) {
				return true;
			}
		}
		return false;
	}

    VulkanContextExtensions& VulkanDynamicRHI::initializeContextExtensions()
    {
        VulkanContextExtensions contextExtensions{
            .KHR_swapchain = true,
            .KHR_maintenance3 = true,
            .EXT_discriptor_indexing = true,
            .EXT_draw_indirect_count = true,
#if defined(__APPLE__)
            .KHR_portability_subset = true
#endif
        };
        return contextExtensions;
    }

    VulkanContextFeatures& VulkanDynamicRHI::initializeContextFeatures()
    {
        VulkanContextFeatures contextFeatures{
            .supportsScreenshots_ = true,
            .geometryShader_ = true,
            .tessellationShader_ = true,
            .multiDrawIndirect = true,
            .drawIndirectFirstInstance = true,
            .vertexPipelineStoresAndAtomics_ = true,
            .fragmentStoresAndAtomics_ = true,
            .shaderSampledImageArrayDynamicIndexing = true,
            .shaderInt64 = true,
            .deviceDescriptorIndexing = true
        };
        return contextFeatures;
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        const bool isDiscreteGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        const bool isIntegratedGPU = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        const bool isGPU = isDiscreteGPU || isIntegratedGPU;

#if defined(WIN32)
        return isGPU && deviceFeatures.geometryShader;
#elif defined(__APPLE__)
        return isGPU;
#endif
        return false;
    }

    void VulkanDynamicRHI::createDeviceInternal()
    {
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        VkPhysicalDeviceFeatures deviceFeatures = initVulkanRenderDeviceFeatures(m_VulkanFeatures, deviceFeatures2);

        VK_CHECK(findSuitablePhysicalDevice(m_VulkanInstance.instance, isDeviceSuitable, &m_VulkanPhysicalDevice));

        std::unordered_set<uint32_t> uniqueQueueFamilies{};
        if (m_DeviceParams.useGraphicsQueue)
        {
            m_GraphicsQueueFamily = findQueueFamilies(m_VulkanPhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
            uniqueQueueFamilies.insert(m_GraphicsQueueFamily);
        }
        //	VK_CHECK(createDevice2(m_Context.m_PhysicalDevice, deviceFeatures2, vkDev.graphicsFamily, &m_Context.m_Device));
        //	VK_CHECK(vkGetBestComputeQueue(m_Context.m_PhysicalDevice, &vkDev.computeFamily));
        if (m_DeviceParams.useComputeQueue)
        {
            m_ComputeQueueFamily = findQueueFamilies(m_VulkanPhysicalDevice, VK_QUEUE_COMPUTE_BIT);
            uniqueQueueFamilies.insert(m_ComputeQueueFamily);
        }

        if (m_DeviceParams.useTransferQueue)
        {
            m_TransferQueueFamily = findQueueFamilies(m_VulkanPhysicalDevice, VK_QUEUE_TRANSFER_BIT);
            uniqueQueueFamilies.insert(m_TransferQueueFamily);
        }

        VkBool32 presentSupported = 0;
        {
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(m_VulkanPhysicalDevice, &familyCount, nullptr);

            for (uint32_t i = 0; i != familyCount; i++)
            {
                vkGetPhysicalDeviceSurfaceSupportKHR(m_VulkanPhysicalDevice, i, m_VulkanInstance.surface, &presentSupported);
                if (presentSupported)
                {
                    m_PresentQueueFamily = i;
                    break;
                }
            }
        }

        if (!presentSupported)
        {
            exit(EXIT_FAILURE);
        }

        if(m_DeviceParams.usePresentQueue && presentSupported)
        {
            uniqueQueueFamilies.insert(m_PresentQueueFamily);
        }

    	VK_CHECK(createDevice(uniqueQueueFamilies, deviceFeatures, deviceFeatures2));

        if (m_DeviceParams.useGraphicsQueue)
        {
            vkGetDeviceQueue(m_VulkanDevice, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
        }
        if (m_GraphicsQueue == nullptr)
        {
            exit(EXIT_FAILURE);
        }

        if (m_DeviceParams.useComputeQueue)
        {
            vkGetDeviceQueue(m_VulkanDevice, m_ComputeQueueFamily, 0, &m_ComputeQueue);
            if (m_ComputeQueue == nullptr)
            {
                exit(EXIT_FAILURE);
            }
        }

        if (m_DeviceParams.usePresentQueue)
        {
            vkGetDeviceQueue(m_VulkanDevice, m_PresentQueueFamily, 0, &m_PresentQueue);
            if (m_PresentQueue == nullptr)
            {
                exit(EXIT_FAILURE);
            }
        }

        RHI::Vulkan::DeviceDesc DeviceDesc = {
            .framebufferWidth = m_DeviceParams.backBufferWidth,
            .framebufferHeight = m_DeviceParams.backBufferHeight,
            .instance = m_VulkanInstance.instance,
            .physicalDevice = m_VulkanPhysicalDevice,
            .device = m_VulkanDevice,
            .ctxExtensions = &m_VulkanExtensions,
            .ctxFeatures = &m_VulkanFeatures,
            .graphicsFamily = m_GraphicsQueueFamily,
            .graphicsQueue = m_GraphicsQueue,
            .useGraphicsQueue = m_DeviceParams.useGraphicsQueue,
            .computeFamily = m_ComputeQueueFamily,
            .computeQueue = m_ComputeQueue,
            .useComputeQueue = m_DeviceParams.useComputeQueue,
            .transferFamily = m_TransferQueueFamily,
            .transferQueue = m_TransferQueue,
            .useTransferQueue = m_DeviceParams.useTransferQueue };

        m_Device = new RHI::Vulkan::Device(DeviceDesc);

        createSwapchain();
        const size_t imageCount = createSwapchainImages();
        m_SwapChainIndex = 0;



        m_PresentSemaphores.reserve(m_DeviceParams.maxFramesInFlight + 1);
        m_AcquireSemaphores.reserve(m_DeviceParams.maxFramesInFlight + 1);
        const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        for (uint32_t i = 0; i < m_DeviceParams.maxFramesInFlight + 1; ++i)
        {
            VkSemaphore presentSemaphore;
            VkSemaphore acquireSemaphore;
            vkCreateSemaphore(m_VulkanDevice, &semaphoreCreateInfo, nullptr, &presentSemaphore);
            vkCreateSemaphore(m_VulkanDevice, &semaphoreCreateInfo, nullptr, &acquireSemaphore);
            m_PresentSemaphores.push_back(presentSemaphore);
            m_AcquireSemaphores.push_back(acquireSemaphore);
        }

        BackBufferResized();
    }

    VkResult VulkanDynamicRHI::createDevice(std::unordered_set<uint32_t>& uniqueQueueFamilies, VkPhysicalDeviceFeatures deviceFeatures, VkPhysicalDeviceFeatures2 deviceFeatures2)
    {
        std::vector<const char*> extensions{};
        if (m_VulkanExtensions.KHR_swapchain)
        {
            extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }
        if (m_VulkanExtensions.KHR_maintenance3)
        {
            extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        }
        if (m_VulkanExtensions.EXT_discriptor_indexing)
        {
            extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        }
        if (m_VulkanExtensions.EXT_draw_indirect_count)
        {
            // for legacy drivers Vulkan 1.1
            extensions.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
        }
#if defined (__APPLE__)
        if (ctx_.ctxExtensions.KHR_portability_subset)
        {
            // for legacy drivers Vulkan 1.1
            extensions.push_back("VK_KHR_portability_subset");
        }
#endif

        if (m_GraphicsQueueFamily == m_ComputeQueueFamily)
        {
            m_DeviceParams.useComputeQueue = false;
        }

        const float queuePriorities[2] = { 0.f, 0.f };

        std::vector<VkDeviceQueueCreateInfo> qci{};

        for(uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo qciGfx{};
            qciGfx.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qciGfx.pNext = nullptr;
            qciGfx.flags = 0;
            qciGfx.queueFamilyIndex = queueFamily;
            qciGfx.queueCount = 1;
            qciGfx.pQueuePriorities = &queuePriorities[0];
            qci.push_back(qciGfx);
        }

        VkDeviceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.pNext = m_VulkanFeatures.deviceDescriptorIndexing ? &deviceFeatures2 : nullptr;
        ci.flags = 0;
        ci.queueCreateInfoCount = static_cast<uint32_t>(qci.size());
        ci.pQueueCreateInfos = qci.data();
        ci.enabledLayerCount = 0;
        ci.ppEnabledLayerNames = nullptr;
        ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        ci.ppEnabledExtensionNames = extensions.data();
        ci.pEnabledFeatures = m_VulkanFeatures.deviceDescriptorIndexing ? nullptr : &deviceFeatures;

        return vkCreateDevice(m_VulkanPhysicalDevice, &ci, nullptr, &m_VulkanDevice);
    }

    GraphicsAPI VulkanDynamicRHI::getGraphicsAPI() const
    {
        return GraphicsAPI::VULKAN;
    }

    void VulkanDynamicRHI::destroySwapChain()
    {
        for (size_t i = 0; i < m_SwapchainImages.size(); i++)
        {
            vkDestroyImageView(m_VulkanDevice, m_SwapchainImageViews[i], nullptr);
        }
        m_SwapchainImageViews.clear();

        if(m_SwapChain)
        {
            vkDestroySwapchainKHR(m_VulkanDevice, m_SwapChain, nullptr);
            m_SwapChain = nullptr;
        }
    }

    uint32_t VulkanDynamicRHI::GetBackBufferCount()
    {
        return uint32_t(m_SwapchainImages.size());
    }

    uint32_t VulkanDynamicRHI::GetCurrentBackBufferIndex()
    {
        return m_SwapChainIndex;
    }

    ITexture* VulkanDynamicRHI::GetBackBuffer(uint32_t index)
    {
        return m_SwapchainTextures[index];
    }

    ITexture* VulkanDynamicRHI::GetDepthBuffer()
    {
        return m_DepthSwapChainTexture;
    }

    IFramebuffer* VulkanDynamicRHI::GetFramebuffer(uint32_t index)
    {
        return m_SwapChainFramebuffers[index];
    }

    bool VulkanDynamicRHI::createSwapchain()
    {
        destroySwapChain();

        auto swapchainSupport = querySwapchainSupport(m_VulkanPhysicalDevice, m_VulkanInstance.surface);
        auto surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
        auto presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);

        const VkSwapchainCreateInfoKHR ci = {
                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                nullptr,
                0,
                m_VulkanInstance.surface,
                chooseSwapImageCount(swapchainSupport.capabilities),
                surfaceFormat.format,
                surfaceFormat.colorSpace,
                {m_DeviceParams.backBufferWidth, m_DeviceParams.backBufferHeight},
                1,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | (m_DeviceParams.supportScreenshots ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0u),
                VK_SHARING_MODE_EXCLUSIVE,
                1,
                &m_GraphicsQueueFamily,
                swapchainSupport.capabilities.currentTransform,
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                presentMode,
                VK_TRUE,
                VK_NULL_HANDLE };

        VK_CHECK(vkCreateSwapchainKHR(m_VulkanDevice, &ci, nullptr, &m_SwapChain));

        return true;
    }

    size_t VulkanDynamicRHI::createSwapchainImages()
    {
        uint32_t imageCount = 0;
        VK_CHECK(vkGetSwapchainImagesKHR(m_VulkanDevice, m_SwapChain, &imageCount, nullptr));

        m_SwapchainImages.resize(imageCount);
        m_SwapchainImageViews.resize(imageCount);

        VK_CHECK(vkGetSwapchainImagesKHR(m_VulkanDevice, m_SwapChain, &imageCount, m_SwapchainImages.data()));

        Vulkan::Device* device = dynamic_cast<Vulkan::Device*>(m_Device);
        for (unsigned i = 0; i < imageCount; i++)
        {
            TextureDesc desc = {};
            desc.setWidth(m_DeviceParams.backBufferWidth)
                .setHeight(m_DeviceParams.backBufferHeight)
                .setFormat(Format::BGRA8_UNORM);
            ITexture* texture = device->createTextureForNative(m_SwapchainImages[i], m_SwapchainImageViews[i], ImageAspectFlagBits::COLOR_BIT, desc);

            m_SwapchainTextures.push_back(texture);
        }

        createDepthSwapchainImage();

        return static_cast<size_t>(imageCount);
    }

    void VulkanDynamicRHI::createDepthSwapchainImage()
    {
        IRHICommandList* commandList = m_Device->createCommandList();
        commandList->beginSingleTimeCommands();

        {
            TextureDesc desc = {};
            desc.setWidth(m_DeviceParams.backBufferWidth)
                .setHeight(m_DeviceParams.backBufferHeight)
                .setFormat(m_Device->findDepthFormat())
                .setIsShaderResource(true)
                .setIsRenderTarget(true);

            m_DepthSwapChainTexture = m_Device->createImage(desc);

            if (!m_DepthSwapChainTexture)
            {
                printf("Cannot create depth texture\n");
                exit(EXIT_FAILURE);
            }

            m_Device->createImageView(m_DepthSwapChainTexture, ImageAspectFlagBits::DEPTH_BIT);
            commandList->transitionImageLayout(m_DepthSwapChainTexture, ImageLayout::UNDEFINED, ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }

        commandList->endSingleTimeCommands();
        std::vector<IRHICommandList*> commandLists;
        commandLists.push_back(commandList);
        m_Device->executeCommandLists(commandLists, 1);
    }

    void VulkanDynamicRHI::resizeSwapchain()
    {
        if (m_VulkanDevice)
        {
            destroySwapChain();
            createSwapchain();
        }
    }

    VkPhysicalDeviceFeatures VulkanDynamicRHI::initVulkanRenderDeviceFeatures(const VulkanContextFeatures& ctxFeatures, VkPhysicalDeviceFeatures2& deviceFeatures2)
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        /* for wireframe outlines */
        deviceFeatures.geometryShader = (VkBool32)(ctxFeatures.geometryShader_ ? VK_TRUE : VK_FALSE);
        /* for tesselation experiments */
        deviceFeatures.tessellationShader = (VkBool32)(ctxFeatures.tessellationShader_ ? VK_TRUE : VK_FALSE);
        /* for indirect instanced rendering */
        deviceFeatures.multiDrawIndirect = (VkBool32)(ctxFeatures.multiDrawIndirect ? VK_TRUE : VK_FALSE);
        deviceFeatures.drawIndirectFirstInstance = (VkBool32)(ctxFeatures.drawIndirectFirstInstance ? VK_TRUE : VK_FALSE);
        /* for OIT and general atomic operations */
        deviceFeatures.vertexPipelineStoresAndAtomics = (VkBool32)(ctxFeatures.vertexPipelineStoresAndAtomics_ ? VK_TRUE : VK_FALSE);
        deviceFeatures.fragmentStoresAndAtomics = (VkBool32)(ctxFeatures.fragmentStoresAndAtomics_ ? VK_TRUE : VK_FALSE);
        /* for arrays of textures */
        deviceFeatures.shaderSampledImageArrayDynamicIndexing = (VkBool32)(ctxFeatures.shaderSampledImageArrayDynamicIndexing ? VK_TRUE : VK_FALSE);
        /* for GL <-> VK material shader compatibility */
        deviceFeatures.shaderInt64 = (VkBool32)(ctxFeatures.shaderInt64 ? VK_TRUE : VK_FALSE);

        if (ctxFeatures.deviceDescriptorIndexing)
        {
            VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures{};
            physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
            physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
            physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;

            deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures2.pNext = &physicalDeviceDescriptorIndexingFeatures;
            deviceFeatures2.features = deviceFeatures;
        }

        return deviceFeatures;
        //return initVulkanRenderDeviceWithCompute(vk, vkDev, width, height, ctxExtensions, isDeviceSuitable, deviceFeatures, deviceFeatures2, ctxFeatures.supportsScreenshots_);
    }

	void VulkanDynamicRHI::createInstance()
	{
        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

        // Enumerate available extensions
        std::uint32_t propertiesCount;
        std::vector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, nullptr);
        properties.resize(static_cast<int>(propertiesCount));
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &propertiesCount, properties.data()));

        std::vector<const char*> exts = {
                "VK_KHR_surface",
    #if defined(_WIN32)
                "VK_KHR_win32_surface",
    #endif
    #if defined (__APPLE__)
                "VK_MVK_macos_surface",
                VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
    #endif
    #if defined (__linux__)
                "VK_KHR_xcb_surface"
    #endif
                //, VK_EXT_DEBUG_UTILS_EXTENSION_NAME
                //, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
                /*for indexed textures*/
                //VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        };

        for (auto& extension : m_DeviceParams.requiredVulkanInstanceExtensions) {
            exts.push_back(extension);
        }
        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            exts.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }

        if (enableValidationLayers)
        {
            exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }

        const VkValidationFeatureEnableEXT validationFeaturesEnabled[] = {
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
        };

#if defined(__APPLE__)
        // Shader validation doesn't work in MoltenVK for SPIR-V 1.6 under Vulkan 1.3:
        // "Invalid SPIR-V binary version 1.6 for target environment SPIR-V 1.5 (under Vulkan 1.2 semantics)."
        const VkValidationFeatureDisableEXT validationFeaturesDisabled[] = {
                VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT,
                VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT,
        };
#endif
        const VkValidationFeaturesEXT features = {
                VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
                nullptr,
                enableValidationFeaturesEnabled ? (uint32_t)(sizeof(validationFeaturesEnabled) / sizeof(VkValidationFeatureEnableEXT)) : 0u,
                enableValidationFeaturesEnabled ? validationFeaturesEnabled : nullptr,
    #if defined(_WIN32)
                0u,
            nullptr
    #endif
    #if defined(__APPLE__)
                enableValidationFeaturesDisabled ? (uint32_t)(sizeof(validationFeaturesDisabled) / sizeof(VkValidationFeatureDisableEXT)) : 0u,
                enableValidationFeaturesDisabled ? validationFeaturesDisabled : nullptr
    #endif
        };

#if defined(__APPLE__)
        // https://github.com/KhronosGroup/MoltenVK/blob/main/Docs/MoltenVK_Configuration_Parameters.md
        const int useMetalArgumentBuffers = 1;
        const VkLayerSettingEXT settings[] = {
                {"MoltenVK", "MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &useMetalArgumentBuffers} };
        const VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo = {
                VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                enableValidationLayers ? &features : nullptr,
                (uint32_t)(sizeof(settings) / sizeof(VkLayerSettingEXT)),
                settings
        };
#endif

        const VkApplicationInfo appInfo = {
                VK_STRUCTURE_TYPE_APPLICATION_INFO,
                nullptr,
                "Vulkan",
                VK_MAKE_VERSION(1, 0, 0),
                "No Engine",
                VK_MAKE_VERSION(1, 0, 0),
                VK_API_VERSION_1_3
        };

        VkInstanceCreateInfo createInfo = {
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    #if defined(__APPLE__)
                &layerSettingsCreateInfo,
    #else
                enableValidationLayers ? &features : nullptr,
    #endif
                0,
                &appInfo,
                enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0u,
                enableValidationLayers ? validationLayers.data() : nullptr,
                static_cast<uint32_t>(exts.size()),
                exts.data()
        };

#if defined (__APPLE__)
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
            exts.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif
#endif

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_VulkanInstance.instance));

        volkLoadInstance(m_VulkanInstance.instance);
    }

    void VulkanDynamicRHI::destroyDevice()
    {
        destroySwapChain();

        //vkDestroyCommandPool(m_VulkanDevice, m_C, nullptr);

        for(VkSemaphore& semaphore : m_PresentSemaphores)
        {
	        if(semaphore)
	        {
                vkDestroySemaphore(m_VulkanDevice, semaphore, nullptr);
                semaphore = nullptr;
	        }
        }

        for (VkSemaphore& semaphore : m_AcquireSemaphores)
        {
            if (semaphore)
            {
                vkDestroySemaphore(m_VulkanDevice, semaphore, nullptr);
                semaphore = nullptr;
            }
        }

        /*if (m_DeviceParams.useComputeQueue)
        {
            vkDestroyCommandPool(m_VulkanDevice, m_Resources.computeCommandPool, nullptr);
        }*/

        m_Device = nullptr;

        vkDestroyDevice(m_VulkanDevice, nullptr);
    }

    const VulkanInstance& VulkanDynamicRHI::getVulkanInstance() const
    {
        return m_VulkanInstance;
    }

    void VulkanDynamicRHI::destroyVulkanInstance()
    {
        vkDestroySurfaceKHR(m_VulkanInstance.instance, m_VulkanInstance.surface, nullptr);

        vkDestroyDebugReportCallbackEXT(m_VulkanInstance.instance, m_VulkanInstance.reportCallback, nullptr);
        vkDestroyDebugUtilsMessengerEXT(m_VulkanInstance.instance, m_VulkanInstance.messenger, nullptr);

        vkDestroyInstance(m_VulkanInstance.instance, nullptr);
    }

    bool VulkanDynamicRHI::BeginFrame()
    {
        const VkSemaphore& semaphore = m_AcquireSemaphores[m_AcquireSemaphoreIndex];

        VkResult result;

        int const maxAttempts = 3;
        for(int attempt = 0; attempt < maxAttempts; ++attempt)
        {
            result = vkAcquireNextImageKHR(m_VulkanDevice, m_SwapChain, 0, semaphore, VK_NULL_HANDLE, &m_SwapChainIndex);

            if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR && attempt < maxAttempts)
            {
                //BackBufferResizing();
                VkSurfaceCapabilitiesKHR surfaceCaps;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VulkanPhysicalDevice, m_VulkanInstance.surface, &surfaceCaps);

                m_DeviceParams.backBufferWidth = surfaceCaps.currentExtent.width;
                m_DeviceParams.backBufferHeight = surfaceCaps.currentExtent.height;

                resizeSwapchain();
                BackBufferResized();
            }
            else
                break;
        }

        m_AcquireSemaphoreIndex = (m_AcquireSemaphoreIndex + 1) % m_AcquireSemaphores.size();

        if (result == VkResult::VK_SUCCESS)
        {
            // Schedule the wait. The actual wait operation will be submitted when the app executes any command list.
            m_Device->queueWaitForSemaphore(RHI::CommandQueue::Graphics, semaphore, 0);
            return true;
        }

        return false;
    }

    bool VulkanDynamicRHI::Present()
    {
        const VkSemaphore& semaphore = m_PresentSemaphores[m_PresentSemaphoreIndex];

        m_Device->queueSignalSemaphore(RHI::CommandQueue::Graphics, semaphore, 0);

        std::vector<IRHICommandList*> commandLists;
        m_Device->executeCommandLists(commandLists, 0, CommandQueue::Graphics);

        VkPresentInfoKHR pi{};
        pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        pi.pNext = nullptr;
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &semaphore;
        pi.swapchainCount = 1;
        pi.pSwapchains = &m_SwapChain;
        pi.pImageIndices = &m_SwapChainIndex;

        m_PresentSemaphoreIndex = (m_PresentSemaphoreIndex + 1) % m_PresentSemaphores.size();

        //VK_CHECK(vkQueuePresentKHR(m_PresentQueue, &pi));
        VK_CHECK(vkQueuePresentKHR(m_PresentQueue, &pi));
        VK_CHECK(vkDeviceWaitIdle(m_VulkanDevice));

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
        VkDebugUtilsMessageTypeFlagsEXT Type,
        const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
        void* UserData
    )
    {
        printf("Validation layer: %s\n", CallbackData->pMessage);
        return VK_FALSE;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(
        VkDebugReportFlagsEXT		flags,
        VkDebugReportObjectTypeEXT	objectType,
        uint64_t					object,
        size_t						location,
        int32_t						messageCode,
        const char* pLayerPrefix,
        const char* pMessage,
        void* UserData
    )
    {
        // https://github.com/zeux/niagara/blob/master/src/device.cpp   [ignoring performance warnings]
        // This silences warnings like "For optimal performance image layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL."
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
            return VK_FALSE;

        printf("Debug callback (%s): %s\n", pLayerPrefix, pMessage);
        return VK_FALSE;
    }

    bool setupDebugCallbacks(VkInstance instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback)
    {
        if (!enableValidationLayers) return true;

        {
            VkDebugUtilsMessengerCreateInfoEXT ci{};
            ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            ci.pfnUserCallback = &VulkanDebugCallback;
            ci.pUserData = nullptr;

            VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &ci, nullptr, messenger));
        }
        {
            VkDebugReportCallbackCreateInfoEXT ci{};
            ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            ci.pNext = nullptr;
            ci.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                VK_DEBUG_REPORT_ERROR_BIT_EXT |
                VK_DEBUG_REPORT_DEBUG_BIT_EXT;
            ci.pfnCallback = &VulkanDebugReportCallback;
            ci.pUserData = nullptr;

            VK_CHECK(vkCreateDebugReportCallbackEXT(instance, &ci, nullptr, reportCallback));
        }

        return true;
    }

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapchainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto mode : availablePresentModes)
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;

        // FIFO will always be supported
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        const uint32_t imageCount = capabilities.minImageCount + 1;

        const bool imageCountExceeded = capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount;

        return imageCountExceeded ? capabilities.maxImageCount : imageCount;
    }


    VkResult findSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice)
    {
        uint32_t deviceCount = 0;
        VK_CHECK_RET(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

        if (!deviceCount) return VK_ERROR_INITIALIZATION_FAILED;

        std::vector<VkPhysicalDevice> devices(deviceCount);
        VK_CHECK_RET(vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()));

        for (const auto& device : devices)
        {
            if (selector(device))
            {
                *physicalDevice = device;
                return VK_SUCCESS;
            }
        }

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t findQueueFamilies(VkPhysicalDevice device, VkQueueFlags desiredFlags)
    {
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

        for (uint32_t i = 0; i != families.size(); i++)
        {
            if (families[i].queueCount > 0 && families[i].queueFlags & desiredFlags)
                return i;
        }

        return 0;
    }
}
