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

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.blendEnable = desc.renderState.blendEnable ? VK_TRUE : VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = convertBlendFactor(desc.renderState.srcColorBlendFactor);
        colorBlendAttachment.dstColorBlendFactor = convertBlendFactor(desc.renderState.dstColorBlendFactor);
        colorBlendAttachment.colorBlendOp = convertBlendOp(desc.renderState.colorBlendOp);
        colorBlendAttachment.srcAlphaBlendFactor = convertBlendFactor(desc.renderState.srcAlphaBlendFactor);
        colorBlendAttachment.dstAlphaBlendFactor = convertBlendFactor(desc.renderState.dstAlphaBlendFactor);
        colorBlendAttachment.alphaBlendOp = convertBlendOp(desc.renderState.alphaBlendOp);
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
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

        VkDynamicState dynamicStateElt = VK_DYNAMIC_STATE_SCISSOR;

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pNext = nullptr;
        dynamicState.flags = 0;
        dynamicState.dynamicStateCount = 1;
        dynamicState.pDynamicStates = &dynamicStateElt;

        VkPipelineTessellationStateCreateInfo tessellationState{};
        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.pNext = nullptr;
        tessellationState.flags = 0;
        tessellationState.patchControlPoints = pipeInfo.patchControlPoints;

        // TODO: refactor push constant visibility logic
        pso->pushConstantsVisibility = VkShaderStageFlagBits();
        if (desc.pushConstants.vtxConstSize > 0)
            pso->pushConstantsVisibility = VK_SHADER_STAGE_VERTEX_BIT;
        if (desc.pushConstants.fragConstSize > 0)
            pso->pushConstantsVisibility = VK_SHADER_STAGE_FRAGMENT_BIT;
        
        BindingLayout* bindingLayout = dynamic_cast<BindingLayout*>(desc.bindingLayouts[0].get());
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts = { bindingLayout->descriptorSetLayout };
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
        const VkPushConstantRange ranges[] =
        {
            {
                    VK_SHADER_STAGE_VERTEX_BIT,		// stageFlags
                    0,								// offset
                    constantsDesc.vtxConstSize		// size
            },

            {
                    VK_SHADER_STAGE_FRAGMENT_BIT,	// stageFlags
                    constantsDesc.vtxConstSize,		// offset
                    constantsDesc.fragConstSize		// size
            }
        };

        uint32_t constSize = (constantsDesc.vtxConstSize > 0) + (constantsDesc.fragConstSize > 0);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext = nullptr;
        pipelineLayoutInfo.flags = 0;
        pipelineLayoutInfo.setLayoutCount = dsLayouts.size();
        pipelineLayoutInfo.pSetLayouts = dsLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = constSize;
        pipelineLayoutInfo.pPushConstantRanges = (constSize == 0) ? nullptr :
            (constantsDesc.vtxConstSize > 0) ? ranges : &ranges[1];

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

        vkCmdBindPipeline(m_CurrentCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

        std::vector<VkBuffer> vertexBuffers;
        std::vector<VkDeviceSize> vertexBuffersOffsets{};
        vertexBuffers.reserve(state.vertexBufferBindings.size());
        vertexBuffersOffsets.reserve(state.vertexBufferBindings.size());
        uint32_t maxVertexBufferIndex = 0;
        for(auto& vertexBinding : state.vertexBufferBindings)
        {
	        if(Buffer* vertexBuffer = dynamic_cast<Buffer*>(vertexBinding.buffer))
	        {
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
