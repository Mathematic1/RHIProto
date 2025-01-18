#pragma once

#include <Vulkan.hpp>

#include <vector>
#include <functional>

#include <map>
#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#define VK_CHECK(value) CHECK(value == VK_SUCCESS, __FILE__, __LINE__);
#define VK_CHECK_RET(value) if(value != VK_SUCCESS) { CHECK(false, __FILE__, __LINE__); return value; }
#define BL_CHECK(value) CHECK(value, __FILE__, __LINE__);

namespace RHI::Vulkan
{
	class Buffer;
	class Device;
	class CommandList;
	class Texture;

	// Features we need for our Vulkan context
	struct VulkanContextFeatures
	{
		bool supportsScreenshots_ = false;

		/* for wireframe outlines */
		bool geometryShader_ = true;
		/* for tesselation experiments */
		bool tessellationShader_ = false;

		/* for indirect instanced rendering */
		bool multiDrawIndirect = false;
		bool drawIndirectFirstInstance = false;

		/* for OIT and general atomic operations */
		bool vertexPipelineStoresAndAtomics_ = false;
		bool fragmentStoresAndAtomics_ = false;

		/* for arrays of textures */
		bool shaderSampledImageArrayDynamicIndexing = false;

		/* for GL <-> VK material shader compatibility */
		bool shaderInt64 = false;

		bool deviceDescriptorIndexing = false;
	};

	struct VulkanContextExtensions
	{
		bool KHR_swapchain = true;//false;
		bool KHR_maintenance3 = false;
		bool EXT_discriptor_indexing = false;
		bool EXT_draw_indirect_count = false;
#if defined (__APPLE__)
		bool KHR_portability_subset = false; // either KHR_ or Vulkan 1.2 versions
#endif
	};

	struct VulkanInstance final
	{
		VkInstance instance;
		VkSurfaceKHR surface;
		VkDebugUtilsMessengerEXT messenger;
		VkDebugReportCallbackEXT reportCallback;
	};

	struct VulkanContext
	{
		VulkanContext(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VulkanContextExtensions contextExtensions, VulkanContextFeatures contextFeatures)
			: instance(instance)
			, physicalDevice(physicalDevice)
			, device(device)
			, ctxExtensions(contextExtensions)
			, ctxFeatures(contextFeatures)
		{
		}

		void updateBuffers(uint32_t imageIndex);
		void composeFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// For Chapter 8 & 9
		// TODO: fixe pipeline issue
		/*inline GraphicsPipelineInfo pipelineParametersForOutputs(const std::vector<Texture>& outputs) const
		{
			GraphicsPipelineInfo pInfo{};
			pInfo.width = outputs.empty() ? vkDev.framebufferWidth : outputs[0].width;
			pInfo.height = outputs.empty() ? vkDev.framebufferHeight : outputs[0].height;
			pInfo.useBlending = false;
			return pInfo;
		}*/

		std::vector<VkFramebuffer> swapchainFramebuffers;
		std::vector<VkFramebuffer> swapchainFramebuffers_NoDepth;

		void beginRenderPass(VkCommandBuffer cmdBuffer, VkRenderPass pass, size_t currentImage, const VkRect2D area,
			VkFramebuffer fb = VK_NULL_HANDLE,
			uint32_t clearValueCount = 0, const VkClearValue* clearValues = nullptr)
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = pass;
			renderPassInfo.framebuffer = (fb != VK_NULL_HANDLE) ? fb : swapchainFramebuffers[currentImage];
			renderPassInfo.renderArea = area;
			renderPassInfo.clearValueCount = clearValueCount;
			renderPassInfo.pClearValues = clearValues;

			vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue graphicsQueue;
		VkDescriptorPool descriptorPool;

		VulkanInstance vulkanInstance;
		VulkanContextExtensions ctxExtensions;
		VulkanContextFeatures ctxFeatures;
	};

	// command buffer with resource tracking
	class TrackedCommandBuffer
	{
	public:

		// the command buffer itself
		VkCommandBuffer commandBuffer = VkCommandBuffer();
		VkCommandPool commandPool = VkCommandPool();

