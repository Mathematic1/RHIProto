#include <VulkanBackend.hpp>

namespace RHI::Vulkan
{
    TrackedCommandBuffer::~TrackedCommandBuffer()
    {
        if (commandPool) {
            vkDestroyCommandPool(m_Context.device, commandPool, nullptr);
        }
    }

    Queue::Queue(const VulkanContext &context, CommandQueue queueID, VkQueue queue, uint32_t queueFamilyIndex)
        : m_Context(context), m_Queue(queue), m_QueueID(queueID), m_QueueFamilyIndex(queueFamilyIndex)
    {
        // const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkSemaphoreTypeCreateInfo timelineCreateInfo;
        timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineCreateInfo.pNext = NULL;
        timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineCreateInfo.initialValue = 0;

        VkSemaphoreCreateInfo semaphoreCreateInfo;
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = &timelineCreateInfo;
        semaphoreCreateInfo.flags = 0;
        vkCreateSemaphore(m_Context.device, &semaphoreCreateInfo, nullptr, &trackingSemaphore);
    }

    Queue::~Queue()
    {
        vkDestroySemaphore(m_Context.device, trackingSemaphore, nullptr);
        trackingSemaphore = VkSemaphore();
    }

    uint64_t Queue::submit(std::vector<IRHICommandList *> &commandLists, size_t numCommandLists)
    {
        std::vector<VkPipelineStageFlags> waitStageArray(m_WaitSemaphores.size());
        std::vector<VkCommandBuffer> commandBuffers(numCommandLists);

        for (size_t i = 0; i < m_WaitSemaphores.size(); i++) {
            waitStageArray[i] = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        m_LastSubmittedID++;

        for (size_t i = 0; i < numCommandLists; i++) {
            if (CommandList *commandList = dynamic_cast<CommandList *>(commandLists[i])) {
                if (TrackedCommandBufferPtr commandBuffer = commandList->getCurrentCommandBuffer()) {
                    commandBuffers[i] = commandBuffer->commandBuffer;
                    // TODO:fix memory leaking
                    m_CommandBuffersInFlight.push_back(commandBuffer);
                }
            }
        }

        m_SignalSemaphores.push_back(trackingSemaphore);
        m_SignalSemaphoreValues.push_back(m_LastSubmittedID);

        VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
                                                            .pNext = nullptr,
                                                            .waitSemaphoreValueCount = 0,
                                                            .pWaitSemaphoreValues = nullptr,
                                                            .signalSemaphoreValueCount =
                                                                static_cast<uint32_t>(m_SignalSemaphoreValues.size()),
                                                            .pSignalSemaphoreValues = m_SignalSemaphoreValues.data()};

        if (!m_WaitSemaphoreValues.empty()) {
            timelineSubmitInfo.waitSemaphoreValueCount = static_cast<uint32_t>(m_WaitSemaphoreValues.size());
            timelineSubmitInfo.pWaitSemaphoreValues = m_WaitSemaphoreValues.data();
        }

        VkSubmitInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.pNext = &timelineSubmitInfo;
        si.waitSemaphoreCount = uint32_t(m_WaitSemaphores.size());
        si.pWaitSemaphores = m_WaitSemaphores.data();
        si.pWaitDstStageMask = waitStageArray.data();
        si.commandBufferCount = uint32_t(commandBuffers.size());
        si.pCommandBuffers = commandBuffers.data();
        si.signalSemaphoreCount = uint32_t(m_SignalSemaphores.size());
        si.pSignalSemaphores = m_SignalSemaphores.data();

        VkResult result = vkQueueSubmit(m_Queue, 1, &si, nullptr);
        VK_CHECK(result);

        m_WaitSemaphores.clear();
        m_WaitSemaphoreValues.clear();
        m_SignalSemaphores.clear();
        m_SignalSemaphoreValues.clear();

        return m_LastSubmittedID;
    }

    uint64_t Queue::updateLastFinishedID()
    {
        vkGetSemaphoreCounterValue(m_Context.device, trackingSemaphore, &m_LastFinishedID);

        return m_LastFinishedID;
    }

    void Queue::retireCommandBuffers()
    {
        std::list<TrackedCommandBufferPtr> submissions = std::move(m_CommandBuffersInFlight);

        uint64_t lastFinishedID = updateLastFinishedID();

        for (const TrackedCommandBufferPtr &cmd : submissions) {
            if (cmd->submissionID <= lastFinishedID) {
                cmd->referencedStagingBuffers.clear();
                cmd->submissionID = 0;
                m_CommandBuffersPool.push_back(cmd);
            } else {
                m_CommandBuffersInFlight.push_back(std::move(cmd));
            }
        }
    }

    TrackedCommandBufferPtr Queue::createCommandBuffer()
    {
        TrackedCommandBufferPtr commandBuffer = std::make_shared<TrackedCommandBuffer>(m_Context);

        VkCommandPoolCreateInfo cpi{};
        cpi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cpi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        cpi.queueFamilyIndex = m_QueueFamilyIndex;

        VK_CHECK(vkCreateCommandPool(m_Context.device, &cpi, nullptr, &commandBuffer->commandPool));

        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.pNext = nullptr;
        ai.commandPool = commandBuffer->commandPool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = static_cast<uint32_t>(1);

        VK_CHECK(vkAllocateCommandBuffers(m_Context.device, &ai, &commandBuffer->commandBuffer));

        return commandBuffer;
    }

    TrackedCommandBufferPtr Queue::getOrCreateCommandBuffer()
    {
        std::lock_guard lock_guard(m_Mutex); // this is called from CommandList::open, so free-threaded

        TrackedCommandBufferPtr commandBuffer;
        if (m_CommandBuffersPool.empty()) {
            commandBuffer = createCommandBuffer();
            //m_CommandBuffersPool.push_back(commandBuffer);
        } else {
            commandBuffer = m_CommandBuffersPool.front();
            // TODO:fix memory leaking
            m_CommandBuffersPool.pop_front();
        }

        return commandBuffer;
    }

    void Queue::addWaitSemaphore(VkSemaphore semaphore, uint64_t value)
    {
        if (!semaphore)
            return;

        m_WaitSemaphores.push_back(semaphore);
        m_WaitSemaphoreValues.push_back(value);
    }

    void Queue::addSignalSemaphore(VkSemaphore semaphore, uint64_t value)
    {
        if (!semaphore)
            return;

        m_SignalSemaphores.push_back(semaphore);
        m_SignalSemaphoreValues.push_back(value);
    }

    VkSemaphore Device::getQueueSemaphore(CommandQueue queueID)
    {
        Queue &queue = *m_Queues[uint32_t(queueID)];

        return queue.trackingSemaphore;
    }

    void Device::queueWaitForSemaphore(CommandQueue waitQueueID, VkSemaphore semaphore, uint64_t value)
    {
        Queue &queue = *m_Queues[uint32_t(waitQueueID)];

        queue.addWaitSemaphore(semaphore, value);
    }

    void Device::queueSignalSemaphore(CommandQueue executionQueueID, VkSemaphore semaphore, uint64_t value)
    {
        Queue &queue = *m_Queues[uint32_t(executionQueueID)];

        queue.addSignalSemaphore(semaphore, value);
    }

}
