#pragma once

#include <Common/Resources.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

#define ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } \
    inline T& operator &= (T &a, const T &b) { a = a & b; return a; } \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T& operator |= (T &a, const T &b) { a = a | b; return a; } \
    inline bool operator ! (T a) { return uint32_t(a) == 0; } \
    inline bool operator != (T a, uint32_t b) { return uint32_t(a) != b; }

namespace RHI
{
    static constexpr uint32_t kMaxRenderTargets = 8;
    static constexpr uint32_t kMaxVertexAttributes = 8;
    static constexpr uint32_t kMaxBindingSets = 8;

    class IBuffer;
    class ITexture;
    class ISampler;
    class IShader;
    class IInputLayout;
    class IBindingLayout;
    class IBindingSet;
    class IGraphicsPipeline;
    class IFramebuffer;
    class IRHICommandList;
    class IDevice;

    typedef std::shared_ptr<IBuffer> BufferHandle;
    typedef std::shared_ptr<ITexture> TextureHandle;
    typedef std::shared_ptr<ISampler> SamplerHandle;
    typedef std::shared_ptr<IShader> ShaderHandle;
    typedef std::shared_ptr<IInputLayout> InputLayoutHandle;
    typedef std::shared_ptr<IBindingLayout> BindingLayoutHandle;
    typedef std::shared_ptr<IBindingSet> BindingSetHandle;
    typedef std::shared_ptr<IGraphicsPipeline> GraphicsPipelineHandle;
    typedef std::shared_ptr<IFramebuffer> FramebufferHandle;
    typedef std::shared_ptr<IRHICommandList> CommandListHandle;
    typedef std::shared_ptr<IDevice> DeviceHandle;

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

    struct FormatInfo
    {
        Format format;
        uint8_t bytesPerBlock;
        uint8_t blockSize;
    };

    const FormatInfo& getFormatInfo(Format format);

    struct Color
    {
        float r, g, b, a;

        Color() : r(0.0f), g(0.0f), b(0.0f), a(0.0f){}
        Color(float red, float green, float blue, float alpha) : r(red), g(green), b(blue), a(alpha) {}

        bool operator==(const Color &other) const {
            constexpr float eps = 1e-5f;
            return std::fabs(r - other.r) < eps && std::fabs(b - other.b) && std::fabs(g - other.g) &&
                   std::fabs(a - other.a);
        }
        bool operator!=(const Color &other) const {
            return !(*this == other);
        }
    };

    struct Viewport
    {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;

        Viewport() : minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0)
        {
        }

