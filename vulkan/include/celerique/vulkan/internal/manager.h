/*

File: ./vulkan/include/celerique/vulkan/internal/manager.h
Author: Aldhinn Espinas
Description: This header file contains declarations for vulkan resource management. 

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_VULKAN_INTERNAL_MANAGER_HEADER_FILE)
#define CELERIQUE_VULKAN_INTERNAL_MANAGER_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/graphics.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#include <vector>
#include <unordered_map>
#include <shared_mutex>

namespace celerique { namespace vulkan { namespace internal {
    /// @brief The type of UI protocol used to create UI elements.
    typedef CeleriqueUiProtocol UiProtocol;
    /// @brief The type for a pointer container.
    typedef CeleriquePointer Pointer;

    /// @brief The description for the vulkan resource manager.
    /// There should only be a single instance to this class.
    class Manager final {
    public:
        /// @brief Retrieves the manager singleton reference.
        /// @return The reference to the manager instance.
        static Manager& getRef();

        /// @brief Add a graphics pipeline.
        /// @param graphicsPipelineConfig The graphics pipeline configuration.
        /// @param currentId The current id of the pipeline config ID to be mapped.
        /// @return The unique identifier to the graphics pipeline configuration that was just added.
        void addGraphicsPipeline(const PipelineConfig& graphicsPipelineConfig, PipelineConfigID currentId);
        /// @brief Remove the graphics pipeline specified.
        /// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
        void removeGraphicsPipeline(PipelineConfigID graphicsPipelineConfigId);
        /// @brief Clear the collection of graphics pipelines.
        void clearGraphicsPipelines();

        /// @brief Graphics draw call.
        /// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
        /// @param numVerticesToDraw The number of vertices to be drawn.
        /// @param vertexStride The size of the individual vertex input.
        /// @param numVertexElements The number of individual vertices to draw.
        /// @param ptrVertexBuffer The pointer to the vertex buffer.
        /// @param ptrIndexBuffer The pointer to the index buffer.
        void draw(
            PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw, size_t vertexStride,
            size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer
        );

        /// @brief Add the window handle to the graphics API.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param windowHandle The handle to the window according to UI protocol.
        void addWindow(UiProtocol uiProtocol, Pointer windowHandle);
        /// @brief Remove the window handle from the graphics API registry.
        /// @param windowHandle The handle to the window according to UI protocol.
        void removeWindow(Pointer windowHandle);
        /// @brief Re-create the swapchain of the specified window.
        /// @param windowHandle The handle to the window whose swapchain needs to be recreated.
        void reCreateSwapChain(Pointer windowHandle);

        /// @brief Create a buffer of memory in the GPU.
        /// @param currentId The unique identifier of the GPU buffer.
        /// @param size The size of the memory to create & allocate.
        /// @param usageFlagBits The usage of the buffer.
        /// @param shaderStage The shader stage this buffer is going to be read from.
        /// @param bindingPoint The binding point of this buffer. (Defaults to 0).
        void createBuffer(
            GpuBufferID currentId, size_t size, GpuBufferUsage usageFlagBits,
            ShaderStage shaderStage, size_t bindingPoint
        );
        /// @brief Copy data from the CPU to the GPU buffer.
        /// @param bufferId The unique identifier of the GPU buffer.
        /// @param ptrDataSrc The pointer to where the data to be copied to the GPU resides.
        /// @param dataSize The size of the data to be copied.
        void copyToBuffer(GpuBufferID bufferId, void* ptrDataSrc, size_t dataSize);
        /// @brief Free the specified GPU buffer.
        /// @param bufferId The unique identifier of the GPU buffer.
        void freeBuffer(GpuBufferID bufferId);
        /// @brief Clear and free all GPU buffers.
        void clearBuffers();

    private:
        /// @brief Default constructor. (Private to prevent instantiation).
        Manager();
        /// @brief Destructor. (Private to prevent external deletion).
        ~Manager();

    // Initializations.
    private:
        /// @brief Creates the vulkan instance object.
        void createVulkanInstance();
        /// @brief Collects all available physical devices in this machine.
        void collectAvailablePhysicalDevices();

    // Resource cleanup.
    private:
        /// @brief Destroy all sync objects.
        void destroySyncObjects();
        /// @brief Destroy all memory buffer handlers.
        void destroyMemoryBufferHandlers();
        /// @brief Destroy all pipeline related objects.
        void destroyPipelines();
        /// @brief Destroy all swapchain frame buffers.
        void destroySwapChainFrameBuffers();
        /// @brief Destroy all render passes.
        void destroyRenderPass();
        /// @brief Destroy swapchain image views.
        void destroySwapChainImageViews();
        /// @brief Destroy all swapchain objects.
        void destroySwapChains();
        /// @brief Destroy all command pools.
        void destroyCommandPools();
        /// @brief Destroys all logical devices.
        void destroyLogicalDevices();
        /// @brief Destroy the registered surfaces.
        void destroyRegisteredSurfaces();
        /// @brief Destroy the vulkan instance.
        void destroyVulkanInstance();

    // Window registration helper methods.
    private:
        /// @brief Creates a vulkan surface to be registered to the widget.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        /// @param uiProtocol The UI protocol used to create UI elements.
        VkSurfaceKHR createVulkanSurface(Pointer windowHandle, UiProtocol uiProtocol);
        /// @brief Select the suitable physical device for creating a graphics logical device.
        /// @param surface The handle to the vulkan surface.
        /// @return The handle to the best physical device for graphics.
        VkPhysicalDevice selectBestPhysicalDeviceForGraphics(VkSurfaceKHR surface);
        /// @brief Create a graphics logical device for the window
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        /// @param physicalDevice The handle to the physical device.
        VkDevice createGraphicsLogicalDevice(Pointer windowHandle, VkPhysicalDevice physicalDevice);
        /// @brief Creates a swapchain for the window.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param physicalDevice The handle to the physical device.
        void createSwapChain(Pointer windowHandle, UiProtocol uiProtocol, VkPhysicalDevice physicalDevice);
        /// @brief Create the swapchain image views.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createSwapChainImageViews(Pointer windowHandle);
        /// @brief Create the render pass for windows implemented in the specified UI protocol.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createRenderPass(Pointer windowHandle);
        /// @brief Create the swapchain image views.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createSwapChainFrameBuffers(Pointer windowHandle);
        /// @brief Create the command buffers for the window.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createCommandBuffers(Pointer windowHandle);
        /// @brief Create the containers for the mesh buffer handles.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createContainersForMeshBufferHandles(Pointer windowHandle);
        /// @brief Create synchronization objects.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        void createSyncObjects(Pointer windowHandle);

    // Swapchain helper functions.
    private:
        /// @brief Choose the swapchain best image format out of the specified surface format.
        /// @param vecSurfaceFormats The specified list of surface formats choices.
        /// @return The best image format.
        VkFormat chooseSwapChainImageFormat(const ::std::vector<VkSurfaceFormatKHR>& vecSurfaceFormats);
        /// @brief Choose the best swapchain present mode out of the specified present modes.
        /// @param vecPresentModes The specified list of present mode choices.
        /// @return The best present mode.
        VkPresentModeKHR chooseSwapChainPresentMode(const ::std::vector<VkPresentModeKHR>& vecPresentModes);
        /// @brief Determine the swapchain extent which calculates the resolution of the swapchain images.
        /// @param surfaceCapabilities The surface capabilities structure.
        /// @param windowHandle The UI protocol native pointer of the window to be registered.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @return The swapchain extent.
        VkExtent2D determineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, Pointer windowHandle, UiProtocol uiProtocol);
        /// @brief Determine the minimum image capabilities based on the surface capabilities.
        /// @param surfaceCapabilities The surface capabilities structure.
        /// @return The minimum image count appropriate
        uint32_t determineMinImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

    // Draw helper functions.
    private:
        /// @brief Draw graphics to a window.
        /// @param windowHandle The handle to the window to be drawn graphics on.
        /// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
        /// @param numVerticesToDraw The number of vertices to be drawn.
        /// @param vertexStride The size of the individual vertex input.
        /// @param numVertexElements The number of individual vertices to draw.
        /// @param ptrVertexBuffer The pointer to the vertex buffer.
        /// @param ptrIndexBuffer The pointer to the index buffer.
        void drawOnWindow(
            Pointer windowHandle, PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw,
            size_t vertexStride, size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer
        );
        /// @brief Fill the mesh buffer with vertices and indices to be drawn.
        /// @param numVerticesToDraw The number of vertices to be drawn.
        /// @param vertexStride The size of the individual vertex input.
        /// @param numVertexElements The number of individual vertices to draw.
        /// @param ptrVertexBuffer The pointer to the vertex buffer.
        /// @param ptrIndexBuffer The pointer to the index buffer.
        /// @param graphicsLogicalDevice The graphics logical device used to draw the window.
        /// @param ptrMeshBuffer The pointer to the handle to the buffer containing vertex and index data.
        /// @param ptrMeshBufferMemory The pointer to the handle to the memory of the mesh buffer in the GPU.
        void fillMeshBuffer(
            size_t numVerticesToDraw, size_t vertexStride, size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer,
            VkDevice graphicsLogicalDevice, VkBuffer* ptrMeshBuffer, VkDeviceMemory* ptrMeshBufferMemory
        );

    // Pipeline helper functions.
    private:
        /// @brief Construct a collection shader stage create information structures.
        /// @param logicalDevice The handle to the logical device that is used to create the pipeline.
        /// @param pipelineConfig The pipeline configuration.
        /// @return The collection of vulkan pipeline shader stages.
        ::std::vector<VkPipelineShaderStageCreateInfo> constructVecShaderStageCreateInfos(
            VkDevice logicalDevice, const PipelineConfig& pipelineConfig
        );
        /// @brief Construct a collection of vertex attribute descriptions.
        /// @param pipelineConfig The pipeline configuration.
        /// @return The collection of vertex attribute descriptions.
        ::std::vector<VkVertexInputAttributeDescription> constructVecVertexAttributeDescriptions(
            const PipelineConfig& pipelineConfig
        );
        /// @brief Construct a collection of descriptor set layouts for the graphics pipeline.
        /// @param pipelineConfig The pipeline configuration.
        /// @return The collection of descriptor set layouts for the graphics pipeline.
        ::std::vector<VkDescriptorSetLayout> constructVecDescriptorSetLayouts(
            const PipelineConfig& pipelineConfig
        );

    // Memory helper functions.
    private:
        /// @brief Create a buffer object and allocate memory.
        /// @param logicalDevice The logical device used to create the resources.
        /// @param deviceSize The size of the memory to be allocated.
        /// @param usageFlags The buffer's usage.
        /// @param memoryPropertyFlags The memory property flags raised.
        /// @param ptrBuffer The pointer to the buffer handle.
        /// @param ptrBufferMemory The pointer to the buffer memory handle.
        void createBufferAndAllocateMemory(
            VkDevice logicalDevice,
            VkDeviceSize deviceSize,
            VkBufferUsageFlags usageFlags,
            VkMemoryPropertyFlags memoryPropertyFlags,
            VkBuffer* ptrBuffer,
            VkDeviceMemory* ptrBufferMemory
        );
        /// @brief Find the memory type index of a given physical device.
        /// @param physicalDevice The physical device specified.
        /// @param typeFilter The bit field types that are suitable.
        /// @param propertiesFlags The property flags raised.
        /// @return The memory type index value.
        uint32_t findMemoryTypeIndex(
            VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags propertiesFlags
        );
        /// @brief Copy the contents of a source vulkan buffer to a destination vulkan buffer.
        /// @param logicalDevice The handle to the logical device to facilitate memory copying.
        /// @param commandQueue The queue used for command submissions.
        /// @param srcBuffer The buffer where the data is coming from.
        /// @param dstBuffer The buffer where the data is to be copied to.
        /// @param size The size of the data to be moved.
        void copyVulkanBufferData(
            VkDevice logicalDevice, VkQueue commandQueue,
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size
        );

    // Helper functions.
    public:
        /// @brief Gets the unique indices between these two vector of indices.
        /// @param leftVecIndices The vector of indices on the left hand side.
        /// @param rightVecIndices The vector of indices on the right hand side.
        static ::std::vector<uint32_t> getUniqueIndices(const ::std::vector<uint32_t>& leftVecIndices, const ::std::vector<uint32_t>& rightVecIndices);

    // Helper functions.
    private:
#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
        /// @brief Create a wayland surface.
        /// @param ptrCreateInfo The creation info.
        /// @param ptrAllocator The vulkan allocator callback.
        /// @param ptrSurface The pointer to the surface.
        /// @return Vulkan api result.
        VkResult vkCreateWaylandSurfaceKHR(Pointer ptrCreateInfo, VkAllocationCallbacks* ptrAllocator, VkSurfaceKHR* ptrSurface);
#endif
        /// @brief Begin a single time use command.
        /// @param logicalDevice The handle to the logical device that manages the command.
        /// @return The handle to the single time use command buffer.
        VkCommandBuffer beginSingleTimeCommand(VkDevice logicalDevice);
        /// @brief End the single time use command.
        /// @param logicalDevice The handle to the logical device that manages the command.
        /// @param singleTimeCommandBuffer The handle to the single time use command buffer.
        /// @param commandQueue The queue used for command submissions.
        void endSingleTimeCommand(VkDevice logicalDevice, VkCommandBuffer singleTimeCommandBuffer, VkQueue commandQueue);
        /// @brief Select the command pool to use for a single time use command.
        /// @param logicalDevice The handle to the logical device that manages the command.
        /// @return The handle to the command pool to use.
        VkCommandPool selectSingleTimeCommandPool(VkDevice logicalDevice);
        /// @brief Select the best queue for graphics command submissions.
        /// @param graphicsLogicalDevice The specified graphics logical device.
        /// @return The handle to the graphics queue.
        VkQueue selectGraphicsQueue(VkDevice graphicsLogicalDevice);
        /// @brief Select the best queue for present command submissions.
        /// @param graphicsLogicalDevice The specified graphics logical device.
        /// @return The handle to the present queue.
        VkQueue selectPresentQueue(VkDevice graphicsLogicalDevice);

    // Vulkan queries.
    private:
        /// @brief Queries the vulkan API whether the physical device has suitable extension.
        /// @param physicalDevice The handle to the physical device.
        /// @return True if the physical has suitable extension, otherwise false.
        bool physicalDeviceHasSuitableExtensions(VkPhysicalDevice physicalDevice);
        /// @brief Queries for the list of surface formats given a physical device and surface.
        /// @param physicalDevice The handle to the physical device.
        /// @param surface The handle to the surface.
        /// @return The list of surface formats.
        ::std::vector<VkSurfaceFormatKHR> getSurfaceFormats(
            VkPhysicalDevice physicalDevice, VkSurfaceKHR surface
        );
        /// @brief Queries for the list of present modes given a physical device and surface.
        /// @param physicalDevice The handle to the physical device.
        /// @param surface The handle to the surface.
        /// @return The list of present modes.
        ::std::vector<VkPresentModeKHR> getPresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        /// @brief Gets the indices to the queue family with the specified flag bits.
        /// @param physicalDevice The handle to the physical device.
        /// @return The indices of queue families with the flag bits raised.
        ::std::vector<uint32_t> getQueueFamilyIndicesWithFlagBits(VkPhysicalDevice physicalDevice, VkQueueFlagBits flagBits);
        /// @brief Gets the indices to the queue family with present capability.
        /// @param physicalDevice The handle to the physical device.
        /// @param surface The handle to the surface.
        /// @return The indices of queue families with present capabilities.
        ::std::vector<uint32_t> getQueueFamilyIndicesWithPresent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    // Validation related methods.
#if defined(CELERIQUE_DEBUG_MODE)
    private:
        /// @brief Setup `_debugMessenger`.
        void setupDebugMessenger();
        /// @brief The callback to be executed for every validation message.
        /// @param messageSeverity The severity of the message.
        /// @param messageType The message type.
        /// @param ptrCallbackData Data from the api.
        /// @param ptrUserData Pointer to the user data object.
        /// @return Callback result returned to the vulkan api.
        VKAPI_ATTR static VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* ptrCallbackData, void* ptrUserData);
        /// @brief The loaded extension function `vkCreateDebugUtilsMessengerEXT`.
        /// @param ptrCreateInfo The pointer to the debug utils create info.
        /// @param ptrAllocator The allocator pointer.
        /// @return Vulkan api result.
        VkResult vkCreateDebugUtilsMessengerEXT(VkDebugUtilsMessengerCreateInfoEXT* ptrCreateInfo, VkAllocationCallbacks* ptrAllocator);
        /// @brief The loaded extension function `vkDestroyDebugUtilsMessengerEXT`.
        /// @param ptrAllocator The allocator pointer. 
        void vkDestroyDebugUtilsMessengerEXT(VkAllocationCallbacks* ptr_allocator);
        /// @brief Destroy `_debugMessenger`.
        void destroyDebugMessenger();
#endif

    // Common vulkan resources and settings.
    private:
        /// @brief The shared mutex object for creating read and write locks.
        ::std::shared_mutex _sharedMutex;
        /// @brief The vulkan layers enabled.
        ::std::vector<const char*> _vecEnabledLayers = {
#if defined(CELERIQUE_DEBUG_MODE)
            "VK_LAYER_KHRONOS_validation"
#endif
        };
        /// @brief The handle to the vulkan instance.
        VkInstance _vulkanInstance = nullptr;
        /// @brief The list of required device extensions for the engine's purposes.
        ::std::vector<const char*> _vecRequiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    // Device resources.
    private:
        /// @brief The list of all available physical devices.
        ::std::vector<VkPhysicalDevice> _vecAvailablePhysDev;
        /// @brief The collection of graphics logical devices used.
        ::std::vector<VkDevice> _vecGraphicsLogicDev;
        /// @brief The map of a logical device to its physical device handle.
        ::std::unordered_map<VkDevice, VkPhysicalDevice> _mapLogicDevToPhysDev;
        /// @brief The map of a logical device to its command pools.
        ::std::unordered_map<VkDevice, ::std::vector<VkCommandPool>> _mapLogicDevToVecCommandPools;
        /// @brief The map of a graphics logical device to its graphics queues.
        ::std::unordered_map<VkDevice, ::std::vector<VkQueue>> _mapGraphicsLogicDevToVecGraphicsQueues;
        /// @brief The map of a graphics logical device to its present queues.
        ::std::unordered_map<VkDevice, ::std::vector<VkQueue>> _mapGraphicsLogicDevToVecPresentQueues;
        /// @brief The render pass instance paired with its logical device creator.
        ::std::pair<VkRenderPass, VkDevice> _pairRenderPassToLogicDev;

    // Maps of window handles to vulkan resources.
    private:
        /// @brief The map of a window to a vulkan surface instance.
        ::std::unordered_map<Pointer, VkSurfaceKHR> _mapWindowToSurface;
        /// @brief The map of a window to its UI protocol type.
        ::std::unordered_map<Pointer, UiProtocol> _mapWindowToUiProtocol;
        /// @brief The map of a window to its associated graphics logical device.
        ::std::unordered_map<Pointer, VkDevice> _mapWindowToGraphicsLogicDev;
        /// @brief The map of a window to its swapchain image format.
        ::std::unordered_map<Pointer, VkFormat> _mapWindowToSwapChainImageFormat;
        /// @brief The map of a window to the extent description of its swapchain.
        ::std::unordered_map<Pointer, VkExtent2D> _mapWindowToSwapChainExtent;
        /// @brief The map of window to its swapchain.
        ::std::unordered_map<Pointer, VkSwapchainKHR> _mapWindowToSwapChain;
        /// @brief The map of a window to the swapchain images.
        ::std::unordered_map<Pointer, ::std::vector<VkImage>> _mapWindowToVecSwapChainImages;
        /// @brief The map of a window to the swapchain image views.
        ::std::unordered_map<Pointer, ::std::vector<VkImageView>> _mapWindowToVecSwapChainImageViews;
        /// @brief The map of a window to its swapchain frame buffers.
        ::std::unordered_map<Pointer, ::std::vector<VkFramebuffer>> _mapWindowToVecSwapChainFrameBuffers;
        /// @brief The map of a window to its assigned graphics command pool.
        ::std::unordered_map<Pointer, VkCommandPool> _mapWindowToGraphicsCommandPool;
        /// @brief The map of a window to its command buffers.
        ::std::unordered_map<Pointer, ::std::vector<VkCommandBuffer>> _mapWindowToVecCommandBuffers;
        /// @brief The map of a window to the current frame index that it is rendering.
        ::std::unordered_map<Pointer, size_t> _mapWindowToCurrentFrameIndex;
        /// @brief The map of a window to its mesh buffer handles.
        ::std::unordered_map<Pointer, ::std::vector<VkBuffer>> _mapWindowToVecMeshBuffers;
        /// @brief The map of a window to its mesh buffer memory handles.
        ::std::unordered_map<Pointer, ::std::vector<VkDeviceMemory>> _mapWindowToVecMeshBufferMemories;
        /// @brief The map of a window to its image available semaphores.
        ::std::unordered_map<Pointer, ::std::vector<VkSemaphore>> _mapWindowToVecImageAvailableSemaphores;
        /// @brief The map of a window to its render finished semaphores.
        ::std::unordered_map<Pointer, ::std::vector<VkSemaphore>> _mapWindowToVecRenderFinishedSemaphores;
        /// @brief The map of a window to its in-flight fences.
        ::std::unordered_map<Pointer, ::std::vector<VkFence>> _mapWindowToVecInFlightFences;

    // Pipeline resources.
    private:
        /// @brief The map of a graphics pipeline identifier to its vulkan pipeline layout.
        ::std::unordered_map<PipelineConfigID, VkPipelineLayout> _mapGraphicsPipelineIdToPipelineLayout;
        /// @brief The map of a graphics pipeline identifier to its vulkan pipelines.
        ::std::unordered_map<PipelineConfigID, VkPipeline> _mapGraphicsPipelineIdToPipeline;
        /// @brief The map of a graphics pipeline identifier to its shader modules.
        ::std::unordered_map<PipelineConfigID, ::std::list<VkShaderModule>> _mapGraphicsPipelineIdToListShaderModules;
        /// @brief The map of a shader module to the logical device that created it.
        ::std::unordered_map<VkShaderModule, VkDevice> _mapShaderModuleToLogicDev;
        /// @brief The map of a pipeline layout to the logical device that created it.
        ::std::unordered_map<VkPipelineLayout, VkDevice> _mapPipelineLayoutToLogicDev;
        /// @brief The map of a pipeline to the logical device that created it.
        ::std::unordered_map<VkPipeline, VkDevice> _mapPipelineToLogicDev;

    // Vulkan memory resources.
    private:
        /// @brief The map of a GPU buffer ID to its logical device.
        ::std::unordered_map<GpuBufferID, VkDevice> _mapGpuBufferIdToLogicDev;
        /// @brief The map of a GPU buffer ID to the vulkan buffer handle.
        ::std::unordered_map<GpuBufferID, VkBuffer> _mapGpuBufferIdToVkBuffer;
        /// @brief The map of a GPU buffer ID to the vulkan device memory handle.
        ::std::unordered_map<GpuBufferID, VkDeviceMemory> _mapGpuBufferIdToDevMemory;
        /// @brief The map of a GPU buffer ID to its memory size.
        ::std::unordered_map<GpuBufferID, size_t> _mapGpuBufferIdToSize;
        /// @brief The map of a GPU buffer ID to its descriptor set layouts.
        ::std::unordered_map<GpuBufferID, VkDescriptorSetLayout> _mapGpuBufferIdToDescSetLayouts;

    // Validation layer objects.
#if defined(CELERIQUE_DEBUG_MODE)
    private:
        /// @brief The debug messenger handle.
        VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
#endif

    public:
        /// @brief Prevent copying.
        Manager(const Manager&) = delete;
        /// @brief Prevent moving.
        Manager(Manager&&) = delete;
        /// @brief Prevent copy re-assignment.
        Manager& operator=(const Manager&) = delete;
        /// @brief Prevent move re-assignment.
        Manager& operator=(Manager&&) = delete;
    };
}}}
#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.