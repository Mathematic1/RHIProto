#pragma once

#include <VulkanBackend.hpp>

namespace RHI
{
	inline IRHIModule* InitializeModuleRHI(const GraphicsAPI& gapiType)
	{
		if (gapiType == GraphicsAPI::VULKAN)
		{
			return new RHI::Vulkan::VulkanRHIModule();
		}
		return nullptr;
	}
}
