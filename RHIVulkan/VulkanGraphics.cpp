#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    void countShaders(IShader* shader, uint32_t& numShaders)
    {
	    if(!shader)
	    {
		    return;
	    }

        numShaders++;
    }

    static std::vector<VkVertexInputBindingDescription> getBindingDescription(IInputLayout *inputLayout)
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        bindingDescriptions.reserve(inputLayout->getNumBindings());

        for (uint32_t idx = 0; idx < inputLayout->getNumBindings(); idx++) {
            const VertexInputBindingDesc &description = *inputLayout->getVertexBindingDesc(idx);
            VkVertexInputBindingDescription bindingDescription;
            bindingDescription.binding = description.binding;
            bindingDescription.stride = description.stride;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescriptions.push_back(bindingDescription);
        }

        return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(IInputLayout* inputLayout) {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.reserve(inputLayout->getNumAttributes());

        for(uint32_t idx = 0; idx < inputLayout->getNumAttributes(); idx++)
        {
            const VertexInputAttributeDesc& description = *inputLayout->getVertexAttributeDesc(idx);
            VkVertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = description.binding;
            attributeDescription.location = description.location;
            attributeDescription.format = convertFormat(description.format);
            attributeDescription.offset = description.offset;
            attributeDescriptions.push_back(attributeDescription);
        }

        return attributeDescriptions;
    }

    GraphicsPipelineHandle Device::createGraphicsPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer)
    {
        GraphicsPipeline* pso = new GraphicsPipeline(m_Context);
        pso->desc = desc;
        const GraphicsPipelineInfo& pipeInfo = desc.pipelineInfo;

        Framebuffer* fb = dynamic_cast<Framebuffer*>(framebuffer);

        //std::vector<Shader> localShaderModules;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        uint32_t numShaders = 0;
        countShaders(desc.VS.get(), numShaders);
        countShaders(desc.HS.get(), numShaders);
        countShaders(desc.DS.get(), numShaders);
        countShaders(desc.GS.get(), numShaders);
        countShaders(desc.PS.get(), numShaders);

        shaderStages.reserve(numShaders);
        /*localShaderModules.resize(numShaders);

        for (size_t i = 0; i < numShaders; i++)
        {
            const char* file = shaderFiles[i];

            auto idx = m_Resources.shaderMap.find(file);

            if (idx != m_Resources.shaderMap.end())
            {
                // printf("Already compiled file (%s)\n", file);
                localShaderModules[i] = m_Resources.shaderModules[idx->second];
            }
            else
            {
                VK_CHECK(createShaderModule(&localShaderModules[i], file));
                m_Resources.shaderModules.push_back(localShaderModules[i]);
                m_Resources.shaderMap[std::string(file)] = (int)m_Resources.shaderModules.size() - 1;
            }

            VkShaderStageFlagBits stage = glslangShaderStageToVulkan(glslangShaderStageFromFileName(file));

            shaderStages[i] = shaderStageInfo(stage, localShaderModules[i], "main");
        }*/

        if (Shader* shader = dynamic_cast<Shader*>(desc.VS.get()))
        {
            //shaderStages.push_back(shaderStageInfo(shader->stage, *shader, "main"));
            shaderStages.push_back(shaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, *shader, "main"));
        }
        if (Shader* shader = dynamic_cast<Shader*>(desc.PS.get()))
        {
            //shaderStages.push_back(shaderStageInfo(shader->stage, *shader, "main"));
            shaderStages.push_back(shaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, *shader, "main"));
        }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescriptions = getBindingDescription(desc.inputLayout.get());
        auto attributeDescriptions = getAttributeDescriptions(desc.inputLayout.get());

        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data(); // optional
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        /* The only difference from createGraphicsPipeline() */
        inputAssembly.topology = (VkPrimitiveTopology)pipeInfo.topology;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(pipeInfo.width > 0 ? pipeInfo.width : m_DeviceDesc.framebufferWidth);
        viewport.height = static_cast<float>(pipeInfo.height > 0 ? pipeInfo.height : m_DeviceDesc.framebufferHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = { pipeInfo.width > 0 ? pipeInfo.width : m_DeviceDesc.framebufferWidth, pipeInfo.height > 0 ? pipeInfo.height : m_DeviceDesc.framebufferHeight };

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = convertFillMode(desc.renderState.polygonFillMode);
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = desc.renderState.CCWCullMode ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        rasterizer.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = (VkSampleCountFlagBits) fb->sampleCount;
        multisampling.sampleShadingEnable = VK_FALSE; // enable sample shading in the pipeline
        multisampling.minSampleShading = 1.0f; // min fraction for sample shading; closer to one is smooth
        multisampling.pSampleMask = nullptr; // optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // optional
        multisampling.alphaToOneEnable = VK_FALSE; // optional

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(fb->desc.colorAttachments.size());
        for (uint32_t i = 0; i < uint32_t(fb->desc.colorAttachments.size()); i++) {
            colorBlendAttachments[i] = convertBlendState(desc.renderState.colorBlendState.renderTargets[i]);
        }

        pso->usesBlendConstants = desc.renderState.colorBlendState.usesConstantColor();

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        const DepthStencilState& depthStencilState = desc.renderState.depthStencilState;
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = static_cast<VkBool32>(depthStencilState.depthTestEnable ? VK_TRUE : VK_FALSE);
        depthStencil.depthWriteEnable = static_cast<VkBool32>(depthStencilState.depthWriteEnable ? VK_TRUE : VK_FALSE);
        depthStencil.depthCompareOp = convertCompareOp(depthStencilState.depthCompareOp);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = depthStencilState.stencilTestEnable;
        depthStencil.front = convertStencilState(depthStencilState, depthStencilState.front);
        depthStencil.back = convertStencilState(depthStencilState, depthStencilState.back);

        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        if (depthStencilState.dynamicStencilReferenceEnable) {
            dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        }

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext = nullptr;
        dynamicState.flags = 0;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineTessellationStateCreateInfo tessellationState{};
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = nullptr;
        tessellationState.flags = 0;
        tessellationState.patchControlPoints = pipeInfo.patchControlPoints;

        // TODO: refactor push constant visibility logic
        pso->pushConstantsVisibility = 0;

        if (desc.pushConstants.vtxConstSize > 0)
            pso->pushConstantsVisibility |= VK_SHADER_STAGE_VERTEX_BIT;

        if (desc.pushConstants.fragConstSize > 0)
            pso->pushConstantsVisibility |= VK_SHADER_STAGE_FRAGMENT_BIT;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        descriptorSetLayouts.reserve(desc.bindingLayouts.size());
        for (auto &bindingLayoutHandle : desc.bindingLayouts) {
            BindingLayout *bindingLayout = dynamic_cast<BindingLayout *>(bindingLayoutHandle.get());
            descriptorSetLayouts.push_back(bindingLayout->descriptorSetLayout);
        }
        createPipelineLayout(descriptorSetLayouts, desc.pushConstants, &pso->pipelineLayout);

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pTessellationState = ((VkPrimitiveTopology)pipeInfo.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) ? &tessellationState : nullptr;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = pipeInfo.dynamicScissorState ? &dynamicState : nullptr;
        pipelineInfo.layout = pso->pipelineLayout;
        pipelineInfo.renderPass = fb->renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        checkSuccess(
            vkCreateGraphicsPipelines(m_Context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pso->pipeline));

        return GraphicsPipelineHandle(pso);
    }

    VkPipeline Device::addPipeline(const GraphicsPipelineDesc& desc, IFramebuffer* framebuffer)
    {
        VkPipeline pipeline;

        if (!this->createGraphicsPipeline(desc, framebuffer))
        {
            printf("Cannot create graphics pipeline\n");
            exit(EXIT_FAILURE);
        }

        m_Resources.allPipelines.push_back(pipeline);
        return pipeline;
    }


    bool Device::createPipelineLayout(std::vector<VkDescriptorSetLayout>& dsLayouts, const PushConstantsDesc& constantsDesc, VkPipelineLayout* pipelineLayout)
    {
        std::vector<VkPushConstantRange> ranges;

        const uint32_t totalSize = constantsDesc.vtxConstSize + constantsDesc.fragConstSize;

        if (totalSize > 0) {
            VkShaderStageFlags stages = 0;

            if (constantsDesc.vtxConstSize > 0)
                stages |= VK_SHADER_STAGE_VERTEX_BIT;

            if (constantsDesc.fragConstSize > 0)
                stages |= VK_SHADER_STAGE_FRAGMENT_BIT;

            ranges.push_back({stages, 0, totalSize});
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext = nullptr;
        pipelineLayoutInfo.flags = 0;
        pipelineLayoutInfo.setLayoutCount = dsLayouts.size();
        pipelineLayoutInfo.pSetLayouts = dsLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
        pipelineLayoutInfo.pPushConstantRanges = ranges.empty() ? nullptr : ranges.data();

        return (vkCreatePipelineLayout(m_Context.device, &pipelineLayoutInfo, nullptr, pipelineLayout) == VK_SUCCESS);
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        if (pipeline) {
            vkDestroyPipeline(m_Context.device, pipeline, nullptr);
            pipeline = nullptr;
        }

        if (pipelineLayout) {
            vkDestroyPipelineLayout(m_Context.device, pipelineLayout, nullptr);
            pipelineLayout = nullptr;
        }
    }

    void CommandList::beginRenderPass(Framebuffer* framebuffer)
    {
        VkRect2D rect = {};
        rect.offset = VkOffset2D(0, 0);
        rect.extent = VkExtent2D(framebuffer->framebufferWidth, framebuffer->framebufferHeight);

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0] = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1] = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = framebuffer->renderPass;
        renderPassInfo.framebuffer = framebuffer->framebuffer; //(fb != VK_NULL_HANDLE) ? fb : swapchainFramebuffers[currentImage];
        renderPassInfo.renderArea = rect;
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        //renderPassInfo.clearValueCount = clearValueCount;
        //renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(m_CurrentCommandBuffer->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandList::endRenderPass()
    {
        if(m_CurrentGraphicsState.framebuffer)
        {
            vkCmdEndRenderPass(m_CurrentCommandBuffer->commandBuffer);
            m_CurrentGraphicsState.framebuffer = nullptr;
        }
    }

    void CommandList::setGraphicsState(const GraphicsState& state)
    {
        GraphicsPipeline* pipeline = dynamic_cast<GraphicsPipeline*>(state.pipeline);
        Framebuffer* fb = dynamic_cast<Framebuffer*>(state.framebuffer);

        if(state.framebuffer != m_CurrentGraphicsState.framebuffer)
        {
            endRenderPass();
        }

        if (!m_CurrentGraphicsState.framebuffer)
        {
            beginRenderPass(fb);
        }

        bool updatePipeline = false;
        if (m_CurrentGraphicsState.pipeline != state.pipeline) {
            updatePipeline = true;
            vkCmdBindPipeline(
                m_CurrentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline
            );
        }

        if (m_CurrentGraphicsState.viewport.viewport != state.viewport.viewport) {
            const Viewport &vp = state.viewport.viewport;
            const VkViewport viewport = VkViewport{vp.minX, vp.minY, vp.getWidth(), vp.getHeight(), vp.minZ, vp.maxZ};
            vkCmdSetViewport(m_CurrentCommandBuffer->commandBuffer, 0, 1, &viewport);
        }

        if (m_CurrentGraphicsState.viewport.scissorRect != state.viewport.scissorRect) {
            const Rect &sc = state.viewport.scissorRect;
            const VkRect2D scissor = VkRect2D{VkOffset2D{sc.minX, sc.minY}, VkExtent2D{static_cast<uint32_t>(std::abs(sc.getWidth())),
                                                                  static_cast<uint32_t>(std::abs(sc.getHeight()))}};
            vkCmdSetScissor(m_CurrentCommandBuffer->commandBuffer, 0, 1, &scissor);
        }

        if (pipeline->desc.renderState.depthStencilState.dynamicStencilReferenceEnable &&
            (updatePipeline || m_CurrentGraphicsState.dynamicStencilReference != state.dynamicStencilReference)) {
            vkCmdSetStencilReference(m_CurrentCommandBuffer->commandBuffer, VK_STENCIL_FRONT_AND_BACK, state.dynamicStencilReference);
        }

        if (pipeline->usesBlendConstants && (updatePipeline || m_CurrentGraphicsState.blendColorFactor != state.blendColorFactor)) {
            vkCmdSetBlendConstants(m_CurrentCommandBuffer->commandBuffer, &state.blendColorFactor.r);
        }

        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkDeviceSize> vertexBuffersOffsets{};
        vertexBuffers.reserve(state.vertexBufferBindings.size());
        vertexBuffersOffsets.reserve(state.vertexBufferBindings.size());
        uint32_t maxVertexBufferIndex = 0;
        for(auto& vertexBinding : state.vertexBufferBindings)
        {
            if(Buffer* vertexBuffer = dynamic_cast<Buffer*>(vertexBinding.buffer)){
                vertexBuffers.push_back(vertexBuffer->buffer);
                vertexBuffersOffsets.push_back(vertexBinding.offset);
                maxVertexBufferIndex = std::max(vertexBinding.bindingSlot, maxVertexBufferIndex);
            }
        }

        if (state.indexBufferBinding.buffer && m_CurrentGraphicsState.indexBufferBinding.buffer != state.indexBufferBinding.buffer) {
            Buffer *indexBuf = dynamic_cast<Buffer *>(state.indexBufferBinding.buffer);
            vkCmdBindIndexBuffer(m_CurrentCommandBuffer->commandBuffer, indexBuf->buffer,
                                 state.indexBufferBinding.offset,
                                 state.indexBufferBinding.index32BitType ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        }

        if (!vertexBuffers.empty()) {
            vkCmdBindVertexBuffers(m_CurrentCommandBuffer->commandBuffer, 0, maxVertexBufferIndex + 1,
                                   vertexBuffers.data(), vertexBuffersOffsets.data()); 
        }

        GraphicsPipeline* pso = dynamic_cast<GraphicsPipeline*>(state.pipeline);
        m_CurrentPipelineLayout = pso->pipelineLayout;
        m_CurrentPushConstantsVisibility = pso->pushConstantsVisibility;

        bindBindingSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pso->pipelineLayout, state.bindingSets);

        m_CurrentGraphicsState = state;
    }
}
