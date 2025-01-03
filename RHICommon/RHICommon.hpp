#pragma once

#include <Common/Resources.hpp>

#include <cstdint>
#include <memory>
#include <vector>

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator & (T a, T b) { return T(uint8_t(a) & uint8_t(b)); } \
    inline T operator | (T a, T b) { return T(uint8_t(a) | uint8_t(b)); } \
    inline bool operator != (T a, uint8_t b) { return uint8_t(a) != b; }

namespace RHI
{
    class IBuffer;
	class ITexture;
    class ISampler;
    class IGraphicsPipeline;
	class IDevice;
    class IFramebuffer;

	enum class GraphicsAPI : uint8_t
    {
        OGL,
        D3D12,
        VULKAN
    };

    enum class Format : uint8_t
    {
        UNKNOWN,

        R8_UINT,
        R8_SINT,
        R8_UNORM,
        R8_SNORM,
        RG8_UINT,
        RG8_SINT,
        RG8_UNORM,
        RG8_SNORM,
        R16_UINT,
        R16_SINT,
        R16_UNORM,
        R16_SNORM,
        R16_FLOAT,
        BGRA4_UNORM,
        B5G6R5_UNORM,
        B5G5R5A1_UNORM,
        RGBA8_UINT,
        RGBA8_SINT,
        RGBA8_UNORM,
        RGBA8_SNORM,
        BGRA8_UNORM,
        SRGBA8_UNORM,
        SBGRA8_UNORM,
        R10G10B10A2_UNORM,
        R11G11B10_FLOAT,
        RG16_UINT,
        RG16_SINT,
        RG16_UNORM,
        RG16_SNORM,
        RG16_FLOAT,
        R32_UINT,
        R32_SINT,
        R32_FLOAT,
        RGBA16_UINT,
        RGBA16_SINT,
        RGBA16_FLOAT,
        RGBA16_UNORM,
        RGBA16_SNORM,
        RG32_UINT,
        RG32_SINT,
        RG32_FLOAT,
        RGB32_UINT,
        RGB32_SINT,
        RGB32_FLOAT,
        RGBA32_UINT,
        RGBA32_SINT,
        RGBA32_FLOAT,

        D16,
        D24S8,
        X24G8_UINT,
        D32,
        D32S8,
        X32G8_UINT,

        BC1_UNORM,
        BC1_UNORM_SRGB,
        BC2_UNORM,
        BC2_UNORM_SRGB,
        BC3_UNORM,
        BC3_UNORM_SRGB,
        BC4_UNORM,
        BC4_SNORM,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_UFLOAT,
        BC6H_SFLOAT,
        BC7_UNORM,
        BC7_UNORM_SRGB,

        COUNT,
    };

    inline uint32_t bytesPerTexFormat(Format fmt)
    {
        switch (fmt)
        {
        case Format::R8_SINT:
        case Format::R8_UNORM:
            return 1;
        case Format::R16_FLOAT:
            return 2;
        case Format::RG16_FLOAT:
            return 4;
        case Format::RG16_SNORM:
            return 4;
        case Format::BGRA8_UNORM:
            return 4;
        case Format::RGBA8_UNORM:
            return 4;
        case Format::RGBA16_FLOAT:
            return 4 * sizeof(uint16_t);
        case Format::RGBA32_FLOAT:
            return 4 * sizeof(float);
        default:
            break;
        }
        return 0;
    }

    enum class MemoryPropertiesBits : uint32_t
    {
        DEVICE_LOCAL_BIT = 0x00000001,
        HOST_VISIBLE_BIT = 0x00000002,
        HOST_COHERENT_BIT = 0x00000004,
        HOST_CACHED_BIT = 0x00000008,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };

    ENUM_CLASS_FLAG_OPERATORS(MemoryPropertiesBits)

    enum class CreateFlagBits : uint8_t
    {
        NONE_BIT = 0,
        SPARSE_BINDING_BIT,
        SPARSE_RESIDENCY_BIT,
        SPARSE_ALIASED_BIT,
        MUTABLE_FORMAT_BIT,
        CUBE_COMPATIBLE_BIT,
        ALIAS_BIT,
        SPLIT_INSTANCE_BIND_REGIONS_BIT,
        ARRAY_2D_COMPATIBLE_BIT,
    	BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
        EXTENDED_USAGE_BIT,
        PROTECTED_BIT,
        DISJOINT_BIT,
        FLAG_BITS_MAX_ENUM
    };

