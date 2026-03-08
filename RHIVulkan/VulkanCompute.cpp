#include <VulkanBackend.hpp>

#include <Common/Miscellaneous.hpp>
#include <cassert>

namespace RHI::Vulkan
{
    ComputePipelineHandle Device::createComputePipeline(const ComputePipelineDesc& desc)
    {
        ComputePipeline* pso = new ComputePipeline(m_Context);
        pso->desc = desc;

        for (const BindingLayoutHandle& layout : desc.bindingLayouts)
        {
            BindingLayout* bindingLayout = dynamic_cast<BindingLayout*>(layout.get());
            pso->pipelineBindingLayouts.push_back(bindingLayout);
        }

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        for (auto binding : desc.bindingLayouts) {
            BindingLayout* layout = dynamic_cast<BindingLayout*>(binding.get());
            descriptorSetLayouts.push_back(layout->descriptorSetLayout);
        }

        std::vector<VkPushConstantRange> ranges;

        const PushConstantsDesc &constantsDesc = desc.pushConstants;
        const uint32_t totalSize = constantsDesc.vtxConstSize + constantsDesc.fragConstSize;

        if (totalSize > 0) {
            ranges.push_back({VK_SHADER_STAGE_COMPUTE_BIT, 0, totalSize});
        }

        pso->pushConstantsVisibility = totalSize > 0 ? VK_SHADER_STAGE_COMPUTE_BIT : 0;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        pipelineLayoutCreateInfo.pPushConstantRanges = ranges.empty() ? nullptr : ranges.data();

        vkCreatePipelineLayout(m_Context.device, &pipelineLayoutCreateInfo, nullptr, &pso->pipelineLayout);

        uint32_t numShaders = 0;
        countShaders(desc.CS.get(), numShaders);
        assert(numShaders == 1);

        Shader* shader = dynamic_cast<Shader*>(desc.CS.get());
        VkPipelineShaderStageCreateInfo shaderStage =
             shaderStage = shaderStageInfo(VK_SHADER_STAGE_COMPUTE_BIT, *shader, "main");

        VkComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.pNext = nullptr;
        computePipelineCreateInfo.flags = 0;
        computePipelineCreateInfo.stage = shaderStage;
        computePipelineCreateInfo.layout = pso->pipelineLayout;
        computePipelineCreateInfo.basePipelineHandle = 0;
        computePipelineCreateInfo.basePipelineIndex = 0;

        vkCreateComputePipelines(m_Context.device, 0, 1, &computePipelineCreateInfo, nullptr, &pso->pipeline);

        return ComputePipelineHandle(pso);
    }

    ComputePipeline::~ComputePipeline() {
        if (pipeline)
        {
            vkDestroyPipeline(m_Context.device, pipeline, nullptr);
            pipeline = nullptr;
        }

        if (pipelineLayout)
        {
            vkDestroyPipelineLayout(m_Context.device, pipelineLayout, nullptr);
            pipelineLayout = nullptr;
        }
    }

    void CommandList::setComputeState(const ComputeState& state) {
        endRenderPass();

        assert(m_CurrentCommandBuffer);

        ComputePipeline* pipeline = dynamic_cast<ComputePipeline*>(state.pipeline);

        if (m_EnableAutoBarriers && arraysAreDifferent(state.bindings, m_CurrentComputeState.bindings))
        {
            for (size_t i = 0; i < state.bindings.size() && i < pipeline->desc.bindingLayouts.size(); i++)
            {
                if (m_EnableAutoBarriers)
                {
                    setResourceStatesForBindingSet(state.bindings[i]);
                }
            }
        }

        if (m_CurrentComputeState.pipeline != state.pipeline)
        {
            vkCmdBindPipeline(m_CurrentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);

            m_CurrentCommandBuffer->referencedResources.push_back(state.pipeline);
        }

        if (arraysAreDifferent(m_CurrentComputeState.bindings, state.bindings))
        {
            bindBindingSets(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, state.bindings);
        }

        m_CurrentPipelineLayout = pipeline->pipelineLayout;
        m_CurrentPushConstantsVisibility = pipeline->pushConstantsVisibility;

        commitBarriers();

        m_CurrentGraphicsState = {};
        m_CurrentComputeState = state;
    }

    void CommandList::dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) {
        assert(m_CurrentCommandBuffer);

        vkCmdDispatch(m_CurrentCommandBuffer->commandBuffer, groupsX, groupsY, groupsZ);
    }
}
