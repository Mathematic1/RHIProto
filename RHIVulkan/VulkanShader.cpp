#include <VulkanBackend.hpp>

#include <ShaderUtils.hpp>

namespace RHI::Vulkan
{
    VkShaderStageFlagBits glslangShaderStageToVulkan(glslang_stage_t sh)
    {
        switch (sh)
        {
        case GLSLANG_STAGE_VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case GLSLANG_STAGE_FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case GLSLANG_STAGE_GEOMETRY:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case GLSLANG_STAGE_TESSCONTROL:
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case GLSLANG_STAGE_TESSEVALUATION:
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case GLSLANG_STAGE_COMPUTE:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        }

        return VK_SHADER_STAGE_VERTEX_BIT;
    }

    glslang_stage_t glslangShaderStageFromFileName(const char* fileName)
    {
        if (ShaderUtils::endsWith(fileName, ".vert"))
            return GLSLANG_STAGE_VERTEX;

        if (ShaderUtils::endsWith(fileName, ".frag"))
            return GLSLANG_STAGE_FRAGMENT;

        if (ShaderUtils::endsWith(fileName, ".geom"))
            return GLSLANG_STAGE_GEOMETRY;

        if (ShaderUtils::endsWith(fileName, ".comp"))
            return GLSLANG_STAGE_COMPUTE;

        if (ShaderUtils::endsWith(fileName, ".tesc"))
            return GLSLANG_STAGE_TESSCONTROL;

        if (ShaderUtils::endsWith(fileName, ".tese"))
            return GLSLANG_STAGE_TESSEVALUATION;

        return GLSLANG_STAGE_VERTEX;
    }

    static size_t compileShader(glslang_stage_t stage, const char* shaderSource, Shader& shaderModule)
    {
        const glslang_input_t input{
                GLSLANG_SOURCE_GLSL,
                stage,
                GLSLANG_CLIENT_VULKAN,
                GLSLANG_TARGET_VULKAN_1_1,
                GLSLANG_TARGET_SPV,
                GLSLANG_TARGET_SPV_1_3,
                shaderSource,
                100,
                GLSLANG_NO_PROFILE,
                false,
                false,
                GLSLANG_MSG_DEFAULT_BIT,
                (const glslang_resource_t*)glslang_default_resource()
        };

        glslang_shader_t* shader = glslang_shader_create(&input);

        if (!glslang_shader_preprocess(shader, &input))
        {
            fprintf(stderr, "GLSL preprocessing failed\n");
            fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
            fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
            ShaderUtils::printShaderSource(input.code);
            return 0;
        }

        if (!glslang_shader_parse(shader, &input))
        {
            fprintf(stderr, "GLSL parsing failed\n");
            fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
            fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
            ShaderUtils::printShaderSource(glslang_shader_get_preprocessed_code(shader));
            return 0;
        }

        glslang_program_t* program = glslang_program_create();
        glslang_program_add_shader(program, shader);

        if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
        {
            fprintf(stderr, "GLSL linking failed\n");
            fprintf(stderr, "\n%s", glslang_program_get_info_log(program));
            fprintf(stderr, "\n%s", glslang_program_get_info_debug_log(program));
            return 0;
        }

        glslang_program_SPIRV_generate(program, stage);

        shaderModule.SPIRV.resize(glslang_program_SPIRV_get_size(program));
        glslang_program_SPIRV_get(program, shaderModule.SPIRV.data());

        {
            const char* spirv_messages = glslang_program_SPIRV_get_messages(program);

            if (spirv_messages)
                fprintf(stderr, "%s", spirv_messages);
        }

        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return shaderModule.SPIRV.size();
    }

    size_t compileShaderFile(const char* file, Shader& shaderModule)
    {
        if (auto shaderSource = ShaderUtils::readShaderFile(file); !shaderSource.empty())
            return compileShader(glslangShaderStageFromFileName(file), shaderSource.c_str(), shaderModule);

        return 0;
    }

    std::shared_ptr<IShader> Device::createShaderModule(const char* fileName)
    {
        std::shared_ptr<Shader> shader = std::make_shared<Shader>();

        if (compileShaderFile(fileName, *shader) < 1)
        {
            //return VK_NOT_READY;
            return nullptr;
        }

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shader->SPIRV.size() * sizeof(unsigned int);
        createInfo.pCode = shader->SPIRV.data();

        vkCreateShaderModule(m_Context.device, &createInfo, nullptr, &shader->shaderModule);

        return shader;
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