    enum class ImageLayout {
        UNDEFINED = 0,
        GENERAL = 1,
        COLOR_ATTACHMENT_OPTIMAL = 2,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
        DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
        SHADER_READ_ONLY_OPTIMAL = 5,
        TRANSFER_SRC_OPTIMAL = 6,
        TRANSFER_DST_OPTIMAL = 7,
        PREINITIALIZED = 8,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 9,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 10,
        DEPTH_ATTACHMENT_OPTIMAL = 11,
        DEPTH_READ_ONLY_OPTIMAL = 12,
        STENCIL_ATTACHMENT_OPTIMAL = 13,
        STENCIL_READ_ONLY_OPTIMAL = 14,
        READ_ONLY_OPTIMAL = 15,
        ATTACHMENT_OPTIMAL = 16,
        COUNT = 17
    };

    enum class ImageAspectFlagBits {
        NONE = 0,
        COLOR_BIT = 0x00000001,
        DEPTH_BIT = 0x00000002,
        STENCIL_BIT = 0x00000004,
        MAX_ENUM = 0x7FFFFFFF
    };
    ENUM_CLASS_FLAG_OPERATORS(ImageAspectFlagBits)

    enum class TextureDimension : uint8_t
    {
        Unknown,
        Texture1D,
        Texture1DArray,
        Texture2D,
        Texture2DArray,
        TextureCube,
        TextureCubeArray,
        Texture2DMS,
        Texture2DMSArray,
        Texture3D
    };

    enum class DescriptorType {
        SAMPLER = 0,
        COMBINED_IMAGE_SAMPLER = 1,
        SAMPLED_IMAGE = 2,
        STORAGE_IMAGE = 3,
        UNIFORM_TEXEL_BUFFER = 4,
        STORAGE_TEXEL_BUFFER = 5,
        UNIFORM_BUFFER = 6,
        STORAGE_BUFFER = 7,
        UNIFORM_BUFFER_DYNAMIC = 8,
        STORAGE_BUFFER_DYNAMIC = 9,
        INPUT_ATTACHMENT = 10,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class ShaderStageFlagBits : uint32_t {
        VERTEX_BIT = 0x00000001,
        TESSELLATION_CONTROL_BIT = 0x00000002,
        TESSELLATION_EVALUATION_BIT = 0x00000004,
        GEOMETRY_BIT = 0x00000008,
        FRAGMENT_BIT = 0x00000010,
        COMPUTE_BIT = 0x00000020,
        ALL_GRAPHICS = 0x0000001F,
        ALL = 0x7FFFFFFF,
        MAX_ENUM = 0x7FFFFFFF
    };

    ENUM_CLASS_FLAG_OPERATORS(ShaderStageFlagBits)

    struct DescriptorInfo
    {
        DescriptorType type = DescriptorType::MAX_ENUM;
        ShaderStageFlagBits shaderStageFlags = ShaderStageFlagBits::VERTEX_BIT;
    };

    struct BufferAttachment
    {
        DescriptorInfo  dInfo;

        IBuffer* buffer = nullptr;
        uint32_t        offset;
        uint32_t        size;

        BufferAttachment& setDescriptorInfo(const DescriptorInfo& value) { dInfo = value; return *this; }
        BufferAttachment& setBuffer(IBuffer* value) { buffer = value; return *this; }
        BufferAttachment& setOffset(uint32_t value) { offset = value; return *this; }
        BufferAttachment& setSize(uint32_t value) { size = value; return *this; }
    };

    struct TextureAttachment
    {
        DescriptorInfo  dInfo;

        ITexture* texture = nullptr;
        ISampler* sampler = nullptr;

        TextureAttachment& setDescriptorInfo(const DescriptorInfo& value) { dInfo = value; return *this; }
        TextureAttachment& setTexture(ITexture* value) { texture = value; return *this; }
        TextureAttachment& setSampler(ISampler* value) { sampler = value; return *this; }
    };

    struct TextureArrayAttachment
    {
        DescriptorInfo  dInfo;

        std::vector<ITexture*>  textures;
    };