		std::vector<IResource*> referencedResources; // to keep them alive
		std::vector<BufferHandle> referencedStagingBuffers; // to allow synchronous mapBuffer

		uint64_t recordingID = 0;
		uint64_t submissionID = 0;

		explicit TrackedCommandBuffer(const VulkanContext& context)
			: m_Context(context)
		{
		}

		~TrackedCommandBuffer() {}

	private:
		const VulkanContext& m_Context;
	};

	typedef std::shared_ptr<TrackedCommandBuffer> TrackedCommandBufferPtr;

	class Queue
	{
	public:
		Queue(const VulkanContext& context, CommandQueue queueID, VkQueue queue, uint32_t queueFamilyIndex);
		~Queue();

		VkSemaphore trackingSemaphore;

		TrackedCommandBufferPtr createCommandBuffer();

		TrackedCommandBufferPtr getOrCreateCommandBuffer();

		void addWaitSemaphore(VkSemaphore semaphore, uint64_t value);
		void addSignalSemaphore(VkSemaphore semaphore, uint64_t value);

		// submits a command buffer to this queue, returns submissionID
		uint64_t submit(std::vector<IRHICommandList*>& commandLists, size_t numCommandLists);

		CommandQueue getQueueID() const { return m_QueueID; }
		uint32_t getQueueFamilyIndex() const { return m_QueueFamilyIndex; }
		VkQueue getVkQueue() const { return m_Queue; }

	private:
		const VulkanContext& m_Context;

		VkQueue m_Queue;
		CommandQueue m_QueueID;
		uint32_t m_QueueFamilyIndex = uint32_t(-1);

		std::mutex m_Mutex;

		std::vector<VkSemaphore> m_WaitSemaphores;
		std::vector<VkSemaphore> m_SignalSemaphores;

		// tracks the list of command buffers in flight on this queue
		std::list<TrackedCommandBufferPtr> m_CommandBuffersInFlight;
		std::list<TrackedCommandBufferPtr> m_CommandBuffersPool;
	};

	class VulkanRHIModule : public IRHIModule
	{
	public:
		VulkanRHIModule();
		virtual ~VulkanRHIModule() override;

		virtual IDynamicRHI* createRHI(const DeviceParams& deviceParams) override;
		virtual void* getWindowInterface() override { return nullptr; }

	private:
	};

	struct DeviceDesc
	{
		uint32_t framebufferWidth;
		uint32_t framebufferHeight;

		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice device;

		VulkanContextExtensions* ctxExtensions;
		VulkanContextFeatures* ctxFeatures;

		uint32_t graphicsFamily;
		VkQueue graphicsQueue;
		bool useGraphicsQueue = false;

		uint32_t computeFamily;
		VkQueue computeQueue;
		bool useComputeQueue = false;

		uint32_t transferFamily;
		VkQueue transferQueue;
		bool useTransferQueue;
	};

	class VulkanDynamicRHI : public IDynamicRHI
	{
	public:
		VulkanDynamicRHI(const DeviceParams& deviceParams);
		virtual ~VulkanDynamicRHI();

		void setWindowSurface(VkSurfaceKHR surface);
		virtual GraphicsAPI getGraphicsAPI() const override;
		virtual void createDeviceInternal() override;
		VkResult createDevice(std::unordered_set<uint32_t>& uniqueQueueFamilies, VkPhysicalDeviceFeatures deviceFeatures, VkPhysicalDeviceFeatures2 deviceFeatures2);
		virtual IDevice* getDevice() const override;
		const VulkanInstance& getVulkanInstance() const;
		bool CreateSwapchain();
		virtual bool BeginFrame() override;
		virtual bool Present() override;

		virtual uint32_t GetBackBufferCount() override;
		virtual uint32_t GetCurrentBackBufferIndex() override;
		virtual TextureHandle GetBackBuffer(uint32_t index) override;
		virtual TextureHandle GetDepthBuffer() override;
		virtual FramebufferHandle GetFramebuffer(uint32_t index) override;

		static VulkanContextFeatures& initializeContextFeatures();
		static VulkanContextExtensions& initializeContextExtensions();

