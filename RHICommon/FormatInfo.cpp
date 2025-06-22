#include <RHICommon.hpp>
#include <cassert>

namespace RHI
{
    static const FormatInfo c_FormatInfo[] = {
        { Format::UNKNOWN, 0, 0},
        { Format::R8_UINT, 1, 1},
        { Format::R8_SINT, 1, 1},
        { Format::R8_UNORM, 1, 1},
        { Format::R8_SNORM, 1, 1},
        { Format::RG8_UINT, 2, 1},
        { Format::RG8_SINT, 2, 1},
        { Format::RG8_UNORM, 2, 1},
        { Format::RG8_SNORM, 2, 1},
        { Format::R16_UINT, 2, 1},
        { Format::R16_SINT, 2, 1},
        { Format::R16_UNORM, 2, 1},
        { Format::R16_SNORM, 2, 1},
        { Format::R16_FLOAT, 2, 1},
        { Format::BGRA4_UNORM, 2, 1},
        { Format::B5G6R5_UNORM, 2, 1 },
        { Format::B5G5R5A1_UNORM, 2, 1},
        { Format::RGBA8_UINT, 4, 1},
        { Format::RGBA8_SINT, 4, 1},
        { Format::RGBA8_UNORM, 4, 1},
        { Format::RGBA8_SNORM, 4, 1},
        { Format::BGRA8_UNORM, 4, 1},
        { Format::SRGBA8_UNORM, 4, 1},
        { Format::SBGRA8_UNORM, 4, 1},
        { Format::R10G10B10A2_UNORM,4, 1},
        { Format::R11G11B10_FLOAT, 4, 1},
        { Format::RG16_UINT, 4, 1},
        { Format::RG16_SINT, 4, 1},
        { Format::RG16_UNORM, 4, 1},
        { Format::RG16_SNORM, 4, 1},
        { Format::RG16_FLOAT, 4, 1},
        { Format::R32_UINT, 4, 1},
        { Format::R32_SINT, 4, 1},
        { Format::R32_FLOAT, 4, 1},
        { Format::RGBA16_UINT, 8, 1},
        { Format::RGBA16_SINT, 8, 1},
        { Format::RGBA16_FLOAT, 8, 1},
        { Format::RGBA16_UNORM, 8, 1},
        { Format::RGBA16_SNORM, 8, 1},
        { Format::RG32_UINT, 8, 1},
        { Format::RG32_SINT, 8, 1},
        { Format::RG32_FLOAT, 8, 1},
        { Format::RGB32_UINT, 12, 1},
        { Format::RGB32_SINT, 12, 1},
        { Format::RGB32_FLOAT, 12, 1},
        { Format::RGBA32_UINT, 16, 1},
        { Format::RGBA32_SINT, 16, 1},
        { Format::RGBA32_FLOAT, 16, 1},
        { Format::D16, 2, 1},
        { Format::D24S8, 4, 1},
        { Format::X24G8_UINT, 4, 1},
        { Format::D32, 4, 1},
        { Format::D32S8, 8, 1},
        { Format::X32G8_UINT, 8, 1},
        { Format::BC1_UNORM, 8, 4},
        { Format::BC1_UNORM_SRGB, 8, 4},
        { Format::BC2_UNORM, 16, 4},
        { Format::BC2_UNORM_SRGB, 16, 4},
        { Format::BC3_UNORM, 16, 4},
        { Format::BC3_UNORM_SRGB, 16, 4},
        { Format::BC4_UNORM, 8, 4},
        { Format::BC4_SNORM, 8, 4},
        { Format::BC5_UNORM, 16, 4},
        { Format::BC5_SNORM, 16, 4},
        { Format::BC6H_UFLOAT, 16, 4},
        { Format::BC6H_SFLOAT, 16, 4},
        { Format::BC7_UNORM, 16, 4},
        { Format::BC7_UNORM_SRGB, 16, 4},
    };

    const FormatInfo& getFormatInfo(Format format)
    {
        static_assert(sizeof(c_FormatInfo) / sizeof(FormatInfo) == size_t(Format::COUNT),
            "The format info table doesn't have the right number of elements");

        if (uint32_t(format) >= uint32_t(Format::COUNT))
            return c_FormatInfo[0]; // UNKNOWN

        const FormatInfo& info = c_FormatInfo[uint32_t(format)];
        assert(info.format == format);
        return info;
    }
}