    /** An aggregate structure with all the data for descriptor set (or descriptor set layout) allocation */
    struct DescriptorSetInfo
    {
        std::vector<BufferAttachment>       buffers;
        std::vector<TextureAttachment>      textures;
        std::vector<TextureArrayAttachment> textureArrays;
    };

    struct VertexInputBindingDesc
    {
        uint32_t binding = 0u;
        uint32_t stride = 0u;

        VertexInputBindingDesc& setBinding(uint32_t value) { binding = value; return *this; }
        VertexInputBindingDesc& setStride(uint32_t value) { stride = value; return *this; }
    };

    struct VertexInputAttributeDesc
    {
        uint32_t    location = 0u;
        uint32_t    binding = 0u;
        Format      format = Format::UNKNOWN;
        uint32_t    offset = 0u;

        VertexInputAttributeDesc& setLocation(uint32_t value) { location = value; return *this; }
        VertexInputAttributeDesc& setBinding(uint32_t value) { binding = value; return *this; }
        VertexInputAttributeDesc& setOffset(uint32_t value) { offset = value; return *this; }
        VertexInputAttributeDesc& setFormat(Format value) { format = value; return *this; }
    };

    class IInputLayout : public IResource
    {
    public:
        virtual uint32_t getNumAttributes() const = 0;
        virtual const VertexInputAttributeDesc* getVertexAttributeDesc(uint32_t index) const = 0;

        virtual uint32_t getNumBindings() const = 0;
        virtual const VertexInputBindingDesc* getVertexBindingDesc(uint32_t index) const = 0;
    };

    struct VertexBufferBinding
    {
        IBuffer* buffer = nullptr;
        uint32_t bindingSlot = 0;
        size_t offset = 0;
    };

    struct IndexBufferBinding
    {
        IBuffer* buffer = nullptr;
        size_t offset = 0;
        bool index32BitType = false;
    };

    class IBindingLayout : public IResource
    {
    public:
    };

    class IBindingSet : public IResource
    {
	    
    };

    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,
        Copy,

        Count
    };

    struct CommandListParameters
    {
    	// Type of the queue that this command list is to be executed on.
        // COPY and COMPUTE queues have limited subsets of methods available.
        CommandQueue queueType = CommandQueue::Graphics;
    };

    struct GraphicsState
    {
        IGraphicsPipeline* pipeline = nullptr;
        IFramebuffer* framebuffer = nullptr;

        std::vector<IBindingSet*> bindingSets;
        std::vector<VertexBufferBinding> vertexBufferBindings;
        IndexBufferBinding indexBufferBinding;

        GraphicsState& setPipeline(IGraphicsPipeline* value) { pipeline = value; return *this; }
        GraphicsState& setFramebuffer(IFramebuffer* value) { framebuffer = value; return *this; }
    };

    struct DrawArguments
    {
        uint32_t vertexCount = 0;
        uint32_t instanceCount = 1;
        uint32_t startIndexLocation = 0;
        uint32_t startVertexLocation = 0;
        uint32_t startInstanceLocation = 0;

        DrawArguments& setVertexCount(uint32_t value) { vertexCount = value; return *this; }
        DrawArguments& setInstanceCount(uint32_t value) { instanceCount = value; return *this; }
        DrawArguments& setStartIndexLocation(uint32_t value) { startIndexLocation = value; return *this; }
        DrawArguments& setStartVertexLocation(uint32_t value) { startVertexLocation = value; return *this; }
        DrawArguments& setStartInstanceLocation(uint32_t value) { startInstanceLocation = value; return *this; }
    };

    class IRHICommandList : public IResource
    {
    public:
        virtual void beginSingleTimeCommands() = 0;
        virtual void endSingleTimeCommands() = 0;
        virtual void queueWaitIdle() = 0;
        virtual void draw(const DrawArguments& args) = 0;
        virtual void drawIndexed(const DrawArguments& args) = 0;
        virtual void setGraphicsState(const GraphicsState& state) = 0;
        virtual void transitionImageLayout(ITexture* texture, ImageLayout oldLayout, ImageLayout newLayout) = 0;
        virtual void transitionBufferLayout(IBuffer* texture, ImageLayout oldLayout, ImageLayout newLayout) = 0;
        virtual bool updateTextureImage(ITexture* texture, const void* imageData, ImageLayout sourceImageLayout = ImageLayout::UNDEFINED) = 0;
        virtual void copyBufferToImage(IBuffer* buffer, ITexture* texture) = 0;
        virtual void copyMIPBufferToImage(IBuffer* buffer, ITexture* texture, uint32_t bytesPP) = 0;
        virtual void copyBuffer(IBuffer* srcBuffer, IBuffer* dstBuffer, size_t size) = 0;
        virtual void writeBuffer(IBuffer* srcBuffer, size_t size, const void* data) = 0;
    };

