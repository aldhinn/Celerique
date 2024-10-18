/*

File: ./vulkan/include/celerique/vulkan/internal/resources.h
Author: Aldhinn Espinas
Description: This header file contains internal interfaces to vulkan resources related objects.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_VULKAN_INTERNAL_RESOURCES_HEADER_FILE)
#define CELERIQUE_VULKAN_INTERNAL_RESOURCES_HEADER_FILE

#include <celerique/graphics.h>
#include <celerique/vulkan/internal/manager.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
namespace celerique { namespace vulkan { namespace internal {
    /// @brief The vulkan implementation to interfacing with GPU resources and functionalities.
    class GpuResources : public virtual IGpuResources {
    public:
        /// @brief Create a buffer of memory in the GPU.
        /// @param size The size of the memory to create & allocate.
        /// @param usageFlagBits The usage of the buffer.
        /// @param shaderStage The shader stage this buffer is going to be read from.
        /// @param bindingPoint The binding point of this buffer. (Defaults to 0).
        /// @return The unique identifier of the GPU buffer.
        GpuBufferID createBuffer(
            size_t size, GpuBufferUsage usageFlagBits,
            ShaderStage shaderStage = CELERIQUE_SHADER_STAGE_UNSPECIFIED, size_t bindingPoint = 0
        ) override;
        /// @brief Copy data from the CPU to the GPU buffer.
        /// @param bufferId The unique identifier of the GPU buffer.
        /// @param ptrDataSrc The pointer to where the data to be copied to the GPU resides.
        /// @param dataSize The size of the data to be copied.
        void copyToBuffer(GpuBufferID bufferId, void* ptrDataSrc, size_t dataSize) override;
        /// @brief Free the specified GPU buffer.
        /// @param bufferId The unique identifier of the GPU buffer.
        void freeBuffer(GpuBufferID bufferId) override;
        /// @brief Clear and free all GPU buffers.
        void clearBuffers() override;

    protected:
        /// @brief Default constructor. Protected to prevent instantiation.
        GpuResources();
    protected:
        /// @brief The reference to the vulkan resource manager.
        Manager& refManager;

    // Copying & moving.
    public:
        /// @brief Prevent copying.
        GpuResources(const GpuResources&) = delete;
        /// @brief Prevent moving.
        GpuResources(GpuResources&&) = delete;
        /// @brief Prevent copy re-assignment.
        GpuResources& operator=(const GpuResources&) = delete;
        /// @brief Prevent move re-assignment.
        GpuResources& operator=(GpuResources&&) = delete;

    public:
        /// @brief Pure virtual destructor.
        virtual ~GpuResources() = 0;
    };
}}}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.