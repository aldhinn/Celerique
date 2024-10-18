/*

File: ./vulkan/src/resources.cpp
Author: Aldhinn Espinas
Description: This source file contains internal implementations of vulkan resources interfaces.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/internal/resources.h>
#include <celerique/logging.h>

/// @brief Create a buffer of memory in the GPU.
/// @param size The size of the memory to create & allocate.
/// @param usageFlagBits The usage of the buffer.
/// @param shaderStage The shader stage this buffer is going to be read from.
/// @param bindingPoint The binding point of this buffer. (Defaults to 0).
/// @return The unique identifier of the GPU buffer.
::celerique::GpuBufferID celerique::vulkan::internal::GpuResources::createBuffer(
    size_t size, GpuBufferUsage usageFlagBits, ShaderStage shaderStage, size_t bindingPoint
) {
    GpuBufferID currentId = genGpuBufferId();
    refManager.createBuffer(currentId, size, usageFlagBits, shaderStage, bindingPoint);
    return currentId;
}

/// @brief Copy data from the CPU to the GPU buffer.
/// @param bufferId The unique identifier of the GPU buffer.
/// @param ptrDataSrc The pointer to where the data to be copied to the GPU resides.
/// @param dataSize The size of the data to be copied.
void celerique::vulkan::internal::GpuResources::copyToBuffer(
    GpuBufferID bufferId, void* ptrDataSrc, size_t dataSize
) {
    refManager.copyToBuffer(bufferId, ptrDataSrc, dataSize);
}

/// @brief Free the specified GPU buffer.
/// @param bufferId The unique identifier of the GPU buffer.
void celerique::vulkan::internal::GpuResources::freeBuffer(GpuBufferID bufferId) {
    refManager.freeBuffer(bufferId);
}

/// @brief Clear and free all GPU buffers.
void celerique::vulkan::internal::GpuResources::clearBuffers() {
    refManager.clearBuffers();
}

/// @brief Default constructor. Protected to prevent instantiation.
celerique::vulkan::internal::GpuResources::GpuResources() : refManager(Manager::getRef()) {
    celeriqueLogTrace("Initialized Vulkan GPU resources interface.");
}

/// @brief Pure virtual destructor.
::celerique::vulkan::internal::GpuResources::~GpuResources() {
    celeriqueLogTrace("Cleaned up Vulkan GPU resources interface.");
}