    class IInstance : public IResource
    {
	    
    };

    struct Resolution
    {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct DeviceParams
    {
        bool useGraphicsQueue = true;
        bool useComputeQueue = false;
        bool useTransferQueue = false;
        bool usePresentQueue = false;

        bool enableDebugRuntime = false;

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        uint32_t maxFramesInFlight = 2;

        bool vSyncEnabled = false;
        bool supportScreenshots = false;

        std::vector<const char*> requiredVulkanInstanceExtensions;
    };

    class IDynamicRHI
    {
    public:
        IDynamicRHI(const DeviceParams& deviceParams)
            : m_DeviceParams(deviceParams)
        {}
        virtual ~IDynamicRHI() = default;

        bool CreateDevice();
        virtual void createDeviceInternal() = 0;
        virtual bool BeginFrame() = 0;
        virtual bool Present() = 0;
        virtual void BackBufferResized();
        virtual ITexture* GetBackBuffer(uint32_t index) = 0;
        virtual ITexture* GetDepthBuffer() = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;
        virtual uint32_t GetBackBufferCount() = 0;
        virtual IFramebuffer* GetFramebuffer(uint32_t index) = 0;
        virtual IDevice* getDevice() const = 0;
        virtual GraphicsAPI getGraphicsAPI() const = 0;

    protected:
        DeviceParams m_DeviceParams;
        std::vector<IFramebuffer*> m_SwapChainFramebuffers;
    };

    class IRHIModule
    {
    public:
        IRHIModule() = default;
        virtual ~IRHIModule() = default;

    public:
        virtual IDynamicRHI* createRHI(const DeviceParams& deviceParams) = 0;
        virtual void* getWindowInterface() = 0;
    };

    enum class SamplerFilter
    {
        NEAREST = 0,
        LINEAR = 1,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class SamplerAddressMode
    {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        CLAMP_TO_BORDER = 3,
        MIRROR_CLAMP_TO_EDGE = 4,
    	MODE_MAX_ENUM = 0x7FFFFFFF
    };

    struct SamplerDesc
    {
        SamplerAddressMode addressU = SamplerAddressMode::REPEAT;
        SamplerAddressMode addressV = SamplerAddressMode::REPEAT;
        SamplerAddressMode addressW = SamplerAddressMode::REPEAT;
        SamplerFilter minFilter = SamplerFilter::LINEAR;
        SamplerFilter magFilter = SamplerFilter::LINEAR;

        constexpr SamplerDesc& setAddressU(SamplerAddressMode value) { addressU = value; return *this; }
        constexpr SamplerDesc& setAddressV(SamplerAddressMode value) { addressV = value; return *this; }
        constexpr SamplerDesc& setAddressW(SamplerAddressMode value) { addressW = value; return *this; }
        constexpr SamplerDesc& setAddressAll(SamplerAddressMode value) { addressU = addressV = addressW = value; return *this; }
        constexpr SamplerDesc& setMinFilter(SamplerFilter value) { minFilter = value; return *this; }
        constexpr SamplerDesc& setMagFilter(SamplerFilter value) { magFilter = value; return *this; }
    };

    class ISampler : public IResource
    {
    public:
        virtual const SamplerDesc& getDesc() const = 0;
    };

    struct ImageUsage
    {
        bool isTransferSrc = false;
        bool isTransferDst = false;
        bool isShaderResource = false;
        bool isRenderTarget = false;
        bool isUAV = false;
    };

    struct TextureDesc
    {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t layerCount = 1;
        Format format = Format::UNKNOWN;
        MemoryPropertiesBits memoryProperties = MemoryPropertiesBits::DEVICE_LOCAL_BIT;
        bool isLinearTiling = false;
        CreateFlagBits flags = CreateFlagBits::NONE_BIT;
        TextureDimension dimension = TextureDimension::Texture2D;

        ImageUsage usage = {};

