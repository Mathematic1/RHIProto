#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    VkResult Device::createComputePipeline(VkShaderModule computeShader, VkPipelineLayout pipelineLayout, VkPipeline* pipeline)
    {
        // ShaderStageInfo, just like in graphics pipeline, but with a single COMPUTE stage
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.pNext = nullptr;
        shaderStageCreateInfo.flags = 0;
        shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStageCreateInfo.module = computeShader;
        shaderStageCreateInfo.pName = "main";
        /* we don't use specialization */
        shaderStageCreateInfo.pSpecializationInfo = nullptr;

        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.flags = 0;
        computePipelineCreateInfo.stage = shaderStageCreateInfo;
        computePipelineCreateInfo.layout = pipelineLayout;
        computePipelineCreateInfo.basePipelineHandle = 0;
        computePipelineCreateInfo.basePipelineIndex = 0;

        /* no caching, single pipeline creation*/
        return vkCreateComputePipelines(m_Context.device, 0, 1, &computePipelineCreateInfo, nullptr, pipeline);
    }
}
