#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    ShaderHandle Device::createShaderModule(const char* fileName, const std::vector<unsigned int>& SPIRV)
    {
        Shader* shader = new Shader();

        shader->SPIRV = SPIRV;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader->SPIRV.size() * sizeof(unsigned int);
        createInfo.pCode = shader->SPIRV.data();

        vkCreateShaderModule(m_Context.device, &createInfo, nullptr, &shader->shaderModule);

        return ShaderHandle(shader);
    }

    uint32_t InputLayout::getNumAttributes() const
    {
        return inputAttributeDesc.size();
    }

    const VertexInputAttributeDesc* InputLayout::getVertexAttributeDesc(uint32_t index) const
    {
        if (index < uint32_t(inputAttributeDesc.size()))
            return &inputAttributeDesc[index];
        else
            return nullptr;
    }

    uint32_t InputLayout::getNumBindings() const
    {
        return inputBindingDesc.size();
    }

    const VertexInputBindingDesc* InputLayout::getVertexBindingDesc(uint32_t index) const
    {
        if (index < uint32_t(inputBindingDesc.size()))
            return &inputBindingDesc[index];
        else
            return nullptr;
    }
}