	private:
		void createInstance();
		void destroyDevice();
		void destroyVulkanInstance();
		bool createSwapchain();
		void destroySwapChain();
		void resizeSwapchain();
		size_t createSwapchainImages();
		void createDepthSwapchainImage();
		VkPhysicalDeviceFeatures initVulkanRenderDeviceFeatures(const VulkanContextFeatures& ctxFeatures, VkPhysicalDeviceFeatures2& deviceFeatures2);

	private:
		VulkanInstance m_VulkanInstance;

		VkPhysicalDevice m_VulkanPhysicalDevice;
		uint32_t m_GraphicsQueueFamily = -1;
		uint32_t m_ComputeQueueFamily = -1;
		uint32_t m_TransferQueueFamily = -1;
		uint32_t m_PresentQueueFamily = -1;

		VkDevice m_VulkanDevice;
		VkQueue m_GraphicsQueue;
		VkQueue m_ComputeQueue;
		VkQueue m_TransferQueue;
		VkQueue m_PresentQueue;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		std::vector<TextureHandle> m_SwapchainTextures;
		uint32_t m_SwapChainIndex = -1;
		TextureHandle m_DepthSwapChainTexture = nullptr;

		std::vector<VkSemaphore> m_AcquireSemaphores;
		std::vector<VkSemaphore> m_PresentSemaphores;
		uint32_t m_AcquireSemaphoreIndex = 0;
		uint32_t m_PresentSemaphoreIndex = 0;

		VkSurfaceKHR m_WindowSurface;

		VkSurfaceFormatKHR m_SwapChainFormat;
		VkSwapchainKHR m_SwapChain = VkSwapchainKHR();

		VulkanContextExtensions m_VulkanExtensions;
		VulkanContextFeatures m_VulkanFeatures;

		Vulkan::DeviceHandle m_Device;
	};

	struct SwapchainSupportDetails final
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class Shader final : public IShader
	{
	public:
		Shader(){}
		virtual ~Shader() override {}

		std::vector<unsigned int> SPIRV;
		VkShaderModule shaderModule = nullptr;

		VkShaderStageFlagBits stage{};
	};

	class Buffer : public IBuffer
	{
	public:
		Buffer(const VulkanContext& context)
			: m_Context(context)
		{}
		virtual ~Buffer() override;

		BufferDesc		desc = {};

		VkBuffer		buffer = VK_NULL_HANDLE;
		VkDeviceSize	size = 0u;
		VkDeviceMemory	memory = VK_NULL_HANDLE;

		/* Permanent mapping to CPU address space (see VulkanResources::addBuffer) */
		void* ptr = nullptr;

		virtual const BufferDesc& getDesc() const override
		{
			return desc;
		}

	private:
		const VulkanContext& m_Context;
	};

	class Sampler : public ISampler
	{
	public:
		Sampler(const VulkanContext& context)
			: m_Context(context)
		{}
		virtual ~Sampler() override;

		SamplerDesc desc;

		virtual const SamplerDesc& getDesc() const override
		{
			return desc;
		}

		VkSampler sampler = VK_NULL_HANDLE;

	private:
		const VulkanContext& m_Context;
	};

	// Aggregate structure for passing around the texture data
	class Texture : public ITexture
	{
	public:
		Texture(const VulkanContext& context)
			: m_Context(context)
		{}
		virtual ~Texture() override;

		TextureDesc desc;

		VkFormat format;

		VkImage image = nullptr;
		VkDeviceMemory imageMemory = nullptr;
		VkImageView imageView = nullptr;

		// Offscreen buffers require VK_IMAGE_LAYOUT_GENERAL && static textures have VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		VkImageLayout desiredLayout;

		virtual const TextureDesc& getDesc() const override
		{
			return desc;
		}

	private:
		const VulkanContext& m_Context;
	};

	inline TextureAttachment makeTextureAttachment(ITexture* tex, ShaderStageFlagBits shaderStageFlags)
	{
		TextureAttachment textureAttachment{};
		textureAttachment.dInfo.type = DescriptorType::COMBINED_IMAGE_SAMPLER;
		textureAttachment.dInfo.shaderStageFlags = shaderStageFlags;
		textureAttachment.texture = tex;

		return textureAttachment;
	}