        TextureDesc& setWidth(uint32_t value) { width = value; return *this; }
        TextureDesc& setHeight(uint32_t value) { height = value; return *this; }
        TextureDesc& setDepth(uint32_t value) { depth = value; return *this; }
        TextureDesc& setMipLevels(uint32_t value) { mipLevels = value; return *this; }
        TextureDesc& setLayerCount(uint32_t value) { layerCount = value; return *this; }
        TextureDesc& setFormat(Format value) { format = value; return *this; }
        TextureDesc& setDimension(TextureDimension value) { dimension = value; return *this; }
        TextureDesc& setMemoryProperties(MemoryPropertiesBits value) { memoryProperties = value; return *this; }
        TextureDesc& setIsTransferSrc(bool value) { usage.isTransferSrc = value; return *this; }
        TextureDesc& setIsTransferDst(bool value) { usage.isTransferDst = value; return *this; }
        TextureDesc& setIsShaderResource(bool value) { usage.isShaderResource = value; return *this; }
        TextureDesc& setIsRenderTarget(bool value) { usage.isRenderTarget = value; return *this; }
        TextureDesc& setIsUAV(bool value) { usage.isUAV = value; return *this; }
    };

    class ITexture : public IResource
    {
    public:
        virtual const TextureDesc& getDesc() const = 0;
    };

    struct BufferUsage
    {
        bool isTransferSrc = false;
        bool isTransferDst = false;
        bool isIndexBuffer = false;
        bool isVertexBuffer = false;
        bool isUniformBuffer = false;
        bool isStorageBuffer = false;
        bool isDrawIndirectBuffer = false;
    };

    struct BufferDesc
    {
        uint64_t size;
        Format format = Format::UNKNOWN;
        MemoryPropertiesBits memoryProperties = MemoryPropertiesBits::DEVICE_LOCAL_BIT;
        BufferUsage usage = {};

        constexpr BufferDesc& setSize(uint64_t value) { size = value; return *this; }
        constexpr BufferDesc& setFormat(Format value) { format = value; return *this; }
        constexpr BufferDesc& setMemoryProperties(MemoryPropertiesBits value) { memoryProperties = value; return *this; }
        constexpr BufferDesc& setIsTransferSrc(bool value) { usage.isTransferSrc = value; return *this; }
        constexpr BufferDesc& setIsTransferDst(bool value) { usage.isTransferDst = value; return *this; }
        constexpr BufferDesc& setIsIndexBuffer(bool value) { usage.isIndexBuffer = value; return *this; }
        constexpr BufferDesc& setIsVertexBuffer(bool value) { usage.isVertexBuffer = value; return *this; }
        constexpr BufferDesc& setIsUniformBuffer(bool value) { usage.isUniformBuffer = value; return *this; }
        constexpr BufferDesc& setIsStorageBuffer(bool value) { usage.isStorageBuffer = value; return *this; }
        constexpr BufferDesc& setIsDrawIndirectBuffer(bool value) { usage.isDrawIndirectBuffer = value; return *this; }
    };

    class IBuffer : public IResource
    {
    public:
        virtual const BufferDesc& getDesc() const = 0;
    };

    struct ShaderDesc
    {
	    
    };

    class IShader : public IResource
    {
	    
    };

    struct FramebufferDesc
    {
	    
    };

    enum eRenderPassBit : uint8_t
    {
        eRenderPassBit_First = 0x01, // clear the attachment
        eRenderPassBit_Last = 0x02, // transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        eRenderPassBit_Offscreen = 0x04, // transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        eRenderPassBit_OffscreenInternal = 0x08, // keepVK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    };

    struct RenderPassCreateInfo final
    {
        bool clearColor = false;
        bool clearDepth = false;
        bool useDepth = false;
        uint32_t numOutputs = 0;
        Format format = Format::UNKNOWN;
        uint8_t flags = 0;
    };

    class IRenderPass : public IResource
    {
	    
    };

    class IFramebuffer : public IResource
    {
    public:
        uint32_t framebufferWidth = -1;
        uint32_t framebufferHeight = -1;

    private:
        IRenderPass* m_RenderPass;
    };

    enum class PrimitiveType : uint8_t
    {
        PointList,
        LineList,
        TriangleList,
        TriangleStrip,
        TriangleFan,
        TriangleListWithAdjacency,
        TriangleStripWithAdjacency,
        PatchList
    };

