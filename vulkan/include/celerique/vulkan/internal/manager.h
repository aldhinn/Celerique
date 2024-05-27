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
        /// @param ptrGraphicsPipelineConfig The pointer to the graphics pipeline configuration.
        /// @return The unique identifier to the graphics pipeline configuration that was just added.
        void addGraphicsPipeline(PipelineConfig* ptrGraphicsPipelineConfig);
        /// @brief Add the window handle to the graphics API.
        /// @param uiProtocol The UI protocol used to create UI elements.
        /// @param windowHandle The handle to the window according to UI protocol.
        void addWindow(UiProtocol uiProtocol, Pointer windowHandle);
        /// @brief Remove the window handle from the graphics API registry.
        /// @param windowHandle The handle to the window according to UI protocol.
        void removeWindow(Pointer windowHandle);

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
        /// @brief Destroy all swapchain frame buffers.
        void destroySwapChainFrameBuffers();
        /// @brief Destroy all render passes.
        void destroyRenderPasses();
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

    // Private member variables.
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
        /// @brief The list of all available physical devices.
        ::std::vector<VkPhysicalDevice> _vecAvailablePhysDev;
        /// @brief The list of required device extensions for the engine's purposes.
        ::std::vector<const char*> _vecRequiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        /// @brief The map of a window to a vulkan surface instance.
        ::std::unordered_map<Pointer, VkSurfaceKHR> _mapWindowToSurface;
        /// @brief The collection of graphics logical devices used.
        ::std::vector<VkDevice> _vecGraphicsLogicDev;
        /// @brief The map of a logical device to its command pools.
        ::std::unordered_map<VkDevice, ::std::vector<VkCommandPool>> _mapLogicDevToVecCommandPools;
        /// @brief The map of a window to its associated graphics logical device.
        ::std::unordered_map<Pointer, VkDevice> _mapWindowToGraphicsLogicDev;
        /// @brief The map of a graphics logical device to its graphics queues.
        ::std::unordered_map<VkDevice, ::std::vector<VkQueue>> _mapGraphicsLogicDevToVecGraphicsQueues;
        /// @brief The map of a graphics logical device to its present queues.
        ::std::unordered_map<VkDevice, ::std::vector<VkQueue>> _mapGraphicsLogicDevToVecPresentQueues;
        /// @brief The map of a window to its swapchain image format.
        ::std::unordered_map<Pointer, VkFormat> _mapWindowToSwapChainImageFormat;
        /// @brief The map of a window to the extent description of its swapchain.
        ::std::unordered_map<Pointer, VkExtent2D> _mapWindowToSwapChainExtent;
        /// @brief The map of window to its swapchain.
        ::std::unordered_map<Pointer, VkSwapchainKHR> _mapWindowToSwapChain;
        /// @brief The map of a window to the swapchain image views.
        ::std::unordered_map<Pointer, ::std::vector<VkImageView>> _mapWindowToVecSwapChainImageViews;
        /// @brief The map of a window to its render pass.
        ::std::unordered_map<Pointer, VkRenderPass> _mapWindowToRenderPass;
        /// @brief The map of a window to its swapchain frame buffers.
        ::std::unordered_map<Pointer, ::std::vector<VkFramebuffer>> _mapWindowToVecSwapChainFrameBuffers;
        /// @brief The map of a window to its assigned graphics command pool.
        ::std::unordered_map<Pointer, VkCommandPool> _mapWindowToGraphicsCommandPool;
        /// @brief The map of a window to its command buffers.
        ::std::unordered_map<Pointer, ::std::vector<VkCommandBuffer>> _mapWindowToVecCommandBuffers;

        // Validation layer objects.
#if defined(CELERIQUE_DEBUG_MODE)
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