	inline TextureAttachment fsTextureAttachment(ITexture* tex)
	{
		return makeTextureAttachment(tex, ShaderStageFlagBits::FRAGMENT_BIT);
	}

	inline TextureArrayAttachment fsTextureArrayAttachment(const std::vector<ITexture*>& textures)
	{
		TextureArrayAttachment textureArrayAttachment{};
		textureArrayAttachment.dInfo.type = DescriptorType::COMBINED_IMAGE_SAMPLER;
		textureArrayAttachment.dInfo.shaderStageFlags = ShaderStageFlagBits::FRAGMENT_BIT;
		textureArrayAttachment.textures = textures;

		return textureArrayAttachment;
	}

	inline BufferAttachment makeBufferAttachment(IBuffer* buffer, uint32_t offset, uint32_t size, DescriptorType type, ShaderStageFlagBits shaderStageFlags)
	{
		BufferAttachment bufferAttachment{};
		bufferAttachment.dInfo = { type, shaderStageFlags };
		bufferAttachment.buffer = buffer;//{ buffer.buffer, buffer.size, buffer.memory, buffer.memory };
		bufferAttachment.offset = offset;
		bufferAttachment.size = size;
		return bufferAttachment;
	}

	inline BufferAttachment uniformBufferAttachment(IBuffer* buffer, uint32_t offset, uint32_t size, ShaderStageFlagBits shaderStageFlags)
	{
		return makeBufferAttachment(buffer, offset, size, DescriptorType::UNIFORM_BUFFER, shaderStageFlags);
	}

	inline BufferAttachment storageBufferAttachment(Buffer* buffer, uint32_t offset, uint32_t size, ShaderStageFlagBits shaderStageFlags)
	{
		return makeBufferAttachment(buffer, offset, size, DescriptorType::STORAGE_BUFFER, shaderStageFlags);
	}

	void CHECK(bool check, const char* fileName, int lineNumber);

	bool setupDebugCallbacks(VkInstance instance, VkDebugUtilsMessengerEXT* messenger, VkDebugReportCallbackEXT* reportCallback);

	size_t compileShaderFile(const char* file, Shader& shaderModule);

	inline VkPipelineShaderStageCreateInfo shaderStageInfo(VkShaderStageFlagBits shaderStage, Shader& shader, const char* entryPoint)
	{
		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.stage = shaderStage;
		createInfo.module = shader.shaderModule;
		createInfo.pName = entryPoint;
		createInfo.pSpecializationInfo = nullptr;
		return createInfo;
	}

