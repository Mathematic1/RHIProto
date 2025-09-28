cmake_minimum_required(VERSION 3.24)

project(rhiProto VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
	set(RHI ON)
	set(RHI_Vulkan ON)
endif ()

if(WIN32)
	set(RHI ON)
	set(RHI_DX12 OFF)
	set(RHI_Vulkan ON)
endif ()

list(APPEND Rhi_Source_File_Ext
        /*.cpp
        /*.hpp
        /*.ini)

if(${RHI})
	set(Rhi_Sources_Ext ${Rhi_Source_File_Ext})
	list(TRANSFORM Rhi_Sources_Ext PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/RHICommon)
	file(GLOB_RECURSE Rhi_Common_Sources ${Rhi_Sources_Ext})
    list(APPEND RhiProto_Sources ${Rhi_Common_Sources})
    list(APPEND RhiProto_Include_Dir ${CMAKE_CURRENT_SOURCE_DIR}/RHICommon)
endif ()

if(${RHI_Vulkan})
	set(Rhi_Sources_Ext ${Rhi_Source_File_Ext})
	list(TRANSFORM Rhi_Sources_Ext PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/RHIVulkan)
	file(GLOB_RECURSE Rhi_Vulkan_Sources ${Rhi_Sources_Ext})
    list(APPEND RhiProto_Sources ${Rhi_Vulkan_Sources})
    list(APPEND RhiProto_Include_Dir ${CMAKE_CURRENT_SOURCE_DIR}/RHIVulkan)
endif()

if(${RHI_DX12})
	set(Rhi_Sources_Ext ${Rhi_Source_File_Ext})
	list(TRANSFORM Rhi_Sources_Ext PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/RHIDX12)
	file(GLOB_RECURSE Rhi_DX12_Sources ${Rhi_Sources_Ext})
    list(APPEND RhiProto_Sources ${Rhi_DX12_Sources})
    list(APPEND RhiProto_Include_Dir ${CMAKE_CURRENT_SOURCE_DIR}/RHIDX12)
endif()

add_library(rhiProto STATIC)
target_sources(rhiProto
        PRIVATE ${RhiProto_Sources})
target_include_directories(rhiProto
		PUBLIC ${RhiProto_Include_Dir})
		
find_package(Vulkan REQUIRED)

target_link_libraries(rhiProto
        PUBLIC Vulkan::Vulkan)

target_compile_features(rhiProto
        PRIVATE cxx_std_20)
