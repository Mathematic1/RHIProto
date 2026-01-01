#include <Common/ResourcesStateTracking.hpp>

namespace RHI {
namespace {
uint32_t calcSubresource(uint32_t mipLevel, uint32_t arraySlice, uint32_t mipCount) {
    return arraySlice * mipCount + mipLevel;
}

bool isEntireTexture(const TextureSubresource &sub, const TextureDesc &desc) {
    if (sub.mipLevel != 0 || sub.mipLevelCount != desc.mipLevels) {
        return false;
    }

    switch (desc.dimension) {
    case TextureDimension::Texture1DArray:
    case TextureDimension::Texture2DArray:
    case TextureDimension::TextureCube:
    case TextureDimension::TextureCubeArray:
    case TextureDimension::Texture2DMSArray:
        if (sub.baseArrayLayer != 0 || sub.layerCount != desc.layerCount) {
            return false;
        }
        break;

    default:
        break;
    }

    return true;
}
}

void CommandListStateTracker::beginTrackingTextureState(
    TextureStateInfo *texture, TextureSubresource subresources, ResourceStates states
) {
    const TextureDesc &desc = texture->descReference;

    TextureState *tracking = getTextureStateTracking(texture, true);

    subresources = subresources.resolveTextureSubresource(desc);

    if (isEntireTexture(subresources, desc)) {
        tracking->state = states;
        tracking->subresourceStates.clear();
    } else {
        tracking->subresourceStates.resize(desc.mipLevels * desc.layerCount, tracking->state);
        tracking->state = ResourceStates::Unknown;

        for (uint32_t mipLevel = subresources.mipLevel; mipLevel < subresources.mipLevel + subresources.mipLevelCount;
             mipLevel++) {
            for (uint32_t arraySlice = subresources.baseArrayLayer;
                 arraySlice < subresources.baseArrayLayer + subresources.layerCount;
                 arraySlice++) {
                uint32_t subresource = calcSubresource(mipLevel, arraySlice, desc.mipLevels);
                tracking->subresourceStates[subresource] = states;
            }
        }
    }
}

void CommandListStateTracker::setPermanentTextureState(TextureStateInfo *texture, TextureSubresource subresources, ResourceStates states) {
    const TextureDesc &desc = texture->descReference;

    subresources = subresources.resolveTextureSubresource(desc);

    bool permanent = true;
    if (!isEntireTexture(subresources, desc)) {
        permanent = false;
    }

    requireTextureState(texture, subresources, states);

    if (permanent) {
        m_PermanentTextureStates.emplace_back(texture, states);
        getTextureStateTracking(texture, true)->permanentTransition = true;
    }
}

TextureState *CommandListStateTracker::getTextureStateTracking(TextureStateInfo *texture, bool allowCreate) {
    auto it = m_TextureStates.find(texture);

    if (it != m_TextureStates.end()) {
        return it->second.get();
    }

    if (!allowCreate) {
        return nullptr;
    }

    std::unique_ptr<TextureState> tracking = std::make_unique<TextureState>();

    TextureState *trackingState = tracking.get();
    m_TextureStates.insert(std::make_pair(texture, std::move(tracking)));

    if (texture->descReference.keepInitialState) {
        trackingState->state = texture->stateInitialized ? texture->descReference.initialState : ResourceStates::Common;
    }

    return trackingState;
}

ResourceStates CommandListStateTracker::getTextureSubresourceState(
    TextureStateInfo *texture, uint32_t arraySlice, uint32_t mipLevel
) {
    TextureState *tracking = getTextureStateTracking(texture, false);

    if (!tracking) {
        return ResourceStates::Unknown;
    }

    uint32_t subresource = calcSubresource(mipLevel, arraySlice, texture->descReference.mipLevels);
    return tracking->subresourceStates[subresource];
}

void CommandListStateTracker::requireTextureState(
    TextureStateInfo *texture, const TextureSubresource &subresources, ResourceStates requiredState
) {
    if (texture->permanentState != 0) {
        return;
    }

    TextureState *tracking = getTextureStateTracking(texture, true);
    const TextureDesc &desc = texture->descReference;

    TextureSubresource resolved = subresources.resolveTextureSubresource(desc);

    const bool entireTexture = isEntireTexture(resolved, desc);

    if (entireTexture && tracking->subresourceStates.empty()) {
        ResourceStates before = tracking->state;

        if (before == requiredState) {
            return;
        }

        TextureBarrier barrier{};
        barrier.texture = texture;
        barrier.entireTexture = true;
        barrier.stateBefore = before;
        barrier.stateAfter = requiredState;

        m_TextureBarriers.push_back(barrier);

        tracking->state = requiredState;
        tracking->subresourceStates.clear();
        return;
    } else {
        if (tracking->subresourceStates.empty()) {
            tracking->subresourceStates.resize(desc.mipLevels * desc.layerCount, tracking->state);
            tracking->state = ResourceStates::Unknown;
        }

        for (uint32_t layer = resolved.baseArrayLayer; layer < resolved.baseArrayLayer + resolved.layerCount; ++layer) {
            for (uint32_t mip = resolved.mipLevel; mip < resolved.mipLevel + resolved.mipLevelCount; ++mip) {
                uint32_t idx = layer * desc.mipLevels + mip;

                ResourceStates before = tracking->subresourceStates[idx];
                if (before == requiredState) {
                    continue;
                }

                TextureBarrier barrier{};
                barrier.texture = texture;
                barrier.entireTexture = false;
                barrier.arraySlice = layer;
                barrier.mipLevel = mip;
                barrier.stateBefore = before;
                barrier.stateAfter = requiredState;

                m_TextureBarriers.push_back(barrier);

                tracking->subresourceStates[idx] = requiredState;
            }
        }
    }
}

void CommandListStateTracker::commandListSubmitted() {
    for (auto [texture, state] : m_PermanentTextureStates) {
        if (texture->permanentState != 0 && texture->permanentState != state) {

            continue;
        }

        texture->permanentState = state;
    }
    m_PermanentTextureStates.clear();

    m_TextureStates.clear();
}

}
