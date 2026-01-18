#pragma once

#include <RHICommon.hpp>
#include <memory>
#include <unordered_map>

namespace RHI
{

struct TextureStateInfo {
    const TextureDesc &descReference;
    ResourceStates permanentState = ResourceStates::Unknown;
    bool stateInitialized = false;

    explicit TextureStateInfo(const TextureDesc &desc)
        : descReference(desc) {
    }
};

struct TextureState {
    std::vector<ResourceStates> subresourceStates;
    ResourceStates state = ResourceStates::Unknown;
    bool enableUavBarriers = true;
    bool firstUavBarrierPlaced = false;
    bool permanentTransition = false;
};

struct TextureBarrier {
    TextureStateInfo *texture = nullptr;
    uint32_t mipLevel = 0;
    uint32_t arraySlice = 0;
    bool entireTexture = false;
    ResourceStates stateBefore = ResourceStates::Unknown;
    ResourceStates stateAfter = ResourceStates::Unknown;
};

class CommandListStateTracker {
  public:
    CommandListStateTracker() = default;

    void beginTrackingTextureState(TextureStateInfo *texture, TextureSubresource subresources, ResourceStates states);

    void setPermanentTextureState(TextureStateInfo *texture, TextureSubresource subresources, ResourceStates states);

    ResourceStates getTextureSubresourceState(TextureStateInfo *texture, uint32_t arraySlice, uint32_t mipLevel);

    TextureState *getTextureStateTracking(TextureStateInfo *texture, bool allowCreate);

    void requireTextureState(TextureStateInfo *texture, const TextureSubresource &subresources, ResourceStates requiredState);

    void keepTextureInitialStates();
    void commandListSubmitted();

    const std::vector<TextureBarrier> &getTextureBarriers() const {
        return m_TextureBarriers;
    }

    void clearBarriers() {
        m_TextureBarriers.clear();
    }

  private:
    std::unordered_map<TextureStateInfo *, std::unique_ptr<TextureState>> m_TextureStates;

    std::vector<std::pair<TextureStateInfo *, ResourceStates>> m_PermanentTextureStates;

    std::vector<TextureBarrier> m_TextureBarriers;
};

}
