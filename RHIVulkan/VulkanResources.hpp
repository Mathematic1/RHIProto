#pragma once

#define VK_NO_PROTOTYPES
#include <Volk/volk.h>

#include <vector>

/**
    For more or less abstract descriptor set setup we need to describe individual items ("bindings").
    These are buffers, textures (samplers, but we call them "textures" here) and arrays of textures.
*/

/// Common information for all bindings (buffer/sampler type and shader stage(s) where this item is used)

namespace RHI::Vulkan
{
}
