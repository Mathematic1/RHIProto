#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    InputLayoutHandle Device::createInputLayout(
        const VertexInputAttributeDesc* attributes, uint32_t attributeCount,
        const VertexInputBindingDesc* bindings, uint32_t bindingCount)
    {
        InputLayout* inputLayout = new InputLayout();
        inputLayout->inputAttributeDesc.reserve(attributeCount);
        for(uint32_t i = 0; i < attributeCount; ++i)
        {
            inputLayout->inputAttributeDesc.push_back(attributes[i]);
        }

        inputLayout->inputBindingDesc.reserve(bindingCount);
        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            inputLayout->inputBindingDesc.push_back(bindings[i]);
        }

        return InputLayoutHandle(inputLayout);
    }

    VkDescriptorPool Device::createDescriptorPool(const DescriptorSetInfo& dsInfo, uint32_t dSetCount)
    {
        uint32_t uniformBufferCount = 0;
        uint32_t storageBufferCount = 0;
        uint32_t samplerCount = static_cast<uint32_t>(dsInfo.textures.size());

        for (const auto& ta : dsInfo.textureArrays)
            samplerCount += static_cast<uint32_t>(ta.textures.size());

        for (const auto& b : dsInfo.buffers)
        {
            if (b.dInfo.type == DescriptorType::UNIFORM_BUFFER)
                uniformBufferCount++;
            if (b.dInfo.type == DescriptorType::STORAGE_BUFFER  )
                storageBufferCount++;
        }

        std::vector<VkDescriptorPoolSize> poolSizes;

        /* printf("Allocating pool[%d | %d | %d]\n", (int)uniformBufferCount, (int)storageBufferCount, (int)samplerCount); */

        if (uniformBufferCount)
            poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, dSetCount * uniformBufferCount });

        if (storageBufferCount)
            poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, dSetCount * storageBufferCount });

        if (samplerCount)
            poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, dSetCount * samplerCount });

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pNext = nullptr;
        poolInfo.flags = 0;
        poolInfo.maxSets = static_cast<uint32_t>(dSetCount);
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data();

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

        if (vkCreateDescriptorPool(m_Context.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            printf("Cannot allocate descriptor pool\n");
            exit(EXIT_FAILURE);
        }

        //m_Resources.allDPools.push_back(descriptorPool);
        return descriptorPool;
    }


    BindingLayoutHandle Device::createDescriptorSetLayout(const DescriptorSetInfo& dsInfo)
    {
        BindingLayout* bindingLayout = new BindingLayout(m_Context);

        VkDescriptorSetLayout descriptorSetLayout;

        uint32_t bindingIdx = 0;

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        for (const auto& b : dsInfo.buffers)
        {
            bindings.push_back(descriptorSetLayoutBinding(bindingIdx++, convertDescriptorType(b.dInfo.type), pickShaderStage(b.dInfo.shaderStageFlags)));
        }

        for (const auto& i : dsInfo.textures)
        {
            bindings.push_back(descriptorSetLayoutBinding(bindingIdx++, convertDescriptorType(i.dInfo.type) /*VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER*/, pickShaderStage(i.dInfo.shaderStageFlags)));
        }

        for (const auto& t : dsInfo.textureArrays)
        {
            bindings.push_back(descriptorSetLayoutBinding(bindingIdx++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, pickShaderStage(t.dInfo.shaderStageFlags), static_cast<uint32_t>(t.textures.size())));
        }

        //std::vector<VkDescriptorBindingFlags> descriptorBindingFlags(bindings.size(), 0);
        //descriptorBindingFlags.back() =
        //    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
        ///*std::vector<VkDescriptorBindingFlags> descriptorBindingFlags = {
        //    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};*/
        //const VkDescriptorSetLayoutBindingFlagsCreateInfoEXT setLayoutBindingFlags = {
        //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        //    .bindingCount = static_cast<uint32_t>(descriptorBindingFlags.size()/*bindings.size()*/),
        //    .pBindingFlags = descriptorBindingFlags.data() };


        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pNext = nullptr; //dsInfo.textureArrays.empty() ? nullptr : &setLayoutBindingFlags;
        layoutInfo.flags = 0;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.empty() ? nullptr : bindings.data();

        if (vkCreateDescriptorSetLayout(m_Context.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            printf("Failed to create descriptor set layout\n");
            exit(EXIT_FAILURE);
        }

        //m_Resources.allDSLayouts.push_back(descriptorSetLayout);
        bindingLayout->descriptorSetLayout = descriptorSetLayout;

        return BindingLayoutHandle(bindingLayout);
    }

    BindingSetHandle Device::createDescriptorSet(const DescriptorSetInfo& dsInfo, uint32_t dSetCount, IBindingLayout* bindingLayout)
    {
        BindingSet* bindingSet = new BindingSet(m_Context);
        bindingSet->descriptorPool = createDescriptorPool(dsInfo);

        BindingLayout* dsLayout = dynamic_cast<BindingLayout*>(bindingLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = bindingSet->descriptorPool;
        allocInfo.descriptorSetCount = dSetCount;
        allocInfo.pSetLayouts = &dsLayout->descriptorSetLayout;

        if (vkAllocateDescriptorSets(m_Context.device, &allocInfo, &bindingSet->descriptorSet) != VK_SUCCESS)
        {
            printf("Cannot allocate descriptor set\n");
            exit(EXIT_FAILURE);
        }

        updateDescriptorSet(bindingSet, dsInfo);

        return BindingSetHandle(bindingSet);
    }

    /*
        This routine counts all textures in all texture arrays (if any of them are present),
        creates a list of DescriptorWrite operations with required buffer/image info structures
        and calls the vkUpdateDescriptorSets()
    */
    void Device::updateDescriptorSet(IBindingSet* ds, const DescriptorSetInfo& dsInfo)
    {
        BindingSet *bindingSet = dynamic_cast<BindingSet *>(ds);
        bindingSet->desc = dsInfo;

        uint32_t bindingIdx = 0;
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        std::vector<VkDescriptorBufferInfo> bufferDescriptors(dsInfo.buffers.size());
        std::vector<VkDescriptorImageInfo> imageDescriptors(dsInfo.textures.size());
        std::vector<VkDescriptorImageInfo> imageArrayDescriptors;

        for (size_t i = 0; i < dsInfo.buffers.size(); i++)
        {
            BufferAttachment b = dsInfo.buffers[i];

            Buffer* buffer = dynamic_cast<Buffer*>(b.buffer);

            bufferDescriptors[i] = VkDescriptorBufferInfo{
                buffer->buffer,
                b.offset,
                (b.size > 0) ? b.size : VK_WHOLE_SIZE
            };

            descriptorWrites.push_back(bufferWriteDescriptorSet(bindingSet->descriptorSet, &bufferDescriptors[i],
                                                                bindingIdx++, convertDescriptorType(b.dInfo.type)));
        }

        for (size_t i = 0; i < dsInfo.textures.size(); i++)
        {
            Texture* tex = dynamic_cast<Texture*>(dsInfo.textures[i].texture);
            Sampler* sampler = dynamic_cast<Sampler*>(dsInfo.textures[i].sampler);
            TextureSubresource subresource = dsInfo.textures[i].dInfo.subresource.resolveTextureSubresource(tex->getDesc());
            TextureView *subresourceView = tex->GetOrCreateSubresourceView(subresource);

            imageDescriptors[i] =
                VkDescriptorImageInfo{ sampler->sampler,
                                       subresourceView->imageView,
                                       /* t.texture.layout */ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

            descriptorWrites.push_back(
                imageWriteDescriptorSet(bindingSet->descriptorSet, &imageDescriptors[i], bindingIdx++));

            if (!tex->permanentState) {
                bindingSet->texturesWithoutPermanentState.emplace_back(i);
            }else {
                
            }
        }

        uint32_t taOffset = 0;
        std::vector<uint32_t> taOffsets(dsInfo.textureArrays.size());
        for (size_t ta = 0; ta < dsInfo.textureArrays.size(); ta++)
        {
            taOffsets[ta] = taOffset;

            for (size_t j = 0; j < dsInfo.textureArrays[ta].textures.size(); j++)
            {
                Texture* tex = dynamic_cast<Texture*>(dsInfo.textureArrays[ta].textures[j]);
                Sampler *sampler = dynamic_cast<Sampler *>(dsInfo.textureArrays[ta].sampler);
                TextureSubresource subresource =
                    dsInfo.textureArrays[ta].subresource.resolveTextureSubresource(tex->getDesc());
                TextureView *subresourceView = tex->GetOrCreateSubresourceView(subresource);

                VkDescriptorImageInfo imageInfo = {
                    sampler->sampler,
                    subresourceView->imageView,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };

                imageArrayDescriptors.push_back(imageInfo); // item 'taOffsets[ta] + j'
            }

            taOffset += static_cast<uint32_t>(dsInfo.textureArrays[ta].textures.size());
        }

        for (size_t ta = 0; ta < dsInfo.textureArrays.size(); ta++)
        {
            VkWriteDescriptorSet writeSet{};
            writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeSet.pNext = nullptr;
            writeSet.dstSet = bindingSet->descriptorSet;
            writeSet.dstBinding = bindingIdx++;
            writeSet.dstArrayElement = 0;
            writeSet.descriptorCount = static_cast<uint32_t>(dsInfo.textureArrays[ta].textures.size());
            writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeSet.pImageInfo = imageArrayDescriptors.data() + taOffsets[ta];

            descriptorWrites.push_back(writeSet);
        }

        vkUpdateDescriptorSets(m_Context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }


    void CommandList::bindBindingSets(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, const std::vector<IBindingSet*> bindingSets)
    {
        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.reserve(bindingSets.size());
        
        for(IBindingSet* bindingSet : bindingSets)
        {
            if(BindingSet* binding = dynamic_cast<BindingSet*>(bindingSet))
            {
                descriptorSets.push_back(binding->descriptorSet);
            }
        }

        vkCmdBindDescriptorSets(m_CurrentCommandBuffer->commandBuffer, bindPoint, pipelineLayout, 0, uint32_t(descriptorSets.size()), descriptorSets.data() , 0, nullptr);
    }

    BindingLayout::~BindingLayout()
    {
        if (descriptorSetLayout) {
            vkDestroyDescriptorSetLayout(m_Context.device, descriptorSetLayout, nullptr);
            descriptorSetLayout = VkDescriptorSetLayout();
        }
    }

    BindingSet::BindingSet(const VulkanContext& context)
	    : m_Context(context)
    {}

    BindingSet::~BindingSet() {
        if (descriptorPool) {
            vkDestroyDescriptorPool(m_Context.device, descriptorPool, nullptr);
            descriptorPool = VkDescriptorPool();
            descriptorSet = VkDescriptorSet();
        }
    }
}
