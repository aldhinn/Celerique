/*

File: ./vulkan/src/manager.cpp
Author: Aldhinn Espinas
Description: This source file contains vulkan resource management implementations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/internal/manager.h>
#include <celerique/logging.h>

#include <cstring>
#include <stdexcept>
#include <unordered_set>
#include <mutex>
#include <algorithm>

// Target platform defines for vulkan.
#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
#define VK_USE_PLATFORM_WAYLAND_KHR
#define VK_USE_PLATFORM_XLIB_KHR

#elif defined(CELERIQUE_FOR_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR

#elif defined(CELERIQUE_FOR_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR

#elif defined(CELERIQUE_FOR_MACOS)
#define VK_USE_PLATFORM_MACOS_MVK

#elif defined(CELERIQUE_FOR_IOS)
#define VK_USE_PLATFORM_IOS_MVK
#endif // END Target platform defines for vulkan.

#include <vulkan/vulkan.h>

#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
#include <wayland-client-protocol.h>
#endif

/// @brief Retrieves the manager singleton reference.
/// @return The reference to the manager instance.
celerique::vulkan::internal::Manager& celerique::vulkan::internal::Manager::getRef() {
    /// @brief The singleton instance of the manager.
    static Manager instance;
    return instance;
}

/// @brief Add a graphics pipeline configuration.
/// @param ptrGraphicsPipelineConfig The pointer to the graphics pipeline configuration.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
void ::celerique::vulkan::internal::Manager::addGraphicsPipeline(PipelineConfig* ptrGraphicsPipelineConfig) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);
    // TODO: Implement.
}

/// @brief Add the window handle to the graphics API.
/// @param uiProtocol The UI protocol used to create UI elements.
/// @param windowHandle The handle to the window according to UI protocol.
void celerique::vulkan::internal::Manager::addWindow(UiProtocol uiProtocol, Pointer windowHandle) {
    // Write lock thread during window registration.
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    // Simply halt.
    if (windowHandle == 0 || uiProtocol == CELERIQUE_UI_PROTOCOL_NULL) {
        celeriqueLogWarning(
            "Invalid windowHandle pointer (" + ::std::to_string(windowHandle) +
            ") and UI protocol (" + ::std::to_string(uiProtocol) + "). Failed to register window."
        );
        return;
    }
    // If the window widget was previously registered.
    if (_mapWindowToSurface.find(windowHandle) != _mapWindowToSurface.end()) {
        celeriqueLogTrace("Window already registered.");
        return;
    }

    /// @brief The handle to the vulkan surface.
    VkSurfaceKHR surface = createVulkanSurface(windowHandle, uiProtocol);
    /// @brief The handle to the physical device to be used.
    VkPhysicalDevice physicalDeviceForGraphics = selectBestPhysicalDeviceForGraphics(surface);
    /// @brief The handle to the graphics logical device.
    VkDevice graphicsLogicalDevice = nullptr;

    // Find if there is an existing graphics logical device.
    if (_vecGraphicsLogicDev.size() == 0) {
        graphicsLogicalDevice = createGraphicsLogicalDevice(windowHandle, physicalDeviceForGraphics);
    } else {
        // TODO: Properly select the best graphics logical device to use.
        // will settle on the first one for now.
        graphicsLogicalDevice = _vecGraphicsLogicDev[0];

        _mapWindowToGraphicsLogicDev[windowHandle] = graphicsLogicalDevice;
        celeriqueLogTrace("Using an existing graphics logical device");
    }

    createSwapChain(windowHandle, uiProtocol, physicalDeviceForGraphics);
    createSwapChainImageViews(windowHandle);
    createRenderPass(windowHandle);
    createSwapChainFrameBuffers(windowHandle);
    createCommandBuffers(windowHandle);

    celeriqueLogDebug("Registered window.");
}

/// @brief Remove the window handle from the graphics API registry.
/// @param windowHandle The handle to the window according to UI protocol.
void celerique::vulkan::internal::Manager::removeWindow(Pointer windowHandle) {
    {
        ::std::shared_lock<::std::shared_mutex> readLock(_sharedMutex);

        // Check if this window is still in the registry. If not, simply halt.
        if (_mapWindowToSurface.find(windowHandle) == _mapWindowToSurface.end()) {
            celeriqueLogDebug("Window is not registered. Will halt from here on.");
            return;
        }
    }

    // Write lock thread during window removal.
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    /// @brief The logical device for graphics purposes.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];

    _mapWindowToVecCommandBuffers.erase(windowHandle);
    celeriqueLogTrace("Removed command buffer references for the window.");

    /// @brief The swapchain frame buffers to be destroyed.
    const ::std::vector<VkFramebuffer>& vecSwapChainFrameBuffers = _mapWindowToVecSwapChainFrameBuffers[windowHandle];
    // Destroy the frame buffers.
    for (VkFramebuffer swapChainFrameBuffer : vecSwapChainFrameBuffers) {
        vkDestroyFramebuffer(graphicsLogicalDevice, swapChainFrameBuffer, nullptr);
    }
    _mapWindowToVecSwapChainFrameBuffers.erase(windowHandle);
    celeriqueLogTrace("Destroyed window swapchain frame buffers.");

    /// @brief The render pass to be destroyed.
    VkRenderPass renderPass = _mapWindowToRenderPass[windowHandle];
    vkDestroyRenderPass(graphicsLogicalDevice, renderPass, nullptr);
    _mapWindowToRenderPass.erase(windowHandle);
    celeriqueLogTrace("Destroyed window render pass.");

    /// @brief The swapchain image views of the window.
    const ::std::vector<VkImageView>& vecSwapChainImageViews = _mapWindowToVecSwapChainImageViews[windowHandle];
    // Destroy image views.
    for (VkImageView swapChainImageView : vecSwapChainImageViews) {
        vkDestroyImageView(graphicsLogicalDevice, swapChainImageView, nullptr);
    }
    _mapWindowToVecSwapChainImageViews.erase(windowHandle);
    celeriqueLogTrace("Destroyed window swapchain image views.");

    /// @brief The swapchain of the window.
    VkSwapchainKHR swapChain = _mapWindowToSwapChain[windowHandle];
    // Destroy swapchain.
    vkDestroySwapchainKHR(graphicsLogicalDevice, swapChain, nullptr);
    _mapWindowToSwapChain.erase(windowHandle);
    celeriqueLogTrace("Destroyed window swapchain.");

    // Delete swapchain extent.
    _mapWindowToSwapChainExtent.erase(windowHandle);
    celeriqueLogTrace("Erased window swapchain extent.");

    _mapWindowToGraphicsCommandPool.erase(windowHandle);
    celeriqueLogTrace("Removed command pool reference for the window.");

    _mapWindowToGraphicsLogicDev.erase(windowHandle);
    celeriqueLogTrace("Removed graphics logical device reference for the window.");

    /// @brief The surface registered to the window.
    VkSurfaceKHR surface = _mapWindowToSurface[windowHandle];
    vkDestroySurfaceKHR(_vulkanInstance, surface, nullptr);
    _mapWindowToSurface.erase(windowHandle);
    celeriqueLogTrace("Destroyed window surface.");

    celeriqueLogDebug("Removed window from registry.");
}

/// @brief Default constructor. (Private to prevent instantiation).
celerique::vulkan::internal::Manager::Manager() {
    // Write lock thread during initialization.
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    createVulkanInstance();
#if defined(CELERIQUE_DEBUG_MODE)
    setupDebugMessenger();
#endif
    collectAvailablePhysicalDevices();

    celeriqueLogDebug("Initialized vulkan manager.");
}

/// @brief Destructor. (Private to prevent external deletion).
celerique::vulkan::internal::Manager::~Manager() {
    // Write lock thread during resource cleanup.
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    destroySwapChainFrameBuffers();
    destroyRenderPasses();
    destroySwapChainImageViews();
    destroySwapChains();
    destroyCommandPools();
    destroyLogicalDevices();
    destroyRegisteredSurfaces();
#if defined(CELERIQUE_DEBUG_MODE)
    destroyDebugMessenger();
#endif
    destroyVulkanInstance();

    celeriqueLogDebug("Destroyed vulkan manager.");
}

/// @brief Creates the vulkan instance object.
void celerique::vulkan::internal::Manager::createVulkanInstance() {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief Contains information about the vulkan application.
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Celerique Engine";
    appInfo.apiVersion = VK_API_VERSION_1_3;

    /// @brief The vulkan extensions to be enabled.
    ::std::vector<const char*> vulkanInstanceEnabledExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,

// Platform specific extensions.
#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,

#elif defined(CELERIQUE_FOR_WINDOWS)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

#elif defined(CELERIQUE_FOR_ANDROID)
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,

#elif defined(CELERIQUE_FOR_MACOS)
    VK_MVK_MACOS_SURFACE_EXTENSION_NAME,

#elif defined(CELERIQUE_FOR_IOS)
    VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#endif // END Platform specific extensions.

#if defined(CELERIQUE_DEBUG_MODE)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif

    };

// Verify if the validation layer is supported to begin with.
#if defined(CELERIQUE_DEBUG_MODE)
    uint32_t availableLayersPropertiesCount;
    result = vkEnumerateInstanceLayerProperties(&availableLayersPropertiesCount, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumerateInstanceLayerProperties"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    std::vector<VkLayerProperties> availableLayersProperties(availableLayersPropertiesCount);
    result = vkEnumerateInstanceLayerProperties(&availableLayersPropertiesCount, availableLayersProperties.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumerateInstanceLayerProperties"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Check if VK_LAYER_KHRONOS_validation is present.
    bool validationLayerPresent = false;
    for (const VkLayerProperties& layerProp : availableLayersProperties) {
        // If VK_LAYER_KHRONOS_validation layer is available,
        // then validation layer is supported.
        if (strcmp("VK_LAYER_KHRONOS_validation", layerProp.layerName) == 0) {
            validationLayerPresent = true;
            break;
        }
    }
    if (!validationLayerPresent) {
        ::std::string errorMessage = "Validation layer is not supported.";
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
#endif

// Create debug messenger info.
#if defined(CELERIQUE_DEBUG_MODE)
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = debugCallback;
#endif

    /// @brief Vulkan instance creation information.
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = vulkanInstanceEnabledExtensions.data();
    createInfo.enabledExtensionCount = vulkanInstanceEnabledExtensions.size();
    createInfo.enabledLayerCount = static_cast<uint32_t>(_vecEnabledLayers.size());
    createInfo.ppEnabledLayerNames = _vecEnabledLayers.data();

#if defined(CELERIQUE_DEBUG_MODE)
    createInfo.pNext = &debugMessengerInfo;
#endif

    // Create the Vulkan instance.
    result = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
        const char* errorMessage = "This machine does not have GPU drivers capable of supporting vulkan.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    } else if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkCreateInstance with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    celeriqueLogTrace("Created vulkan instance.");
}

/// @brief Collects all available physical devices in this machine.
void celerique::vulkan::internal::Manager::collectAvailablePhysicalDevices() {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    // Obtain the handles of the available physical devices.
    uint32_t numOfAvailablePhysicalDevices = 0;
    result = vkEnumeratePhysicalDevices(_vulkanInstance, &numOfAvailablePhysicalDevices, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumeratePhysicalDevices"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _vecAvailablePhysDev = ::std::vector<VkPhysicalDevice>(numOfAvailablePhysicalDevices);
    result = vkEnumeratePhysicalDevices(_vulkanInstance, &numOfAvailablePhysicalDevices, _vecAvailablePhysDev.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumeratePhysicalDevices"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    celeriqueLogTrace(
        "Collected the handles of the available physical devices in this machine. "
        "There are " + ::std::to_string(numOfAvailablePhysicalDevices) + " of them."
    );
}

/// @brief Destroy all swapchain frame buffers.
void celerique::vulkan::internal::Manager::destroySwapChainFrameBuffers() {
    for (const auto& pairWindowToVecSwapChainFrameBuffers : _mapWindowToVecSwapChainFrameBuffers) {
        /// @brief The window handle.
        Pointer windowHandle = pairWindowToVecSwapChainFrameBuffers.first;
        /// @brief The assigned graphics logical device to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The swapchain buffers for the window.
        const ::std::vector<VkFramebuffer>& vecSwapChainFrameBuffers = pairWindowToVecSwapChainFrameBuffers.second;
        // Destroy frame buffers.
        for (VkFramebuffer swapChainFrameBuffer : vecSwapChainFrameBuffers) {
            vkDestroyFramebuffer(graphicsLogicalDevice, swapChainFrameBuffer, nullptr);
        }
    }
    _mapWindowToVecSwapChainFrameBuffers.clear();
    celeriqueLogTrace("Destroyed all frame buffers.");
}

/// @brief Destroy all render passes.
void celerique::vulkan::internal::Manager::destroyRenderPasses() {
    for (const auto& pairWindowToRenderPass : _mapWindowToRenderPass) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToRenderPass.first;
        /// @brief The graphics logical device that created the render pass.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The render pass to be destroyed.
        VkRenderPass renderPass = pairWindowToRenderPass.second;

        // Destroy render pass.
        vkDestroyRenderPass(graphicsLogicalDevice, renderPass, nullptr);
    }
    _mapWindowToRenderPass.clear();

    celeriqueLogTrace("Destroyed all render passes.");
}

/// @brief Destroy swapchain image views.
void celerique::vulkan::internal::Manager::destroySwapChainImageViews() {
    for (const auto& pairWindowToVecSwapChainImageViews : _mapWindowToVecSwapChainImageViews) {
        /// @brief The window handle.
        Pointer windowHandle = pairWindowToVecSwapChainImageViews.first;
        /// @brief The handle to the graphics logical device that created the swapchain.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The vector of image views to be destroyed.
        const ::std::vector<VkImageView>& vecSwapChainImageViews = pairWindowToVecSwapChainImageViews.second;
        // Iterate and destroy each.
        for (VkImageView swapChainImageView : vecSwapChainImageViews) {
            vkDestroyImageView(graphicsLogicalDevice, swapChainImageView, nullptr);
        }
    }
    _mapWindowToVecSwapChainImageViews.clear();

    celeriqueLogTrace("Destroyed swapchain image views.");
}

/// @brief Destroy all swapchain objects.
void celerique::vulkan::internal::Manager::destroySwapChains() {
    for (const auto& pairWindowToSwapchain : _mapWindowToSwapChain) {
        /// @brief The window handle.
        Pointer windowHandle = pairWindowToSwapchain.first;
        /// @brief The handle to the graphics logical device that created the swapchain.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The swapchain to be destroyed.
        VkSwapchainKHR swapchain = pairWindowToSwapchain.second;

        // Destroy swapchain.
        vkDestroySwapchainKHR(graphicsLogicalDevice, swapchain, nullptr);
    }
    _mapWindowToSwapChain.clear();
    _mapWindowToSwapChainImageFormat.clear();

    celeriqueLogTrace("Destroyed swapchains.");
}

/// @brief Destroy all command pools.
void celerique::vulkan::internal::Manager::destroyCommandPools() {
    for (const auto& pairLogicDevToVecCommandPool : _mapLogicDevToVecCommandPools) {
        /// @brief The handle to the logical device.
        VkDevice logicalDevice = pairLogicDevToVecCommandPool.first;
        /// @brief The handle to the command pool to be destroyed.
        const ::std::vector<VkCommandPool>& vecCommandPool = pairLogicDevToVecCommandPool.second;
        // Iterate and destroy each command pool.
        for (VkCommandPool commandPool : vecCommandPool) {
            // This also frees the command buffers that this pool created.
            // No need to explicitly free the command buffers.
            vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
        }
    }
    _mapLogicDevToVecCommandPools.clear();
    celeriqueLogTrace("Destroyed command pools.");
}

/// @brief Destroys all logical devices.
void celerique::vulkan::internal::Manager::destroyLogicalDevices() {
    for (VkDevice logicalDevice : _vecGraphicsLogicDev) {
        vkDestroyDevice(logicalDevice, nullptr);
    }
    _vecGraphicsLogicDev.clear();

    celeriqueLogTrace("Destroyed logical devices.");
}

/// @brief Destroy the registered surfaces.
void celerique::vulkan::internal::Manager::destroyRegisteredSurfaces() {
    for (const auto& pairWindowToSurface : _mapWindowToSurface) {
        /// @brief The surface to be destroyed.
        VkSurfaceKHR surface = pairWindowToSurface.second;
        vkDestroySurfaceKHR(_vulkanInstance, surface, nullptr);
    }
    celeriqueLogTrace("Destroyed surfaces.");
}

/// @brief Destroy the vulkan instance.
void celerique::vulkan::internal::Manager::destroyVulkanInstance() {
    if (_vulkanInstance != nullptr) {
        vkDestroyInstance(_vulkanInstance, nullptr);
        _vulkanInstance = nullptr;

        celeriqueLogTrace("Destroyed vulkan instance.");
    }
}

/// @brief Creates a vulkan surface to be registered to the widget.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
/// @param uiProtocol The UI protocol used to create UI elements.
VkSurfaceKHR celerique::vulkan::internal::Manager::createVulkanSurface(Pointer windowHandle, UiProtocol uiProtocol) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;
    /// @brief The surface to be created.
    VkSurfaceKHR surface = nullptr;

    switch(uiProtocol) {
#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
    case CELERIQUE_UI_PROTOCOL_X11: {
        /// @brief The surface creation information.
        VkXlibSurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.dpy = XOpenDisplay(NULL);
        createInfo.window = reinterpret_cast<XID>(windowHandle);

        // Create surface.
        result = vkCreateXlibSurfaceKHR(_vulkanInstance, &createInfo, nullptr, &surface);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create XLib surface with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        celeriqueLogTrace("Registered an x11 vulkan surface.");
    } break;

    case CELERIQUE_UI_PROTOCOL_WAYLAND: {
        /// @brief The surface creation information.
        VkWaylandSurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        createInfo.display = wl_display_connect(NULL);
        createInfo.surface = reinterpret_cast<wl_surface*>(windowHandle);

        // Create surface.
        result = vkCreateWaylandSurfaceKHR(reinterpret_cast<Pointer>(&createInfo), nullptr, &surface);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create wayland surface with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        celeriqueLogTrace("Registered a wayland vulkan surface.");
    } break;

#elif defined(CELERIQUE_FOR_WINDOWS)
    case CELERIQUE_UI_PROTOCOL_WIN32: {
        /// @brief The surface creation information.
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = GetModuleHandle(nullptr);
        createInfo.hwnd = reinterpret_cast<HWND>(windowHandle);

        // Create surface.
        result = vkCreateWin32SurfaceKHR(_vulkanInstance, &createInfo, NULL, &surface);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create vulkan surface with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        celeriqueLogTrace("Registered a win32 vulkan surface.");
    } break;
#endif

    default:
        ::std::string errorMessage = "Unsupported UI protocol value: " + ::std::to_string(uiProtocol);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    _mapWindowToSurface[windowHandle] = surface;
    return surface;
}

/// @brief Select the suitable physical device for creating a graphics logical device.
/// @param surface The handle to the vulkan surface.
/// @return The handle to the best physical device for graphics.
VkPhysicalDevice celerique::vulkan::internal::Manager::selectBestPhysicalDeviceForGraphics(VkSurfaceKHR surface) {
    /// @brief The container for the list of suitable physical devices for the surface.
    ::std::vector<VkPhysicalDevice> suitablePhysicalDevices;
    // Iterate over the available devices.
    for (VkPhysicalDevice availablePhysicalDevice : _vecAvailablePhysDev) {
        // Query supported features.
        VkPhysicalDeviceFeatures supportedFeatures = {};
        vkGetPhysicalDeviceFeatures(availablePhysicalDevice, &supportedFeatures);
        // Query for surface formats.
        ::std::vector<VkSurfaceFormatKHR> surfaceFormats = getSurfaceFormats(availablePhysicalDevice, surface);
        // Query for present modes.
        ::std::vector<VkPresentModeKHR> presentModes = getPresentModes(availablePhysicalDevice, surface);
        // Obtain queue family indices with graphics capabilities.
        ::std::vector<uint32_t> vecQueueFamIndicesGraphics = getQueueFamilyIndicesWithFlagBits(availablePhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
        // Obtain queue family indices with present capabilities.
        ::std::vector<uint32_t> vecQueueFamIndicesPresent = getQueueFamilyIndicesWithPresent(availablePhysicalDevice, surface);

        // Calculate physical device suitability for the engine.
        if (supportedFeatures.samplerAnisotropy == VK_TRUE &&
        physicalDeviceHasSuitableExtensions(availablePhysicalDevice) &&
        !surfaceFormats.empty() && !presentModes.empty() &&
        !vecQueueFamIndicesGraphics.empty() && !vecQueueFamIndicesPresent.empty()) {
            // Push availablePhysicalDevice as it satisfy the suitability criteria.
            suitablePhysicalDevices.push_back(availablePhysicalDevice);
        }
    }

    if (suitablePhysicalDevices.empty()) {
        const char* errorMessage = "No suitable physical device for graphics found.";
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    celeriqueLogTrace("Selected the best physical device for graphics.");
    // TODO: Select the best out of the suitable devices. Will select the first one for now.
    return suitablePhysicalDevices[0];
}

/// @brief Create a graphics logical device for the window
/// @param windowHandle The UI protocol native pointer of the window to be registered.
/// @param physicalDevice The handle to the physical device.
VkDevice celerique::vulkan::internal::Manager::createGraphicsLogicalDevice(Pointer windowHandle, VkPhysicalDevice physicalDevice) {
    // The variable that stores the result of any vulkan function called.
    VkResult result;

    /// @brief The handle to the vulkan surface.
    VkSurfaceKHR surface = _mapWindowToSurface[windowHandle];

    // Obtain queue family indices with graphics capabilities.
    ::std::vector<uint32_t> vecQueueFamIndicesGraphics = getQueueFamilyIndicesWithFlagBits(
        physicalDevice, VK_QUEUE_GRAPHICS_BIT
    );
    // Obtain queue family indices with present capabilities.
    ::std::vector<uint32_t> vecQueueFamIndicesPresent = getQueueFamilyIndicesWithPresent(
        physicalDevice, surface
    );

    /// @brief The unique indices between `vecQueueFamIndicesGraphics`
    /// and `vecQueueFamIndicesPresent` when combined.
    ::std::vector<uint32_t> vecUniqueIndices = getUniqueIndices(
        vecQueueFamIndicesGraphics, vecQueueFamIndicesPresent
    );
    /// @brief The number of queues to be requested.
    uint32_t numQueue = static_cast<uint32_t>(
        vecQueueFamIndicesGraphics.size() < vecQueueFamIndicesPresent.size() ?
        vecQueueFamIndicesGraphics.size() : vecQueueFamIndicesPresent.size()
    );
    /// @brief The array of queue priorities.
    float* arrQueuePriorities = static_cast<float*>(alloca(sizeof(float) * numQueue));
    // TODO: Determine proper priority assignments. Will do 1.0 for all for now.
    ::std::fill(arrQueuePriorities, arrQueuePriorities + numQueue, 1.0f);

    /// @brief The list of information structures on how to create the device queues.
    ::std::vector<VkDeviceQueueCreateInfo> vecDeviceQueueInfo;
    // Iterating through the unique indices.
    for (uint32_t index : vecUniqueIndices) {
        VkDeviceQueueCreateInfo deviceQueueInfo = {};
        deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueInfo.queueFamilyIndex = index;
        deviceQueueInfo.queueCount = numQueue;
        deviceQueueInfo.pQueuePriorities = arrQueuePriorities;
        vecDeviceQueueInfo.push_back(deviceQueueInfo);
    }

    /// @brief Information about the device features to be enabled.
    VkPhysicalDeviceFeatures enabledDeviceFeatures = {};
    enabledDeviceFeatures.samplerAnisotropy = VK_TRUE;

    /// @brief Information about how to create the graphics logical device.
    VkDeviceCreateInfo graphicsLogicalDeviceInfo = {};
    graphicsLogicalDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    graphicsLogicalDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(vecDeviceQueueInfo.size());
    graphicsLogicalDeviceInfo.pQueueCreateInfos = vecDeviceQueueInfo.data();
    graphicsLogicalDeviceInfo.pEnabledFeatures = &enabledDeviceFeatures;
    graphicsLogicalDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(_vecRequiredDeviceExtensions.size());
    graphicsLogicalDeviceInfo.ppEnabledExtensionNames = _vecRequiredDeviceExtensions.data();
    graphicsLogicalDeviceInfo.enabledLayerCount = static_cast<uint32_t>(_vecEnabledLayers.size());
    graphicsLogicalDeviceInfo.ppEnabledLayerNames = _vecEnabledLayers.data();

    /// @brief The handle to the logical device.
    VkDevice graphicsLogicalDevice;
    // Create the logical device.
    result = vkCreateDevice(physicalDevice, &graphicsLogicalDeviceInfo, nullptr, &graphicsLogicalDevice);
    if(result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create logical device "
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _mapWindowToGraphicsLogicDev[windowHandle] = graphicsLogicalDevice;
    _vecGraphicsLogicDev.push_back(graphicsLogicalDevice);
    celeriqueLogTrace("Created graphics logical device.");

    /// @brief The container for the graphics queues.
    ::std::vector<VkQueue> vecGraphicsQueues;
    /// @brief The container for the graphicsCommandPool.
    ::std::vector<VkCommandPool> vecCommandPools;
    /// @brief The container for the present queues.
    ::std::vector<VkQueue> vecPresentQueues;

    /// @brief The unordered set containing queue family indices with graphics.
    ::std::unordered_set<uint32_t> setQueueFamIndicesGraphics(
        vecQueueFamIndicesGraphics.begin(), vecQueueFamIndicesGraphics.end()
    );
    /// @brief The unordered set containing queue family indices with present.
    ::std::unordered_set<uint32_t> setQueueFamIndicesPresent(
        vecQueueFamIndicesPresent.begin(), vecQueueFamIndicesPresent.end()
    );

    // Retrieve graphics queue handles.
    for (uint32_t queueIndex = 0; queueIndex < numQueue; queueIndex++) {
        uint32_t queueFamilyIndex = vecUniqueIndices[queueIndex];
        /// @brief The handle to the queue to be obtained.
        VkQueue queue = nullptr;
        vkGetDeviceQueue(graphicsLogicalDevice, queueFamilyIndex, queueIndex, &queue);

        // Collect queue with graphics flag.
        if (setQueueFamIndicesGraphics.find(queueFamilyIndex) != setQueueFamIndicesGraphics.end()) {
            vecGraphicsQueues.push_back(queue);
        }
        // Collect queue with present flag.
        if (setQueueFamIndicesPresent.find(queueFamilyIndex) != setQueueFamIndicesPresent.end()) {
            vecPresentQueues.push_back(queue);
        }

        /// @brief The handle to the command pool.
        VkCommandPool commandPool = nullptr;
        /// @brief The information on how to create the command pool.
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Create the command pool.
        result = vkCreateCommandPool(graphicsLogicalDevice, &commandPoolInfo, nullptr, &commandPool);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create command pool "
            "with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecCommandPools.push_back(commandPool);
    }
    _mapGraphicsLogicDevToVecGraphicsQueues[graphicsLogicalDevice] = ::std::move(vecGraphicsQueues);
    _mapGraphicsLogicDevToVecPresentQueues[graphicsLogicalDevice] = ::std::move(vecPresentQueues);
    celeriqueLogTrace("Retrieved necessary queues for rendering graphics.");

    _mapLogicDevToVecCommandPools[graphicsLogicalDevice] = ::std::move(vecCommandPools);
    celeriqueLogTrace("Created graphics command pools.");

    return graphicsLogicalDevice;
}

/// @brief Creates a swapchain for the window.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
/// @param uiProtocol The UI protocol used to create UI elements.
/// @param physicalDevice The handle to the physical device.
void celerique::vulkan::internal::Manager::createSwapChain(Pointer windowHandle, UiProtocol uiProtocol, VkPhysicalDevice physicalDevice) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The surface to be used to create the swapchain.
    VkSurfaceKHR surface = _mapWindowToSurface[windowHandle];
    /// @brief The graphics logical device to be used to create the swapchain.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];

    /// @brief The device surface format.
    ::std::vector<VkSurfaceFormatKHR> surfaceFormats = getSurfaceFormats(physicalDevice, surface);

    // If a window of the same UI protocol has yet to be registered,
    if (_mapWindowToSwapChainImageFormat.find(windowHandle) == _mapWindowToSwapChainImageFormat.end())
        _mapWindowToSwapChainImageFormat[windowHandle] = chooseSwapChainImageFormat(surfaceFormats);

    /// @brief The device present modes.
    ::std::vector<VkPresentModeKHR> presentModes = getPresentModes(physicalDevice, surface);

    /// @brief Contains the surface capabilities.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    if(result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to get surface capabilities "
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    _mapWindowToSwapChainExtent[windowHandle] = determineSwapChainExtent(surfaceCapabilities, windowHandle, uiProtocol);

    ::std::vector<uint32_t> queueFamilyIndicesWithGraphics = getQueueFamilyIndicesWithFlagBits(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
    ::std::vector<uint32_t> queueFamilyIndicesWithPresent = getQueueFamilyIndicesWithPresent(physicalDevice, surface);

    /// @brief The information about how to create the swapchain.
    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = surface;
    swapChainInfo.minImageCount = determineMinImageCount(surfaceCapabilities);
    swapChainInfo.imageFormat = _mapWindowToSwapChainImageFormat[windowHandle];
    swapChainInfo.presentMode = chooseSwapChainPresentMode(presentModes);
    swapChainInfo.clipped = VK_TRUE; // Simply clip the obscured pixels.
    swapChainInfo.imageExtent = _mapWindowToSwapChainExtent[windowHandle];
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.preTransform = surfaceCapabilities.currentTransform;
    // TODO: Used for blending with other windows in the windows system. Perhaps, to create
    // some translucency effect on the window. We'll set it to opaque for now.
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    /// @brief Contains handle to the swapchain.
    VkSwapchainKHR swapChain;
    result = vkCreateSwapchainKHR(graphicsLogicalDevice, &swapChainInfo, nullptr, &swapChain);
    if (result != VkResult::VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create swapchain "
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _mapWindowToSwapChain[windowHandle] = swapChain;
    celeriqueLogTrace("Created swapchain.");
}

/// @brief Create the swapchain image views.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void celerique::vulkan::internal::Manager::createSwapChainImageViews(Pointer windowHandle) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The handle to the graphics logical device that created the swapchain.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    /// @brief The handle to the window's swapchain.
    VkSwapchainKHR swapChain = _mapWindowToSwapChain[windowHandle];

    // Retrieve swapchain images.
    uint32_t swapChainImagesCount = 0;
    result = vkGetSwapchainImagesKHR(graphicsLogicalDevice, swapChain, &swapChainImagesCount, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to retrieve swapchain images count "
        "with result code: " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    ::std::vector<VkImage> vecSwapChainImages(swapChainImagesCount);
    result = vkGetSwapchainImagesKHR(graphicsLogicalDevice, swapChain, &swapChainImagesCount, vecSwapChainImages.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to retrieve swapchain images "
        "with result code: " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The handle to the swapchain image views.
    ::std::vector<VkImageView> vecSwapChainImageViews;
    vecSwapChainImageViews.reserve(swapChainImagesCount);
    // Create an image view for each swapchain image.
    for (VkImage swapChainImage : vecSwapChainImages) {
        /// @brief Contains information on how to create the swapchain image view.
        VkImageViewCreateInfo imageViewInfo = {};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = swapChainImage;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = _mapWindowToSwapChainImageFormat[windowHandle];
        imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;

        // The image view to be created.
        VkImageView swapChainImageView = nullptr;
        // Create image view.
        result = vkCreateImageView(graphicsLogicalDevice, &imageViewInfo, nullptr, &swapChainImageView);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create swapchain image view "
            "with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecSwapChainImageViews.push_back(swapChainImageView);
    }

    _mapWindowToVecSwapChainImageViews[windowHandle] = ::std::move(vecSwapChainImageViews);
    celeriqueLogTrace("Created swapchain image views.");
}

/// @brief Create the render pass for windows implemented in the specified UI protocol.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void ::celerique::vulkan::internal::Manager::createRenderPass(Pointer windowHandle) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;
    /// @brief The handle to the graphics logical device.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];

    /// @brief Contains information about the colour attachment.
    VkAttachmentDescription colourAttachment = {};
    colourAttachment.format = _mapWindowToSwapChainImageFormat[windowHandle];
    colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference refColourAttachment = {};
    refColourAttachment.attachment = 0;
    refColourAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &refColourAttachment;

    // Render pass subpass dependency.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render pass info.
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colourAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    /// @brief The handle to the render pass.
    VkRenderPass renderPass = nullptr;
    // Create render pass.
    result = vkCreateRenderPass(graphicsLogicalDevice, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create render pass "
        "with result code: " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _mapWindowToRenderPass[windowHandle] = renderPass;

    celeriqueLogTrace("Created render pass.");
}

/// @brief Create the swapchain image views.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void ::celerique::vulkan::internal::Manager::createSwapChainFrameBuffers(Pointer windowHandle) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The handle to the graphics logical device that created the swapchain.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    /// @brief The swapchain image views of the window.
    const ::std::vector<VkImageView>& vecSwapChainImageViews = _mapWindowToVecSwapChainImageViews[windowHandle];
    /// @brief The swapchain frame buffers.
    ::std::vector<VkFramebuffer> vecSwapChainFrameBuffers;
    vecSwapChainFrameBuffers.reserve(vecSwapChainImageViews.size());

    // Iterate over each swapchain image view and create a framebuffer.
    for (VkImageView swapChainImageView : vecSwapChainImageViews) {
        /// @brief The image view the framebuffer is attaching to.
        VkImageView attachments[] = {swapChainImageView};

        /// @brief The information about the framebuffer to be created.
        VkFramebufferCreateInfo frameBufferInfo = {};
        frameBufferInfo.sType = VkStructureType
            ::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = _mapWindowToRenderPass[windowHandle];
        frameBufferInfo.width = _mapWindowToSwapChainExtent[windowHandle].width;
        frameBufferInfo.height = _mapWindowToSwapChainExtent[windowHandle].height;
        frameBufferInfo.layers = 1;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;

        /// @brief The framebuffer to be created.
        VkFramebuffer frameBuffer;
        // Create the framebuffer.
        result = vkCreateFramebuffer(graphicsLogicalDevice, &frameBufferInfo, nullptr, &frameBuffer);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create swapchain frame buffer "
            "with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecSwapChainFrameBuffers.emplace_back(::std::move(frameBuffer));
    }

    _mapWindowToVecSwapChainFrameBuffers[windowHandle] = ::std::move(vecSwapChainFrameBuffers);
    celeriqueLogTrace("Created swapchain frame buffers.");
}

/// @brief Create the command buffers for the window.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void celerique::vulkan::internal::Manager::createCommandBuffers(Pointer windowHandle) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The number of command buffers.
    size_t numOfCommandBuffers = _mapWindowToVecSwapChainFrameBuffers[windowHandle].size();
    /// @brief The assigned graphics logical device for the window.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    /// @brief The vector of command buffers.
    ::std::vector<VkCommandBuffer> vecCommandBuffers;
    vecCommandBuffers.reserve(numOfCommandBuffers);

    /// @brief The graphics command pool used to create the command buffers.
    VkCommandPool graphicsCommandPool = _mapLogicDevToVecCommandPools[graphicsLogicalDevice][0];
    // TODO: Properly assign the command pool for the window. We'll use the first one for now.

    for (size_t i = 0; i < numOfCommandBuffers; i++) {
        /// @brief The information regarding how the command buffer is allocated.
        VkCommandBufferAllocateInfo commandBufferInfo = {};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = graphicsCommandPool;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = 1;

        /// @brief The command buffer handle to be allocated.
        VkCommandBuffer commandBuffer = nullptr;
        // Allocate command buffer.
        result = vkAllocateCommandBuffers(graphicsLogicalDevice, &commandBufferInfo, &commandBuffer);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create command buffer "
            "with result code: " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecCommandBuffers.push_back(commandBuffer);
    }

    _mapWindowToGraphicsCommandPool[windowHandle] = graphicsCommandPool;
    _mapWindowToVecCommandBuffers[windowHandle] = ::std::move(vecCommandBuffers);
    celeriqueLogTrace("Created command buffers.");
}

/// @brief Choose the swapchain best image format out of the specified surface format.
/// @param vecSurfaceFormats The specified list of surface formats choices.
/// @return The best image format.
VkFormat celerique::vulkan::internal::Manager::chooseSwapChainImageFormat(const ::std::vector<VkSurfaceFormatKHR>& vecSurfaceFormats) {
    for (const VkSurfaceFormatKHR& surfaceFormat : vecSurfaceFormats) {
        if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            celeriqueLogTrace("Image format: VK_FORMAT_B8G8R8A8_SRGB");
            return surfaceFormat.format;
        }
    }

    // Just settle on the first one if the formats aren't perfect.
    celeriqueLogTrace("Image format: " + ::std::to_string(vecSurfaceFormats[0].format));
    return vecSurfaceFormats[0].format;
}

/// @brief Choose the best swapchain present mode out of the specified present modes.
/// @param vecPresentModes The specified list of present mode choices.
/// @return The best present mode.
VkPresentModeKHR celerique::vulkan::internal::Manager::chooseSwapChainPresentMode(const ::std::vector<VkPresentModeKHR>& vecPresentModes) {
    /// @brief The chosen present mode.
    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;

    // Iterate over present modes.
    for (VkPresentModeKHR presentMode : vecPresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            celeriqueLogTrace("Present mailbox mode.");
            // Priority choice of present mode.
            // Will halt from here on.
            return VK_PRESENT_MODE_MAILBOX_KHR;
        } else if (presentMode == VK_PRESENT_MODE_FIFO_KHR) {
            celeriqueLogTrace("Present fifo mode chosen but will keep looking.");
            // Will settle on this but will keep looking.
            chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        }
    }

    return chosenPresentMode;
}

/// @brief Determine the swapchain extent which calculates the resolution of the swapchain images.
/// @param surfaceCapabilities The surface capabilities structure.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
/// @param uiProtocol The UI protocol used to create UI elements.
/// @return The swapchain extent.
VkExtent2D celerique::vulkan::internal::Manager::determineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, Pointer windowHandle, UiProtocol uiProtocol) {
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
        return surfaceCapabilities.currentExtent;

    /// @brief The container for the result code from the vulkan api.
    VkResult result;
    /// @brief The number of pixel units in the screen.
    typedef CeleriquePixelUnits PixelUnits;

    /// @brief The width of the viewport.
    PixelUnits viewportWidth;
    /// @brief The height of the viewport.
    PixelUnits viewportHeight;

    switch(uiProtocol) {
#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
    case CELERIQUE_UI_PROTOCOL_X11: {
        // Open a connection to the X server
        Display* ptrDisplay = XOpenDisplay(NULL);
        if (ptrDisplay == nullptr) {
            const char* errorMessage = "Failed to open x11 display.";
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        /// @brief The x11 ID.
        XID x11Id = reinterpret_cast<XID>(windowHandle);
        /// @brief Contains the x11 attributes.
        XWindowAttributes x11Attributes;

        if (!XGetWindowAttributes(ptrDisplay, x11Id, &x11Attributes)) {
            XCloseDisplay(ptrDisplay);

            const char* errorMessage = "Failed to get x11 window attributes.";
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        XCloseDisplay(ptrDisplay);

        viewportWidth = x11Attributes.width;
        viewportHeight = x11Attributes.height;
    } break;

    case CELERIQUE_UI_PROTOCOL_WAYLAND: {
        // TODO: Implement.
        const char* errorMessage = "Unimplemented width and height calculation.";
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    } break;

#elif defined(CELERIQUE_FOR_WINDOWS)
    case CELERIQUE_UI_PROTOCOL_WIN32: {
        RECT clientRect;
        HWND viewportHwnd = reinterpret_cast<HWND>(windowHandle);
        if (!GetClientRect(viewportHwnd, &clientRect)) {
            const char* errorMessage = "Failed to obtain viewport area rect.";
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }

        viewportWidth = clientRect.right - clientRect.left;
        viewportHeight = clientRect.bottom - clientRect.top;
    } break;
#endif

    default:
        ::std::string errorMessage = "Unsupported UI protocol value: " + ::std::to_string(uiProtocol);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    VkExtent2D surfaceExtent = {static_cast<uint32_t>(viewportWidth), static_cast<uint32_t>(viewportHeight)};
    surfaceExtent.width = ::std::clamp(
        surfaceExtent.width, surfaceCapabilities.minImageExtent.width,
        surfaceCapabilities.maxImageExtent.width
    );
    surfaceExtent.height = ::std::clamp(
        surfaceExtent.height, surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.height
    );

    return surfaceExtent;
}

/// @brief Determine the minimum image capabilities based on the surface capabilities.
/// @param surfaceCapabilities The surface capabilities structure.
/// @return The minimum image count appropriate
uint32_t celerique::vulkan::internal::Manager::determineMinImageCount(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    return ::std::clamp(
        imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount > 0 ?
        surfaceCapabilities.maxImageCount : UINT32_MAX
    );
}

/// @brief Gets the unique indices between these two vector of indices.
/// @param leftVecIndices The vector of indices on the left hand side.
/// @param rightVecIndices The vector of indices on the right hand side.
::std::vector<uint32_t> celerique::vulkan::internal::Manager::getUniqueIndices(const ::std::vector<uint32_t>& leftVecIndices, const ::std::vector<uint32_t>& rightVecIndices) {
    // Store all indices from the left hand side vector.
    ::std::unordered_set<uint32_t> setUniqueIndices(leftVecIndices.begin(), leftVecIndices.end());
    // Iterate over the right hand side vector of indices.
    for (uint32_t index : rightVecIndices) {
        // If index is not yet in setUniqueIndices, add it.
        if (setUniqueIndices.find(index) == setUniqueIndices.end()) {
            setUniqueIndices.insert(index);
        }
    }

    return ::std::vector(setUniqueIndices.begin(), setUniqueIndices.end());
}

#if (defined(CELERIQUE_FOR_LINUX_SYSTEMS) || defined(CELERIQUE_FOR_BSD_SYSTEMS)) && !defined(CELERIQUE_FOR_ANDROID)
/// @brief Create a wayland surface.
/// @param ptrCreateInfo The creation info.
/// @param ptrAllocator The vulkan allocator callback.
/// @param ptrSurface The pointer to the surface.
/// @return Vulkan api result.
VkResult celerique::vulkan::internal::Manager::vkCreateWaylandSurfaceKHR(Pointer ptrCreateInfo, VkAllocationCallbacks* ptrAllocator, VkSurfaceKHR* ptrSurface) {
    // Load the Wayland extension function
    PFN_vkCreateWaylandSurfaceKHR func = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(
        vkGetInstanceProcAddr(_vulkanInstance, "vkCreateWaylandSurfaceKHR")
    );
    if (func != nullptr) {
        return func(_vulkanInstance, reinterpret_cast<VkWaylandSurfaceCreateInfoKHR*>(ptrCreateInfo), ptrAllocator, ptrSurface);
    } else {
        // Function is not present.
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
#endif

/// @brief Queries the vulkan API whether the physical device has suitable extension.
/// @param physicalDevice The handle to the physical device.
/// @return True if the physical has suitable extension, otherwise false.
bool celerique::vulkan::internal::Manager::physicalDeviceHasSuitableExtensions(VkPhysicalDevice physicalDevice) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    // Retrieve physical device extension properties.
    uint32_t physicalDeviceExtensionPropertiesCount = 0;
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &physicalDeviceExtensionPropertiesCount, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumerateDeviceExtensionProperties"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    ::std::vector<VkExtensionProperties> physicalDeviceExtensionProperties(physicalDeviceExtensionPropertiesCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &physicalDeviceExtensionPropertiesCount,physicalDeviceExtensionProperties.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumerateDeviceExtensionProperties"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief Required device extension names stored in unordered set of std::string.
    ::std::unordered_set<::std::string> setRequiredExtensions(
        _vecRequiredDeviceExtensions.begin(), _vecRequiredDeviceExtensions.end()
    );
    // Iterating over the extension properties delete from setRequiredExtensions
    // anything that matches the iterated extension name.
    for (const VkExtensionProperties& props : physicalDeviceExtensionProperties) {
        setRequiredExtensions.erase(props.extensionName);
        // This means that the required extensions has been matched with.
        if (setRequiredExtensions.empty()) return true;
    }
    // If it didn't return true from the for loop, it means that there still is left
    // on setRequiredExtensions that hasn't been matched, therefore the physical
    // device is no suitable.
    return false;
}

/// @brief Queries for the list of surface formats given a physical device and surface.
/// @param physicalDevice The handle to the physical device.
/// @param surface The handle to the surface.
/// @return The list of surface formats.
::std::vector<VkSurfaceFormatKHR> celerique::vulkan::internal::Manager::getSurfaceFormats(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    // Retrieve surface formats.
    uint32_t surfaceFormatsCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkGetPhysicalDeviceSurfaceFormatsKHR"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    ::std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, surfaceFormats.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkGetPhysicalDeviceSurfaceFormatsKHR"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    return surfaceFormats;
}

/// @brief Queries for the list of present modes given a physical device and surface.
/// @param physicalDevice The handle to the physical device.
/// @param surface The handle to the surface.
/// @return The list of present modes.
::std::vector<VkPresentModeKHR> celerique::vulkan::internal::Manager::getPresentModes(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    // Retrieve present modes.
    uint32_t presentModesCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkGetPhysicalDeviceSurfacePresentModesKHR"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    ::std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, presentModes.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkGetPhysicalDeviceSurfacePresentModesKHR"
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    return presentModes;
}

/// @brief Gets the indices to the queue family with the specified flag bits.
/// @param physicalDevice The handle to the physical device.
/// @return The indices of queue families with the flag bits raised.
::std::vector<uint32_t> celerique::vulkan::internal::Manager::getQueueFamilyIndicesWithFlagBits(VkPhysicalDevice physicalDevice, VkQueueFlagBits flagBits) {
    /// @brief The container for the indices of queue families with specified flag bits raised.
    ::std::vector<uint32_t> queueFamilyIndicesWithFlagBits;

    // Obtain the queue family properties of the physical device and surface.
    uint32_t queueFamilyPropsCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, nullptr);
    ::std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyPropsCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, queueFamilyProps.data());
    // Iterate over all queue family properties.
    for (uint32_t index = 0; index < queueFamilyPropsCount; index++) {
        // Check for graphics capability.
        if ((queueFamilyProps[index].queueFlags & flagBits) != 0) {
            queueFamilyIndicesWithFlagBits.push_back(index);
        }
    }

    return queueFamilyIndicesWithFlagBits;
}

/// @brief Gets the indices to the queue family with present capability.
/// @param physicalDevice The handle to the physical device.
/// @param surface The handle to the surface.
/// @return The indices of queue families with present capabilities.
::std::vector<uint32_t> celerique::vulkan::internal::Manager::getQueueFamilyIndicesWithPresent(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The container for the indices of queue families with present capabilities.
    ::std::vector<uint32_t> vecQueueFamIndicesPresent;

    // Obtain the queue family properties of the physical device and surface.
    uint32_t queueFamilyPropsCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, nullptr);
    ::std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyPropsCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropsCount, queueFamilyProps.data());
    // Iterate over all queue family properties.
    for (uint32_t index = 0; index < queueFamilyPropsCount; index++) {
        // Querying for present support.
        VkBool32 presentSupport = VK_FALSE;
        result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentSupport);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Error in calling vkGetPhysicalDeviceSurfaceSupportKHR"
            "with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }

        if (presentSupport == VK_TRUE) {
            vecQueueFamIndicesPresent.push_back(index);
        }
    }

    return vecQueueFamIndicesPresent;
}

// Validation related methods.
#if defined(CELERIQUE_DEBUG_MODE)
/// @brief Setup `_debugMessenger`.
void celerique::vulkan::internal::Manager::setupDebugMessenger() {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The creation info for debug messenger.
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
    debugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    debugMessengerInfo.pfnUserCallback = debugCallback;

    result = vkCreateDebugUtilsMessengerEXT(&debugMessengerInfo, nullptr);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in setting up debug messenger "
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    celeriqueLogTrace("Setup debug messenger.");
}

/// @brief The callback to be executed for every validation message.
/// @param messageSeverity The severity of the message.
/// @param messageType The message type.
/// @param ptrCallbackData Data from the api.
/// @param ptrUserData Pointer to the user data object.
/// @return Callback result returned to the vulkan api.
VKAPI_ATTR VkBool32 VKAPI_CALL celerique::vulkan::internal::Manager::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* ptrCallbackData, void* ptrUserData) {
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        celeriqueLogWarning("\033[0;93m Vulkan: \033[0m" + ::std::string(ptrCallbackData->pMessage));
    } else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        celeriqueLogError("\033[0;95m Vulkan: \033[0m" + ::std::string(ptrCallbackData->pMessage));
    }

    return VK_FALSE;
}

/// @brief The loaded extension function `vkCreateDebugUtilsMessengerEXT`.
/// @param ptrCreateInfo The pointer to the debug utils create info.
/// @param ptrAllocator The allocator pointer.
/// @return Vulkan api result.
VkResult celerique::vulkan::internal::Manager::vkCreateDebugUtilsMessengerEXT(VkDebugUtilsMessengerCreateInfoEXT* ptrCreateInfo, VkAllocationCallbacks* ptrAllocator) {
    // vkCreateDebugUtilsMessengerEXT is an extension function.
    // Therefore, it needed to be looked up using `vkGetInstanceProcAddr`
    PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_vulkanInstance, "vkCreateDebugUtilsMessengerEXT")
    );
    if (func != nullptr) {
        return func(_vulkanInstance, ptrCreateInfo, ptrAllocator, &_debugMessenger);
    } else {
        // Function is not present.
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/// @brief The loaded extension function `vkDestroyDebugUtilsMessengerEXT`.
/// @param ptrAllocator The allocator pointer. 
void celerique::vulkan::internal::Manager::vkDestroyDebugUtilsMessengerEXT(VkAllocationCallbacks* ptrAllocator) {
    // vkDestroyDebugUtilsMessengerEXT is an extension function.
    // Therefore, it needed to be looked up using `vkGetInstanceProcAddr`
    PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_vulkanInstance, "vkDestroyDebugUtilsMessengerEXT")
    );
    if (func != nullptr) {
        func(_vulkanInstance, _debugMessenger, ptrAllocator);
    } else {
        celeriqueLogWarning("vkDestroyDebugUtilsMessengerEXT function pointer points to null.");
    }
}

/// @brief Destroy `_debugMessenger`.
void celerique::vulkan::internal::Manager::destroyDebugMessenger() {
    if (_debugMessenger != nullptr) {
        vkDestroyDebugUtilsMessengerEXT(nullptr);
        _debugMessenger = nullptr;

        celeriqueLogTrace("Destroyed debug messenger.");
    }
}
#endif