	inline VkDescriptorSetLayoutBinding descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount = 1)
	{
		return VkDescriptorSetLayoutBinding{
			binding,
			descriptorType,
			descriptorCount,
			stageFlags,
			nullptr
		};
	}

	inline VkWriteDescriptorSet bufferWriteDescriptorSet(VkDescriptorSet ds, const VkDescriptorBufferInfo* bi, uint32_t bindIdx, VkDescriptorType dType)
	{
		VkWriteDescriptorSet descriptorSet{};
		descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSet.pNext = nullptr;
		descriptorSet.dstSet = ds;
		descriptorSet.dstBinding = bindIdx;
		descriptorSet.dstArrayElement = 0;
		descriptorSet.descriptorCount = 1;
		descriptorSet.descriptorType = dType;
		descriptorSet.pImageInfo = nullptr;
		descriptorSet.pBufferInfo = bi;
		descriptorSet.pTexelBufferView = nullptr;
		return descriptorSet;
	}

	inline VkWriteDescriptorSet imageWriteDescriptorSet(VkDescriptorSet ds, const VkDescriptorImageInfo* ii, uint32_t bindIdx)
	{
		VkWriteDescriptorSet descriptorSet{};
		descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSet.pNext = nullptr;
		descriptorSet.dstSet = ds;
		descriptorSet.dstBinding = bindIdx;
		descriptorSet.dstArrayElement = 0;
		descriptorSet.descriptorCount = 1;
		descriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSet.pImageInfo = ii;
		descriptorSet.pBufferInfo = nullptr;
		descriptorSet.pTexelBufferView = nullptr;
		return descriptorSet;
	}

	VkResult createSemaphore(VkDevice device, VkSemaphore* outSemaphore);

	bool isDeviceSuitable(VkPhysicalDevice device);

	SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& caps);

	VkResult findSuitablePhysicalDevice(VkInstance instance, std::function<bool(VkPhysicalDevice)> selector, VkPhysicalDevice* physicalDevice);

	uint32_t findQueueFamilies(VkPhysicalDevice device, VkQueueFlags desiredFlags);

	bool downloadImageData(Device& vkDev, VkImage& textureImage, uint32_t texWidth, uint32_t texHeight, VkFormat texFormat, uint32_t layerCount, void* imageData, VkImageLayout sourceImageLayout);

	bool createDepthResources(Device& vkDev, uint32_t width, uint32_t height, Texture& depth);

	bool createTexturedVertexBuffer(Device& vkDev, const char* filename, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize, size_t* indexBufferSize);

	bool createPBRVertexBuffer(Device& vkDev, const char* filename, VkBuffer* storageBuffer, VkDeviceMemory* storageBufferMemory, size_t* vertexBufferSize, size_t* indexBufferSize);

	bool executeComputeShader(Device& vkDev,
		VkPipeline computePipeline, VkPipelineLayout pl, VkDescriptorSet ds,
		uint32_t xsize, uint32_t ysize, uint32_t zsize);

	bool createComputeDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayout);

	void insertComputedImageBarrier(VkCommandBuffer commandBuffer, VkImage image);


	/* Check if the texture is used as a depth buffer */
	inline bool isDepthFormat(VkFormat fmt) {
		return
			(fmt == VK_FORMAT_D16_UNORM) ||
			(fmt == VK_FORMAT_X8_D24_UNORM_PACK32) ||
			(fmt == VK_FORMAT_D32_SFLOAT) ||
			(fmt == VK_FORMAT_D16_UNORM_S8_UINT) ||
			(fmt == VK_FORMAT_D24_UNORM_S8_UINT) ||
			(fmt == VK_FORMAT_D32_SFLOAT_S8_UINT);
	}

	bool setVkObjectName(Device& vkDev, void* object, VkObjectType objectType, const char* name);

	inline bool setVkImageName(Device& vkDev, void* object, const char* name)
	{
		return setVkObjectName(vkDev, object, VK_OBJECT_TYPE_IMAGE, name);
	}

	/* This routine updates one texture discriptor in one descriptor set */
	void updateTextureInDescriptorSetArray(Device& vkDev, VkDescriptorSet ds, Texture t, uint32_t textureIndex, uint32_t bindingIdx);

	VkShaderStageFlagBits glslangShaderStageToVulkan(glslang_stage_t sh);
	glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

	bool hasStencilComponent(VkFormat format);

	struct VulkanResources
	{
		VulkanResources(Device* device)
			: m_Device(device)
		{}
		~VulkanResources() {};

		std::vector<Texture> allTextures;
		std::vector<Buffer> allBuffers;

		std::vector<VkFramebuffer> allFramebuffers;
		std::vector<VkRenderPass> allRenderPasses;

		std::vector<VkPipelineLayout> allPipelineLayouts;
		std::vector<VkPipeline> allPipelines;

		std::vector<VkDescriptorSetLayout> allDSLayouts;
		std::vector<VkDescriptorPool> allDPools;

		std::vector<Shader> shaderModules;
		std::map<std::string, uint32_t> shaderMap;

		std::vector<VkImageView> swapchainImageViews;

		VkCommandBuffer computeCommandBuffer;
		VkCommandPool computeCommandPool;

	private:
		Device* m_Device;
	};

	class RenderPass : public IRenderPass
	{
	public:
		RenderPass() = default;
		explicit RenderPass(const RenderPassCreateInfo& ci = RenderPassCreateInfo()) {};

		virtual ~RenderPass() override {}

		RenderPassCreateInfo info;
		VkRenderPass handle = VK_NULL_HANDLE;
	};

	class Framebuffer : public IFramebuffer
	{
	public:
		explicit Framebuffer(const VulkanContext& context)
			: m_Context(context)
		{}

		virtual ~Framebuffer() override {}

		const FramebufferDesc& getDesc() const override { return desc; }

		FramebufferDesc desc;
		VkRenderPass renderPass = VkRenderPass();
		VkFramebuffer framebuffer = VkFramebuffer();
	private:
		const VulkanContext& m_Context;
	};

	class InputLayout : public IInputLayout
	{
	public:
		std::vector<VertexInputAttributeDesc> inputAttributeDesc;
		std::vector<VertexInputBindingDesc> inputBindingDesc;

		std::vector <VkVertexInputBindingDescription> bindingDescriptions = {};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		virtual uint32_t getNumAttributes() const override;
		virtual const VertexInputAttributeDesc* getVertexAttributeDesc(uint32_t index) const override;

		virtual uint32_t getNumBindings() const override;
		virtual const VertexInputBindingDesc* getVertexBindingDesc(uint32_t index) const override;
	};

	class BindingLayout : public IBindingLayout
	{
	public:
		VkDescriptorSetLayout descriptorSetLayout;

		virtual ~BindingLayout() {};
	};

	class BindingSet : public IBindingSet
	{
	public:
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		BindingSet(const VulkanContext& context);
		virtual ~BindingSet();

	private:
		const VulkanContext& m_Context;
	};

	class GraphicsPipeline : public IGraphicsPipeline
	{
	public:
		GraphicsPipelineDesc desc = {};
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;

		explicit GraphicsPipeline(const VulkanContext& context)
			: m_Context(context)
		{}

		virtual ~GraphicsPipeline() override {};
		const GraphicsPipelineDesc& getDesc() const override { return desc; }
	private:
		const VulkanContext& m_Context;
	};

	class Device : public IDevice
	{
	public:
		Device(const DeviceDesc& desc);
		virtual ~Device() override;

		Queue* getQueue(CommandQueue queue) const { return m_Queues[int(queue)].get(); }
		VulkanResources* getResources() { return &m_Resources; }

		virtual GraphicsAPI getGraphicsAPI() const override;

		virtual TextureHandle createImage(const TextureDesc& desc) override;

		virtual bool createImageView(ITexture* texture, ImageAspectFlagBits aspectFlags) override;

		virtual SamplerHandle createTextureSampler(const SamplerDesc& desc = SamplerDesc()) override;

		virtual SamplerHandle createDepthSampler() override;

		virtual TextureHandle createTextureForNative(VkImage image, VkImageView imageView, ImageAspectFlagBits aspectFlags, const TextureDesc& desc) override;

		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		virtual Format findDepthFormat() override;

		BufferHandle createUniformBuffer(VkDeviceSize bufferSize);

		virtual BufferHandle createBuffer(const BufferDesc& desc) override;

		virtual BufferHandle createSharedBuffer(const BufferDesc& desc) override;

		virtual BufferHandle addBuffer(const BufferDesc& desc, bool createMapping = false) override;

		inline BufferHandle addUniformBuffer(uint64_t bufferSize, bool createMapping = false)
		{
			BufferDesc desc = BufferDesc{}
				.setSize(bufferSize)
				.setIsUniformBuffer(true)
				.setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT); /* for debugging we make it host-visible */
			return addBuffer(desc, createMapping);
		}

		inline BufferHandle addIndirectBuffer(VkDeviceSize bufferSize, bool createMapping = false) {
			BufferDesc desc = BufferDesc{}
				.setSize(bufferSize)
				.setIsDrawIndirectBuffer(true) // | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
				.setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT); /* for debugging we make it host-visible */
			return addBuffer(desc, createMapping);
		}

		inline BufferHandle addStorageBuffer(VkDeviceSize bufferSize, bool createMapping = false)
		{
			BufferDesc desc = BufferDesc{}
				.setSize(bufferSize)
				.setIsStorageBuffer(true)
				.setMemoryProperties(MemoryPropertiesBits::HOST_VISIBLE_BIT | MemoryPropertiesBits::HOST_COHERENT_BIT); /* for debugging we make it host-visible */
			return addBuffer(desc, createMapping); /* for debugging we make it host-visible */
		}

		inline BufferHandle addLocalDeviceStorageBuffer(VkDeviceSize bufferSize, bool createMapping = false)
		{
			BufferDesc desc = BufferDesc{}
				.setSize(bufferSize)
				.setIsStorageBuffer(true)
				.setMemoryProperties(MemoryPropertiesBits::DEVICE_LOCAL_BIT);
			return addBuffer(desc, createMapping);
		}

		/** Copy [data] to GPU device buffer */
		virtual void uploadBufferData(IBuffer* buffer, size_t deviceOffset, const void* data, const size_t dataSize) override;
		virtual void uploadVertexIndexBufferData(IBuffer* buffer, size_t deviceOffset, size_t vertexDataSize, const void* vertexData,
			size_t indexDataSize, const void* indexData, const size_t dataSize) override;

		/** Copy GPU device buffer data to [outData] */
		void downloadBufferData(const VkDeviceMemory& bufferMemory, VkDeviceSize deviceOffset, void* outData, size_t dataSize);

		virtual void* mapBufferMemory(IBuffer* buffer, size_t offset, size_t size) override;
		virtual void* mapTextureMemory(ITexture* texture, size_t offset, size_t size) override;
		virtual void unmapBufferMemory(IBuffer* buffer) override;
		virtual void unmapTextureMemory(ITexture* texture) override;

		inline uint32_t getVulkanBufferAlignment()
		{
			VkPhysicalDeviceProperties devProps;
			vkGetPhysicalDeviceProperties(m_Context.physicalDevice, &devProps);
			return static_cast<uint32_t>(devProps.limits.minStorageBufferOffsetAlignment);
		}

		bool createColorAndDepthRenderPass(VkRenderPass* renderPass, const RenderPassCreateInfo& ci, const FramebufferDesc& framebufferDesc);
		bool createDepthOnlyRenderPass(VkRenderPass* renderPass, const RenderPassCreateInfo& ci);

		IRenderPass* addFullScreenPass(const RenderPassCreateInfo ci = RenderPassCreateInfo());

		IRenderPass* createRenderPass(const FramebufferDesc& framebufferDesc, const RenderPassCreateInfo& ci = {
			true, true, eRenderPassBit_Offscreen | eRenderPassBit_First }) override;

		IRenderPass* addDepthRenderPass(const RenderPassCreateInfo ci = {
			false, true, eRenderPassBit_Offscreen | eRenderPassBit_First });

		bool createPipelineLayout(std::vector<VkDescriptorSetLayout>& dsLayouts, VkPipelineLayout* pipelineLayout);

		bool createPipelineLayoutWithConstants(VkDescriptorSetLayout dsLayout, VkPipelineLayout* pipelineLayout, uint32_t vtxConstSize, uint32_t fragConstSize);

		VkPipelineLayout addPipelineLayout(VkDescriptorSetLayout dsLayout, uint32_t vtxConstSize = 0, uint32_t fragConstSize = 0);

		VkPipeline addPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer);

		VkResult createComputePipeline(VkShaderModule computeShader, VkPipelineLayout pipelineLayout, VkPipeline* pipeline);

		/* Calculate the descriptor pool size from the list of buffers and textures */
		VkDescriptorPool createDescriptorPool(const DescriptorSetInfo& dsInfo, uint32_t dSetCount = 1);

		IBindingLayout* createDescriptorSetLayout(const DescriptorSetInfo& dsInfo);

		virtual IBindingSet* createDescriptorSet(const DescriptorSetInfo& dsInfo, uint32_t dSetCount, IBindingLayout* bindingLayout) override;

		virtual IInputLayout* createInputLayout(const VertexInputAttributeDesc* attributes, const VertexInputBindingDesc* bindings) override;

		void updateDescriptorSet(VkDescriptorSet ds, const DescriptorSetInfo& dsInfo);

		bool createColorAndDepthFramebuffers(VkRenderPass renderPass, VkImageView depthImageView, std::vector<VkFramebuffer>& swapchainFramebuffers);

		virtual FramebufferHandle createFramebuffer(IRenderPass* renderPass, const std::vector<ITexture*>& images) override;

		std::vector<VkFramebuffer> addFramebuffers(VkRenderPass renderPass, VkImageView depthView = VK_NULL_HANDLE);

		virtual ShaderHandle createShaderModule(const char* fileName) override;

		virtual CommandListHandle createCommandList(const CommandListParameters& params) override;
		virtual uint64_t executeCommandLists(std::vector<IRHICommandList*>& commandLists, size_t numCommandLists, CommandQueue executionQueue) override;

		// vulkan::IDevice implementation
		VkSemaphore getQueueSemaphore(CommandQueue queueID) override;
		void queueWaitForSemaphore(CommandQueue waitQueueID, VkSemaphore semaphore, uint64_t value) override;
		void queueSignalSemaphore(CommandQueue executionQueueID, VkSemaphore semaphore, uint64_t value) override;

	private:
		VulkanContext m_Context;
		DeviceDesc m_DeviceDesc;
		VulkanResources m_Resources;

		// array of submission queues
		std::array<std::unique_ptr<Queue>, uint32_t(CommandQueue::Count)> m_Queues;

		// a list of all queues indices (for shared buffer allocations)
		std::vector<uint32_t> m_DeviceQueueIndices;

		virtual GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer) override;
	};

	class CommandList : public IRHICommandList
	{
	public:
		CommandList(Device* device, VulkanContext& context, const CommandListParameters& parameters);
		virtual ~CommandList() override;

		virtual void beginSingleTimeCommands() override;
		virtual void endSingleTimeCommands() override;
		virtual void queueWaitIdle() override;

		virtual void copyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size) override;
		virtual void writeBuffer(IBuffer* srcBuffer, size_t size, const void* data) override;
		virtual void transitionImageLayout(ITexture* texture, ImageLayout oldLayout, ImageLayout newLayout) override;
		virtual void transitionBufferLayout(IBuffer* texture, ImageLayout oldLayout, ImageLayout newLayout) override;
		void transitionImageLayoutCmd(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1, uint32_t mipLevels = 1);
		void transitionBufferLayoutCmd(VkBuffer buffer, VkFormat format, VkAccessFlags oldAccess, VkAccessFlags newAccess, uint32_t offset = 0, uint32_t size = 0);

		/* VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for real update of an existing texture */
		virtual bool updateTextureImage(ITexture* texture, const void* imageData, ImageLayout sourceImageLayout = ImageLayout::UNDEFINED) override;

		virtual void copyBufferToImage(IBuffer* buffer, ITexture* texture) override;
		virtual void copyMIPBufferToImage(IBuffer* buffer, ITexture* texture, uint32_t bytesPP) override;
		void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height, uint32_t layerCount = 1);
		virtual void copyTexture(ITexture* srcTexture, const TextureSubresourse& srcSubresource, ITexture* dstTexture, const TextureSubresourse dstSubresource) override;
		virtual void blitTexture(ITexture* srcTexture, const TextureSubresourse& srcSubresource, ITexture* dstTexture, const TextureSubresourse dstSubresource) override;
		virtual void resolveTexture(ITexture* srcTexture, const TextureSubresourse& srcSubresource, ITexture* dstTexture, const TextureSubresourse dstSubresource) override;
		virtual void clearColorTexture(ITexture* texture, const TextureSubresourse& subresource, const Color& color) override;
		virtual void clearDepthTexture(ITexture* texture, const TextureSubresourse& subresource, float depthValue, uint32_t stencilValue) override;

		void beginRenderPass(Framebuffer* framebuffer);
		void endRenderPass();
		void setGraphicsState(const GraphicsState& state) override;
		void draw(const DrawArguments& args) override;
		void drawIndexed(const DrawArguments& args) override;

		void bindBindingSets(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, const std::vector<IBindingSet*> bindings);

		TrackedCommandBufferPtr getCurrentCommandBuffer() const { return m_CurrentCommandBuffer; }

	private:
		Device* m_Device;
		const VulkanContext& m_Context;
		CommandListParameters m_CommandListParameters;

		TrackedCommandBufferPtr m_CurrentCommandBuffer;

		VkCommandPool m_CommandPool;
		VkCommandBuffer m_CommandBuffer;

		GraphicsState m_CurrentGraphicsState{};
	};
}