        Viewport(float _minX, float _maxX, float _minY, float _maxY, float _minZ, float _maxZ)
            : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY), minZ(_minZ), maxZ(_maxZ)
        {
        }

        bool operator==(const Viewport &rhs) const
        {
            constexpr float eps = 1e-5f;
            return std::fabs(minX - rhs.minX) < eps && std::fabs(maxX - rhs.maxX) < eps &&
                   std::fabs(minY - rhs.minY) < eps && std::fabs(maxY - rhs.maxY) < eps &&
                   std::fabs(minZ - rhs.minZ) < eps && std::fabs(maxZ - rhs.maxZ) < eps;
        }

        bool operator!=(const Viewport &rhs) const
        {
            return !(*this == rhs);
        }

        float getWidth() const
        {
            return maxX - minX;
        }

        float getHeight() const
        {
            return maxY - minY;
        }

        float getDepth() const
        {
            return maxZ - minZ;
        }
    };

    struct Rect
    {
        int32_t minX, maxX;
        int32_t minY, maxY;

        Rect()
        : minX(0), maxX(0), minY(0), maxY(0)
        {}

        Rect(int32_t _minX, int32_t _maxX, int32_t _minY, int32_t _maxY)
        : minX(_minX), maxX(_maxX), minY(_minY), maxY(_maxY)
        {}

        bool operator==(const Rect &rhs) const
        {
            return minX == rhs.minX && minY == rhs.minY && maxX == rhs.maxX && maxY == rhs.maxY;
        }
        bool operator!=(const Rect &rhs) const
        {
            return !(*this == rhs);
        }

        int32_t getWidth() const
        {
            return maxX - minX;
        }

        int32_t getHeight() const
        {
            return maxY - minY;
        }
    };

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

    enum class ResourceStates : uint32_t {
        Unknown = 0,
        Common = 1 << 0,

        VertexBuffer = 1 << 1,
        IndexBuffer = 1 << 2,
        ConstantBuffer = 1 << 3,

        ShaderResource = 1 << 4,
        UnorderedAccess = 1 << 5,

        RenderTarget = 1 << 6,
        DepthWrite = 1 << 7,
        DepthRead = 1 << 8,

        CopySource = 1 << 9,
        CopyDestination = 1 << 10,

        Present = 1 << 11
    };

    ENUM_CLASS_FLAG_OPERATORS(ResourceStates)

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
        PRESENT_SRC_KHR = 17,
        COUNT = 18
    };

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

    struct Resolution {
        uint32_t width = 0;
        uint32_t height = 0;
    };

    enum class SamplerFilter {
        NEAREST = 0,
        LINEAR = 1,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class SamplerAddressMode {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        CLAMP_TO_BORDER = 3,
        MIRROR_CLAMP_TO_EDGE = 4,
        MODE_MAX_ENUM = 0x7FFFFFFF
    };

    struct SamplerDesc {
        SamplerAddressMode addressU = SamplerAddressMode::REPEAT;
        SamplerAddressMode addressV = SamplerAddressMode::REPEAT;
        SamplerAddressMode addressW = SamplerAddressMode::REPEAT;
        SamplerFilter minFilter = SamplerFilter::LINEAR;
        SamplerFilter magFilter = SamplerFilter::LINEAR;
        SamplerFilter mipFilter = SamplerFilter::LINEAR;
        bool anisotropyEnable = false;
        float maxAnisotropy = 1.0f;

        constexpr SamplerDesc &setAddressU(SamplerAddressMode value) {
            addressU = value;
            return *this;
        }

        constexpr SamplerDesc &setAddressV(SamplerAddressMode value) {
            addressV = value;
            return *this;
        }

        constexpr SamplerDesc &setAddressW(SamplerAddressMode value) {
            addressW = value;
            return *this;
        }

        constexpr SamplerDesc &setAddressAll(SamplerAddressMode value) {
            addressU = addressV = addressW = value;
            return *this;
        }

        constexpr SamplerDesc &setMinFilter(SamplerFilter value) {
            minFilter = value;
            return *this;
        }

        constexpr SamplerDesc &setMagFilter(SamplerFilter value) {
            magFilter = value;
            return *this;
        }

        constexpr SamplerDesc &setAnisotropyEnable(bool value) {
            anisotropyEnable = value;
            return *this;
        }

        constexpr SamplerDesc &setMaxAnisotropy(float value) {
            maxAnisotropy = value;
            return *this;
        }
    };

    class ISampler : public IResource {
      public:
        virtual const SamplerDesc &getDesc() const = 0;
    };

    struct ImageUsage {
        bool isTransferSrc = false;
        bool isTransferDst = false;
        bool isShaderResource = false;
        bool isRenderTarget = false;
        bool isUAV = false;
    };

    struct TextureDesc {
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mipLevels = 1;
        uint32_t layerCount = 1;
        uint32_t sampleCount = 1;
        Format format = Format::UNKNOWN;
        MemoryPropertiesBits memoryProperties = MemoryPropertiesBits::DEVICE_LOCAL_BIT;
        bool isLinearTiling = false;
        CreateFlagBits flags = CreateFlagBits::NONE_BIT;
        TextureDimension dimension = TextureDimension::Texture2D;
        ImageUsage usage = {};
        std::string debugName;

        ResourceStates initialState = ResourceStates::Unknown;
        bool keepInitialState = false;

        TextureDesc &setWidth(uint32_t value) {
            width = value;
            return *this;
        }

        TextureDesc &setHeight(uint32_t value) {
            height = value;
            return *this;
        }

        TextureDesc &setDepth(uint32_t value) {
            depth = value;
            return *this;
        }

        TextureDesc &setMipLevels(uint32_t value) {
            mipLevels = value;
            return *this;
        }

        TextureDesc &setLayerCount(uint32_t value) {
            layerCount = value;
            return *this;
        }

        TextureDesc &setSampleCount(uint32_t value) {
            sampleCount = value;
            return *this;
        }

        TextureDesc &setFormat(Format value) {
            format = value;
            return *this;
        }

        TextureDesc &setDimension(TextureDimension value) {
            dimension = value;
            return *this;
        }

        TextureDesc &setMemoryProperties(MemoryPropertiesBits value) {
            memoryProperties = value;
            return *this;
        }

        TextureDesc &setIsTransferSrc(bool value) {
            usage.isTransferSrc = value;
            return *this;
        }

        TextureDesc &setIsTransferDst(bool value) {
            usage.isTransferDst = value;
            return *this;
        }

        TextureDesc &setIsShaderResource(bool value) {
            usage.isShaderResource = value;
            return *this;
        }

        TextureDesc &setIsRenderTarget(bool value) {
            usage.isRenderTarget = value;
            return *this;
        }

        TextureDesc &setIsUAV(bool value) {
            usage.isUAV = value;
            return *this;
        }

        TextureDesc &setDebugName(const std::string &value) {
            debugName = value;
            return *this;
        }
    };

    struct TextureRegion {
        // Origin
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;

        // Dimensions, where -1 means the entire mip level or texture.
        uint32_t width = std::numeric_limits<uint32_t>::max();
        uint32_t height = std::numeric_limits<uint32_t>::max();
        uint32_t depth = std::numeric_limits<uint32_t>::max();

        uint32_t mipLevel = 0;
        uint32_t arrayLayer = 0;

        [[nodiscard]]
        TextureRegion resolveRegion(const TextureDesc &desc) const {
            TextureRegion result = *this;

            result.width =
                (width == std::numeric_limits<uint32_t>::max()) ? std::max(1u, desc.width >> mipLevel) : width;
            result.height =
                (height == std::numeric_limits<uint32_t>::max()) ? std::max(1u, desc.height >> mipLevel) : height;
            result.depth =
                (depth == std::numeric_limits<uint32_t>::max()) ? std::max(1u, desc.depth >> mipLevel) : depth;

            return result;
        }

        constexpr TextureRegion &setOrigin(uint32_t vx = 0, uint32_t vy = 0, uint32_t vz = 0) {
            x = vx;
            y = vy;
            z = vz;
            return *this;
        }

        constexpr TextureRegion &setSize(
            uint32_t vx = std::numeric_limits<uint32_t>::max(), uint32_t vy = std::numeric_limits<uint32_t>::max(),
            uint32_t vz = std::numeric_limits<uint32_t>::max()
        ) {
            width = vx;
            height = vy;
            depth = vz;
            return *this;
        }

        constexpr TextureRegion &setWidth(uint32_t value) {
            width = value;
            return *this;
        }

        constexpr TextureRegion &setHeight(uint32_t value) {
            height = value;
            return *this;
        }

        constexpr TextureRegion &setDepth(uint32_t value) {
            depth = value;
            return *this;
        }

        constexpr TextureRegion &setMipLevel(uint32_t value) {
            mipLevel = value;
            return *this;
        }

        constexpr TextureRegion &setArrayLayer(uint32_t value) {
            arrayLayer = value;
            return *this;
        }
    };

    struct TextureSubresource {
        static constexpr uint32_t kMaxMipLevel = std::numeric_limits<uint32_t>::max();
        static constexpr uint32_t kMaxArrayLayer = std::numeric_limits<uint32_t>::max();

        uint32_t mipLevel = 0;
        uint32_t mipLevelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t layerCount = 1;

        TextureSubresource() = default;

        TextureSubresource(uint32_t _mipLevel, uint32_t _mipLevelCount, uint32_t _baseArrayLayer, uint32_t _layerCount)
            : mipLevel(_mipLevel),
              mipLevelCount(_mipLevelCount),
              baseArrayLayer(_baseArrayLayer),
              layerCount(_layerCount) {
        }

        bool operator==(const TextureSubresource &other) const noexcept {
            return mipLevel == other.mipLevel && mipLevelCount == other.mipLevelCount &&
                   baseArrayLayer == other.baseArrayLayer && layerCount == other.layerCount;
        }

        TextureSubresource resolveTextureSubresource(const TextureDesc &desc) const;
    };

    static const TextureSubresource kAllSubresources =
        TextureSubresource(0, TextureSubresource::kMaxMipLevel, 0, TextureSubresource::kMaxArrayLayer);

    class ITexture : public IResource {
      public:
        virtual const TextureDesc &getDesc() const = 0;
    };

    enum class ResourceAttachmentType : uint8_t {
        None,
        Buffer,
        Texture,
        TextureArray,

        Count
    };

    struct DescriptorInfo {
        DescriptorInfo() {}

        DescriptorInfo(DescriptorType _type, ShaderStageFlagBits _shaderStageFlags)
            : type(_type),
              shaderStageFlags(_shaderStageFlags) {
        }

        DescriptorType type = DescriptorType::MAX_ENUM;
        ShaderStageFlagBits shaderStageFlags = ShaderStageFlagBits::VERTEX_BIT;
        union {
            TextureSubresource subresource;
        };
    };

    struct BufferAttachment
    {
        DescriptorInfo dInfo;
        IBuffer* buffer = nullptr;
        uint32_t offset;
        uint32_t size;

        BufferAttachment& setDescriptorInfo(const DescriptorInfo& value) { dInfo = value; return *this; }
        BufferAttachment& setBuffer(IBuffer* value) { buffer = value; return *this; }
        BufferAttachment& setOffset(uint32_t value) { offset = value; return *this; }
        BufferAttachment& setSize(uint32_t value) { size = value; return *this; }
    };

    struct TextureAttachment
    {
        DescriptorInfo dInfo;
        ITexture *texture = nullptr;
        ISampler *sampler = nullptr;

        TextureAttachment()
            : dInfo() {
            dInfo.subresource =
                TextureSubresource{ 0, TextureSubresource::kMaxMipLevel, 0, TextureSubresource::kMaxArrayLayer };
        }

        TextureAttachment &setDescriptorType(const DescriptorType &value) { dInfo.type = value; return *this; }
        TextureAttachment &setShaderStages(const ShaderStageFlagBits &value) { dInfo.shaderStageFlags = value; return *this; }
        TextureAttachment &setTextureSubresource(const TextureSubresource &value) { dInfo.subresource = value; return *this; }
        TextureAttachment &setTexture(ITexture *value) { texture = value; return *this; }
        TextureAttachment &setSampler(ISampler *value) { sampler = value; return *this; }
    };

    struct TextureArrayAttachment
    {
        DescriptorInfo dInfo;
        std::vector<ITexture*> textures;
        ISampler *sampler = nullptr;
        TextureSubresource subresource;

        TextureArrayAttachment &setTextures(const std::vector<ITexture *> &value) { textures = value; return *this; }
        TextureArrayAttachment &setSampler(ISampler *value) { sampler = value; return *this; }
        TextureArrayAttachment &setTextureSubresource(const TextureSubresource &value) { subresource = value; return *this; }
    };

    struct FramebufferAttachment
    {
        ITexture* texture = nullptr;
        TextureSubresource subresource = TextureSubresource{ 0, 1, 0, 1 };
        Format format = Format::UNKNOWN;

        FramebufferAttachment& setTexture(ITexture* value) { texture = value; return *this; }
        FramebufferAttachment& setTextureSubresourse(const TextureSubresource& value) { subresource = value; return *this; }
        FramebufferAttachment& setFormat(Format value) { format = value; return *this; }
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
        virtual const DescriptorSetInfo &getDesc() const = 0;
    };

    enum CommandQueue : uint8_t
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

    struct ViewportState
    {
        Viewport viewport;
        Rect scissorRect;

        ViewportState &setViewport(const Viewport &value) { viewport = value; return *this; }
        ViewportState &setScissorRect(const Rect &value) { scissorRect = value; return *this; }
    };

    struct GraphicsState
    {
        IGraphicsPipeline* pipeline = nullptr;
        IFramebuffer* framebuffer = nullptr;

        std::vector<IBindingSet*> bindingSets;
        std::vector<VertexBufferBinding> vertexBufferBindings;
        IndexBufferBinding indexBufferBinding;

        ViewportState viewport;
        Color blendColorFactor;
        uint8_t dynamicStencilReference = 0;

        GraphicsState& setPipeline(IGraphicsPipeline* value) { pipeline = value; return *this; }
        GraphicsState& setFramebuffer(IFramebuffer* value) { framebuffer = value; return *this; }
        GraphicsState& setViewport(const ViewportState& value) { viewport = value; return *this; }
        GraphicsState& setBlendColorFactor(const Color& value) { blendColorFactor = value; return *this; }
        GraphicsState& setDynamicStencilReference(const uint8_t &value) { dynamicStencilReference = value; return *this; }
        GraphicsState& setBindingSets(const std::vector<IBindingSet*>& value) { bindingSets = value; return *this; }
        GraphicsState& setVertexBufferBindings(const std::vector<VertexBufferBinding>& value) { vertexBufferBindings = value; return *this; }
        GraphicsState& addBindingSet(IBindingSet* value) { bindingSets.push_back(value); return *this; }
        GraphicsState& addVertexBufferBinding(const VertexBufferBinding& value) { vertexBufferBindings.push_back(value); return *this; }
        GraphicsState& setIndexBufferBinding(const IndexBufferBinding& value) { indexBufferBinding = value; return *this; }
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

    class IInstance : public IResource
    {

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
        std::string debugName;

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
        constexpr BufferDesc& setDebugName(const std::string& value) { debugName = value; return *this; }
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
        std::vector<FramebufferAttachment> colorAttachments;
        FramebufferAttachment depthAttachment;

        FramebufferDesc& addColorAttachment(const FramebufferAttachment& value) { colorAttachments.push_back(value); return *this; }
        FramebufferDesc& setDepthAttachment(const FramebufferAttachment& value) { depthAttachment = value; return *this; }
    };

    enum eRenderPassBit : uint8_t {
        eRenderPassBit_First = 0x01,             // clear the attachment
        eRenderPassBit_Last = 0x02,              // transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        eRenderPassBit_Offscreen = 0x04,         // transition to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        eRenderPassBit_OffscreenInternal = 0x08, // keep VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL
    };

    struct RenderPassCreateInfo final
    {
        bool clearColor = false;
        bool clearDepth = false;
        bool clearStencil = false;
        uint8_t flags = 0;
    };

    class IRenderPass : public IResource
    {

    };

    class IFramebuffer : public IResource
    {
    public:
        std::vector<RHI::ITexture *> textures;
        uint32_t framebufferWidth = -1;
        uint32_t framebufferHeight = -1;
        uint32_t sampleCount = 1;

        virtual const FramebufferDesc& getDesc() const = 0;

    private:
        IRenderPass* m_RenderPass = nullptr;
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

    enum class RasterizerCullMode : uint8_t
    {
        None,
        Back,
        Front
    };

    /* A structure with pipeline parameters */
    struct GraphicsPipelineInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t topology = 3; /* defaults to triangles*/

        bool dynamicScissorState = false;

        uint32_t patchControlPoints = 0;
    };

    enum BlendFactor
    {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
        CONSTANT_COLOR = 10,
        ONE_MINUS_CONSTANT_COLOR = 11,
        CONSTANT_ALPHA = 12,
        ONE_MINUS_CONSTANT_ALPHA = 13,
        SRC_ALPHA_SATURATE = 14,
        SRC1_COLOR = 15,
        ONE_MINUS_SRC1_COLOR = 16,
        SRC1_ALPHA = 17,
        ONE_MINUS_SRC1_ALPHA = 18,
    };

    enum BlendOp
    {
        ADD = 0,
        SUBTRACT = 1,
        REVERSE_SUBTRACT = 2,
        MIN = 3,
        MAX = 4,
    };

    enum class RasterizerFillMode : uint8_t
    {
        FILL = 0,
        LINE = 1,
        POINT = 2,
    };

    enum CompareOp {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class StencilOp {
        KEEP,
        ZERO,
        REPLACE,
        INCR_CLAMP,
        DECR_CLAMP,
        INVERT,
        INCR_WRAP,
        DECR_WRAP
    };

    enum class ColorMask : uint8_t {
        NONE = 0,
        R = 1 << 0,
        G = 1 << 1,
        B = 1 << 2,
        A = 1 << 3,

        RGB = R | G | B,
        RGBA = R | G | B | A,
    };
    ENUM_CLASS_FLAG_OPERATORS(ColorMask)

    struct DepthStencilState
    {
        struct StencilFaceState {
            StencilOp failOp = StencilOp::KEEP;
            StencilOp passOp = StencilOp::KEEP;
            StencilOp depthFailOp = StencilOp::KEEP;
            CompareOp compareOp = CompareOp::ALWAYS;
        };

        bool depthTestEnable = true;
        bool depthWriteEnable = true;
        CompareOp depthCompareOp = CompareOp::LESS_OR_EQUAL;
        bool stencilTestEnable = false;
        uint32_t compareMask = 0xFF;
        uint32_t writeMask = 0xFF;
        uint32_t reference = 0;
        bool dynamicStencilReferenceEnable = false;
        StencilFaceState front; // front face ops
        StencilFaceState back;  // back face ops
    };

    struct ColorBlendState {
        struct RenderTargetBlendState {
            bool blendEnable = false;

            BlendFactor srcColorBlendFactor = BlendFactor::SRC_ALPHA;
            BlendFactor dstColorBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;
            BlendOp colorBlendOp = BlendOp::ADD;

            BlendFactor srcAlphaBlendFactor = BlendFactor::ONE;
            BlendFactor dstAlphaBlendFactor = BlendFactor::ZERO;
            BlendOp alphaBlendOp = BlendOp::ADD;

            ColorMask colorWriteMask = ColorMask::RGBA;

            bool usesConstantColor(BlendFactor state) const {
                return state == BlendFactor::CONSTANT_COLOR || state == BlendFactor::ONE_MINUS_CONSTANT_COLOR ||
                       state == BlendFactor::CONSTANT_ALPHA || state == BlendFactor::ONE_MINUS_CONSTANT_ALPHA;
            }

            constexpr RenderTargetBlendState &setBlendEnable(bool value) { blendEnable = value; return *this; }
            constexpr RenderTargetBlendState &setSrcColorBlendFactor(BlendFactor value) { srcColorBlendFactor = value; return *this; }
            constexpr RenderTargetBlendState &setDstColorBlendFactor(BlendFactor value) { dstColorBlendFactor = value; return *this; }
            constexpr RenderTargetBlendState &setColorBlendOp(BlendOp value) { colorBlendOp = value; return *this; }
            constexpr RenderTargetBlendState &setSrcAlphaBlendFactor(BlendFactor value) { srcAlphaBlendFactor = value; return *this; }
            constexpr RenderTargetBlendState &setDstAlphaBlendFactor(BlendFactor value) { dstAlphaBlendFactor = value; return *this; }
            constexpr RenderTargetBlendState &setAlphaBlendOp(BlendOp value) { alphaBlendOp = value; return *this; }
            constexpr RenderTargetBlendState &setColorWriteMask(ColorMask value) { colorWriteMask = value; return *this; }
        };

        uint32_t renderTargetCount = 1;
        RenderTargetBlendState renderTargets[kMaxRenderTargets];

        constexpr ColorBlendState &setRenderTarget(int32_t index, const RenderTargetBlendState &renderTarget) {
            renderTargets[index] = renderTarget; return *this;
        }

        bool usesConstantColor() const {
            for (uint32_t i = 0; i < renderTargetCount; i++) {
                const auto &rt = renderTargets[i];

                if (!rt.blendEnable) {
                    continue;
                }

                if (rt.usesConstantColor(rt.srcColorBlendFactor) || rt.usesConstantColor(rt.dstColorBlendFactor) ||
                    rt.usesConstantColor(rt.srcAlphaBlendFactor) || rt.usesConstantColor(rt.dstAlphaBlendFactor)) {
                    return true;
                }
            }

            return false;
        }
    };

    struct RenderState
    {
        // rasterization
        RasterizerFillMode fillMode = RasterizerFillMode::FILL;
        RasterizerCullMode cullMode = RasterizerCullMode::Back;
        bool CCWCullMode = false;

        // depth stencil state
        DepthStencilState depthStencilState = {};

        // color blend state
        ColorBlendState colorBlendState = {};

        // multisampling
        bool multisampleAA = false;
    };

    struct PushConstantsDesc
    {
        uint32_t vtxConstSize = 0;
        uint32_t fragConstSize = 0;
    };

    struct GraphicsPipelineDesc
    {
        PrimitiveType primType = PrimitiveType::TriangleList;
        InputLayoutHandle inputLayout = nullptr;
        std::vector<BindingLayoutHandle> bindingLayouts;

        ShaderHandle VS = nullptr;
        ShaderHandle HS = nullptr;
        ShaderHandle DS = nullptr;
        ShaderHandle GS = nullptr;
        ShaderHandle PS = nullptr;

        RenderState renderState;

        GraphicsPipelineInfo pipelineInfo;
        PushConstantsDesc pushConstants;

        GraphicsPipelineDesc& setPrimType(PrimitiveType value) { primType = value; return *this; }
        GraphicsPipelineDesc& setInputLayout(IInputLayout* value) { inputLayout = InputLayoutHandle(value); return *this; }
        GraphicsPipelineDesc& setVertexShader(IShader* value) { VS = ShaderHandle(value); return *this; }
        GraphicsPipelineDesc& setTessallationControlShader(IShader* value) { HS = ShaderHandle(value); return *this; }
        GraphicsPipelineDesc& setTessallationEvaluationShader(IShader* value) { DS = ShaderHandle(value); return *this; }
        GraphicsPipelineDesc& setGeometryShader(IShader* value) { GS = ShaderHandle(value); return *this; }
        GraphicsPipelineDesc& setPixelShader(IShader* value) { PS = ShaderHandle(value); return *this; }
    };

    class IGraphicsPipeline : public IResource
    {
        virtual const GraphicsPipelineDesc& getDesc() const = 0;
    };

    class IRHICommandList : public IResource {
      public:
        virtual void beginSingleTimeCommands() = 0;
        virtual void endSingleTimeCommands() = 0;

        // Clears the graphics state of the underlying command list object and resets the state cache.
        virtual void clearState() = 0;

        virtual void queueWaitIdle() = 0;
        virtual void draw(const DrawArguments &args) = 0;
        virtual void drawIndexed(const DrawArguments &args) = 0;
        virtual void setGraphicsState(const GraphicsState &state) = 0;
        virtual void transitionBufferLayout(IBuffer *texture, ImageLayout oldLayout, ImageLayout newLayout) = 0;
        virtual bool updateTextureImage(
            ITexture *texture, uint32_t mipLevel, uint32_t baseArrayLayer, const void *imageData, size_t rowPitch = 0,
            size_t depthPitch = 0
        ) = 0;
        virtual void
        copyBufferToImage(IBuffer *buffer, ITexture *texture, uint32_t mipLevel = 0, uint32_t baseArrayLayer = 0) = 0;
        virtual void copyTexture(
            ITexture *srcTexture, const TextureSubresource &srcSubresource, const TextureRegion &srcRegion,
            ITexture *dstTexture, const TextureSubresource &dstSubresource, const TextureRegion &dstRegion
        ) = 0;
        virtual void blitTexture(
            ITexture *srcTexture, const TextureSubresource &srcSubresource, const TextureRegion &srcRegion,
            ITexture *dstTexture, const TextureSubresource &dstSubresource, const TextureRegion &dstRegion,
            RHI::SamplerFilter filter
        ) = 0;
        virtual void resolveTexture(
            ITexture *srcTexture, const TextureSubresource &srcSubresource, ITexture *dstTexture,
            const TextureSubresource dstSubresource
        ) = 0;
        virtual void
        clearColorTexture(ITexture *texture, const TextureSubresource &subresource, const Color &color) = 0;
        virtual void clearDepthStencilTexture(
            ITexture *texture, TextureSubresource subresources, bool clearDepth, bool clearStencil, float depthValue,
            uint32_t stencilValue
        ) = 0;
        virtual void clearAttachments(
            std::vector<ITexture *> colorAttachments, ITexture *depthAttachment, const std::vector<Rect> &rects
        ) = 0;
        virtual void copyMIPBufferToImage(IBuffer *buffer, ITexture *texture) = 0;
        virtual void copyBuffer(IBuffer *srcBuffer, IBuffer *dstBuffer, size_t size) = 0;
        virtual void writeBuffer(IBuffer *srcBuffer, size_t size, const void *data) = 0;
        virtual void setPushConstants(const void *data, size_t byteSize) = 0;

        virtual void
        beginTrackingTextureState(ITexture *texture, TextureSubresource subresource, ResourceStates states) = 0;
        virtual void setTextureState(ITexture *texture, TextureSubresource subresource, ResourceStates states) = 0;
        virtual void setPermanentTextureState(ITexture *texture, ResourceStates states) = 0;

        virtual void commitBarriers() = 0;
    };

    class IDevice : public IResource
    {
    public:
        virtual CommandListHandle createCommandList(const CommandListParameters& params = CommandListParameters()) = 0;
        virtual uint64_t executeCommandLists(std::vector<IRHICommandList*>& commandLists, size_t numCommandLists, CommandQueue executionQueue = CommandQueue::Graphics) = 0;
        virtual bool waitForIdle() = 0;
        virtual void runGarbageCollection() = 0;
        virtual GraphicsAPI getGraphicsAPI() const = 0;
        virtual IRenderPass* createRenderPass(const FramebufferDesc& framebufferDesc, const RenderPassCreateInfo& ci = RenderPassCreateInfo()) = 0;
        virtual FramebufferHandle createFramebuffer(IRenderPass* renderPass, const FramebufferDesc& desc) = 0;
        virtual GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer) = 0;
        virtual ShaderHandle createShaderModule(const char* fileName, const std::vector<unsigned int>& SPIRV) = 0;
        virtual BindingLayoutHandle createDescriptorSetLayout(const DescriptorSetInfo& dsInfo) = 0;
        virtual BindingSetHandle createDescriptorSet(const DescriptorSetInfo& dsInfo, uint32_t dSetCount, IBindingLayout* bindingLayout) = 0;
        virtual void updateDescriptorSet(IBindingSet *ds, const DescriptorSetInfo &dsInfo) = 0;
        virtual InputLayoutHandle createInputLayout(const VertexInputAttributeDesc* attributes, uint32_t attributeCount, const VertexInputBindingDesc* bindings, uint32_t bindingCount) = 0;
        virtual TextureHandle createImage(const TextureDesc& desc) = 0;
        virtual SamplerHandle createTextureSampler(const SamplerDesc& desc = SamplerDesc()) = 0;
        virtual SamplerHandle createDepthSampler() = 0;
        virtual BufferHandle createBuffer(const BufferDesc& desc) = 0;
        virtual BufferHandle createSharedBuffer(const BufferDesc& desc) = 0;
        virtual BufferHandle addBuffer(const BufferDesc& desc, bool createMapping = false) = 0;
        virtual void uploadBufferData(IBuffer* buffer, size_t deviceOffset, const void* data, const size_t dataSize) = 0;
        virtual void uploadVertexIndexBufferData(IBuffer* buffer, size_t deviceOffset, size_t vertexDataSize, const void* vertexData,
            size_t indexDataSize, const void* indexData, const size_t dataSize) = 0;
        virtual void uploadMipLevelToStagingBuffer(IBuffer* stagingBuffer, size_t deviceOffset, const void* imageData, const size_t imageSize,
            uint32_t deviceNumRows, uint32_t deviceNumColumns, uint32_t mipDepth, uint32_t layerCount,
            size_t rowPitch,     // from source
            size_t depthPitch,   // from source
            size_t deviceRowSize) = 0; // GPU expected row size
        virtual Format findDepthFormat() = 0;
        // mapping
        virtual void* mapBufferMemory(IBuffer* buffer, size_t offset, size_t size) = 0;
        virtual void* mapStagingTextureMemory(ITexture* texture, size_t offset, size_t size) = 0;
        virtual void unmapBufferMemory(IBuffer* buffer) = 0;
        virtual void unmapStagingTextureMemory(ITexture* texture) = 0;

        uint64_t executeCommandList(IRHICommandList* commandList, CommandQueue executionQueue = CommandQueue::Graphics)
        {
            std::vector<IRHICommandList*> commandLists{ commandList };
            return executeCommandLists(commandLists, 1, executionQueue);
        }
    };

    struct DeviceParams {
        bool useGraphicsQueue = true;
        bool useComputeQueue = false;
        bool useTransferQueue = false;
        bool usePresentQueue = false;

        bool enableDebugRuntime = false;

        uint32_t backBufferWidth = 0;
        uint32_t backBufferHeight = 0;
        uint32_t maxFramesInFlight = 2;

        bool backBufferUseDepth = false;
        RenderPassCreateInfo renderPassCreateInfo = {};

        bool vSyncEnabled = false;
        bool supportScreenshots = false;

        std::vector<const char *> requiredVulkanInstanceExtensions;
    };

    class IDynamicRHI
    {
    public:
        IDynamicRHI(const DeviceParams &deviceParams)
            : m_DeviceParams(deviceParams) {
        }

        virtual ~IDynamicRHI() = default;

        bool CreateDevice();
        virtual bool BeginFrame() = 0;
        virtual bool Present() = 0;

    protected:
        virtual void createDeviceInternal() = 0;
        virtual void ResizeSwapChain() = 0;

    public:
        void BackBufferResizing();
        void BackBufferResized();
        virtual RHI::ITexture *GetCurrentBackBuffer() = 0;
        virtual RHI::ITexture *GetBackBuffer(uint32_t index) = 0;
        virtual RHI::ITexture *GetDepthBuffer() = 0;
        virtual uint32_t GetCurrentBackBufferIndex() = 0;
        virtual uint32_t GetBackBufferCount() = 0;
        virtual RHI::IFramebuffer *GetCurrentFramebuffer() = 0;
        virtual RHI::IFramebuffer *GetFramebuffer(uint32_t index) = 0;
        virtual DeviceHandle getDevice() const = 0;
        virtual GraphicsAPI getGraphicsAPI() const = 0;

        virtual DeviceParams &getDeviceParams() {
            return m_DeviceParams;
        }

    protected:
        DeviceParams m_DeviceParams;
        std::vector<FramebufferHandle> m_SwapChainFramebuffers;
    };

    class IRHIModule {
    public:
        IRHIModule() = default;
        virtual ~IRHIModule() = default;

        virtual IDynamicRHI *createRHI(const DeviceParams &deviceParams) = 0;
        virtual void *getWindowInterface() = 0;
    };

}