    /* A structure with pipeline parameters */
    struct GraphicsPipelineInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t topology = 3; /* defaults to triangles*/

        bool useDepth = true;
        bool useBlending = false;
        bool dynamicScissorState = false;

        uint32_t patchControlPoints = 0;
    };

    struct GraphicsPipelineDesc
    {
        PrimitiveType primType = PrimitiveType::TriangleList;
        IInputLayout* inputLayout = nullptr;
        std::vector<IBindingLayout*> bindingLayouts;


        std::shared_ptr<IShader> VS = nullptr;
        std::shared_ptr<IShader> HS = nullptr;
        std::shared_ptr<IShader> DS = nullptr;
        std::shared_ptr<IShader> GS = nullptr;
        std::shared_ptr<IShader> PS = nullptr;

        //VkPipelineLayout pipelineLayout,
        GraphicsPipelineInfo pipelineInfo;

        GraphicsPipelineDesc& setPrimType(PrimitiveType value) { primType = value; return *this; }
        GraphicsPipelineDesc& setInputLayout(IInputLayout* value) { inputLayout = value; return *this; }
        GraphicsPipelineDesc& setVertexShader(std::shared_ptr<IShader> value) { VS = value; return *this; }
        GraphicsPipelineDesc& setTessallationControlShader(std::shared_ptr<IShader> value) { HS = value; return *this; }
        GraphicsPipelineDesc& setTessallationEvaluationShader(std::shared_ptr<IShader> value) { DS = value; return *this; }
        GraphicsPipelineDesc& setGeometryShader(std::shared_ptr<IShader> value) { GS = value; return *this; }
        GraphicsPipelineDesc& setPixelShader(std::shared_ptr<IShader> value) { PS = value; return *this; }
    };

    class IGraphicsPipeline : public IResource
    {
        virtual const GraphicsPipelineDesc& getDesc() const = 0;
    };

    class IDevice : public IResource
    {
    public:
        virtual IRHICommandList* createCommandList(const CommandListParameters& params = CommandListParameters()) = 0;
        virtual uint64_t executeCommandLists(std::vector<IRHICommandList*>& commandLists, size_t numCommandLists, CommandQueue executionQueue = CommandQueue::Graphics) = 0;
        virtual GraphicsAPI getGraphicsAPI() const = 0;
        virtual IRenderPass* createRenderPass(const RenderPassCreateInfo& ci = RenderPassCreateInfo()) = 0;
        virtual IFramebuffer* createFramebuffer(IRenderPass* renderPass, const std::vector<ITexture*>& images) = 0;
        virtual IGraphicsPipeline* createGraphicsPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer) = 0;
        virtual std::shared_ptr<IShader> createShaderModule(const char* fileName) = 0;
        virtual IBindingLayout* createDescriptorSetLayout(const DescriptorSetInfo& dsInfo) = 0;
        virtual IBindingSet* createDescriptorSet(const DescriptorSetInfo& dsInfo, uint32_t dSetCount, IBindingLayout* bindingLayout) = 0;
        virtual IInputLayout* createInputLayout(const VertexInputAttributeDesc* attributes, const VertexInputBindingDesc* bindings) = 0;
        virtual ITexture* createImage(const TextureDesc& desc) = 0;
        virtual bool createImageView(ITexture* texture, ImageAspectFlagBits aspectFlags) = 0;
        virtual ISampler* createTextureSampler(const SamplerDesc& desc = SamplerDesc()) = 0;
        virtual ISampler* createDepthSampler() = 0;
        virtual IBuffer* createBuffer(const BufferDesc& desc) = 0;
        virtual IBuffer* createSharedBuffer(const BufferDesc& desc) = 0;
        virtual IBuffer* addBuffer(const BufferDesc& desc, bool createMapping = false) = 0;
        virtual void uploadBufferData(IBuffer* buffer, size_t deviceOffset, const void* data, const size_t dataSize) = 0;
        virtual void uploadVertexIndexBufferData(IBuffer* buffer, size_t deviceOffset, size_t vertexDataSize, const void* vertexData,
            size_t indexDataSize, const void* indexData, const size_t dataSize) = 0;
        virtual Format findDepthFormat() = 0;
    };
}
