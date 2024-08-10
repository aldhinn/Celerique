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
#include <thread>

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

/// @brief Add a graphics pipeline.
/// @param ptrGraphicsPipelineConfig The pointer to the graphics pipeline configuration.
/// @param currentId The current id of the pipeline config ID to be mapped.
/// @return The unique identifier to the graphics pipeline configuration that was just added.
void ::celerique::vulkan::internal::Manager::addGraphicsPipeline(
    PipelineConfig* ptrGraphicsPipelineConfig, PipelineConfigID currentId
) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    /// @brief The handle to the graphics logical device.
    VkDevice graphicsLogicalDevice = nullptr;

    if (_mapWindowToSurface.size() == 0) {
        const char* errorMessage = "addWindow should be called prior to adding a graphics pipeline.";
        celeriqueLogFatal(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // TODO: Properly select the best graphics logical device to use.
    // will settle on the first one for now.
    graphicsLogicalDevice = _vecGraphicsLogicDev[0];

    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The description of the vertex input binding.
    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.binding = 0;
    vertexBindingDescription.stride = ptrGraphicsPipelineConfig->stride();
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    /// @brief The collection of vertex attribute descriptions.
    ::std::vector<VkVertexInputAttributeDescription> vecVertexAttributeDescriptions = constructVecVertexAttributeDescriptions(
        ptrGraphicsPipelineConfig
    );

    /// @brief Information about how the input buffer layout.
    VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {};
    vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    // Assign only if there are vertex input layout specified in the pipeline configuration.
    if (!ptrGraphicsPipelineConfig->listVertexInputLayouts().empty()) {
        vertexInputStateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
        vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vecVertexAttributeDescriptions.size());
        vertexInputStateInfo.pVertexAttributeDescriptions = vecVertexAttributeDescriptions.data();
        celeriqueLogTrace("Vertex layout specified.");
    }

    /// @brief Input assembly stage information.
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    /// @brief The collection of vulkan pipeline shader stages.
    ::std::vector<VkPipelineShaderStageCreateInfo> vecShaderStageCreateInfos = constructVecShaderStageCreateInfos(
        graphicsLogicalDevice, ptrGraphicsPipelineConfig
    );
    /// @brief The list of shader modules associated with the graphics pipeline configuration identifier.
    ::std::list<VkShaderModule> listShaderModules;
    // Retrieve shader modules.
    for (VkPipelineShaderStageCreateInfo shaderStageInfo : vecShaderStageCreateInfos) {
        listShaderModules.push_back(shaderStageInfo.module);
    }
    _mapGraphicsPipelineIdToListShaderModules[currentId] = ::std::move(listShaderModules);

    /// @brief The collection of vulkan dynamic states.
    VkDynamicState arrDynamicState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    /// @brief The information about the pipeline dynamic state.
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateInfo = {};
    pipelineDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(sizeof(arrDynamicState) / sizeof(VkDynamicState));
    pipelineDynamicStateInfo.pDynamicStates = arrDynamicState;

    /// @brief The number of viewports to render to.
    size_t numOfViewports = _mapWindowToSwapChainExtent.size();

    /// @brief The viewport state information.
    VkPipelineViewportStateCreateInfo viewportStateInfo = {};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = static_cast<uint32_t>(numOfViewports);
    viewportStateInfo.scissorCount = static_cast<uint32_t>(numOfViewports);

    /// @brief Rasterization Information.
    VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo.lineWidth = 1.0f;
    rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo.depthBiasEnable = VK_FALSE;

    /// @brief Multi-sampling information.
    VkPipelineMultisampleStateCreateInfo multiSamplingInfo = {};
    multiSamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSamplingInfo.sampleShadingEnable = VK_FALSE;
    multiSamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multiSamplingInfo.minSampleShading = 1.0f;

    /// @brief Colour Blend Attachment.
    VkPipelineColorBlendAttachmentState colourBlendAttachment = {};
    colourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colourBlendAttachment.blendEnable = VK_FALSE;
    colourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colourBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colourBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colourBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    /// @brief Colour blending information.
    VkPipelineColorBlendStateCreateInfo colourBlendingInfo = {};
    colourBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colourBlendingInfo.logicOpEnable = VK_FALSE;
    colourBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    colourBlendingInfo.attachmentCount = 1;
    colourBlendingInfo.pAttachments = &colourBlendAttachment;
    colourBlendingInfo.blendConstants[0] = 0.0f;
    colourBlendingInfo.blendConstants[1] = 0.0f;
    colourBlendingInfo.blendConstants[2] = 0.0f;
    colourBlendingInfo.blendConstants[3] = 0.0f;

    /// @brief Graphics Pipeline layout information.
    VkPipelineLayoutCreateInfo graphicsPipelineLayoutInfo = {};
    graphicsPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    /// @brief The handle to the graphics pipeline layout.
    VkPipelineLayout graphicsPipelineLayout = nullptr;
    // Create pipeline layout.
    result = vkCreatePipelineLayout(graphicsLogicalDevice, &graphicsPipelineLayoutInfo, nullptr, &graphicsPipelineLayout);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create graphics pipeline layout with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _mapPipelineLayoutToLogicDev[graphicsPipelineLayout] = graphicsLogicalDevice;
    _mapGraphicsPipelineIdToPipelineLayout[currentId] = graphicsPipelineLayout;

    /// @brief Graphics pipeline information.
    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.layout = graphicsPipelineLayout;
    graphicsPipelineInfo.pVertexInputState = &vertexInputStateInfo;
    graphicsPipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    graphicsPipelineInfo.stageCount = static_cast<uint32_t>(vecShaderStageCreateInfos.size());
    graphicsPipelineInfo.pStages = vecShaderStageCreateInfos.data();
    graphicsPipelineInfo.pViewportState = &viewportStateInfo;
    graphicsPipelineInfo.pRasterizationState = &rasterizationInfo;
    graphicsPipelineInfo.pColorBlendState = &colourBlendingInfo;
    graphicsPipelineInfo.pMultisampleState = &multiSamplingInfo;
    graphicsPipelineInfo.renderPass = _pairRenderPassToLogicDev.first;
    graphicsPipelineInfo.pDynamicState = &pipelineDynamicStateInfo;

    /// @brief The handle to the graphics pipeline.
    VkPipeline graphicsPipeline = nullptr;
    // Create graphics pipeline.
    result = vkCreateGraphicsPipelines(
        graphicsLogicalDevice, nullptr, 1, &graphicsPipelineInfo, nullptr, &graphicsPipeline
    );
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create graphics pipeline with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    _mapPipelineToLogicDev[graphicsPipeline] = graphicsLogicalDevice;
    _mapGraphicsPipelineIdToPipeline[currentId] = graphicsPipeline;

    celeriqueLogDebug("Created graphics pipeline.");
}

/// @brief Remove the graphics pipeline specified.
/// @param graphicsPipelineConfigId The identifier of the graphics pipeline configuration to be removed.
void ::celerique::vulkan::internal::Manager::removeGraphicsPipeline(PipelineConfigID graphicsPipelineConfigId) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    /// @brief The handle to the graphics pipeline to be destroyed.
    VkPipeline graphicsPipeline = _mapGraphicsPipelineIdToPipeline[graphicsPipelineConfigId];
    /// @brief The graphics logical device that created the pipeline objects.
    VkDevice graphicsLogicalDevice = _mapPipelineToLogicDev[graphicsPipeline];
    // Destroy pipeline.
    vkDestroyPipeline(graphicsLogicalDevice, graphicsPipeline, nullptr);
    // Erase.
    _mapGraphicsPipelineIdToPipeline.erase(graphicsPipelineConfigId);
    _mapPipelineToLogicDev.erase(graphicsPipeline);

    /// @brief The handle to the graphics pipeline layout to be destroyed.
    VkPipelineLayout graphicsPipelineLayout = _mapGraphicsPipelineIdToPipelineLayout[graphicsPipelineConfigId];
    // Destroy pipeline layout.
    vkDestroyPipelineLayout(graphicsLogicalDevice, graphicsPipelineLayout, nullptr);
    // Erase.
    _mapGraphicsPipelineIdToPipelineLayout.erase(graphicsPipelineConfigId);
    _mapPipelineLayoutToLogicDev.erase(graphicsPipelineLayout);

    /// @brief The list of shader modules associated with the graphics pipeline configuration identifier.
    const ::std::list<VkShaderModule>& listShaderModules = _mapGraphicsPipelineIdToListShaderModules[graphicsPipelineConfigId];
    // Iterate and destroy.
    for (VkShaderModule shaderModule : listShaderModules) {
        vkDestroyShaderModule(graphicsLogicalDevice, shaderModule, nullptr);
        // Erase reference.
        _mapShaderModuleToLogicDev.erase(shaderModule);
    }
    // Erase.
    _mapGraphicsPipelineIdToListShaderModules.erase(graphicsPipelineConfigId);
}

/// @brief Clear the collection of graphics pipelines.
void ::celerique::vulkan::internal::Manager::clearGraphicsPipelines() {
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    // Iterate and destroy each object related to graphics pipelines.
    for (const auto& pairGraphicsPipelineIdToPipeline : _mapGraphicsPipelineIdToPipeline) {
        /// @brief The graphics pipeline configuration identifier.
        PipelineConfigID graphicsPipelineConfigId = pairGraphicsPipelineIdToPipeline.first;

        /// @brief The handle to the graphics pipeline to be destroyed.
        VkPipeline graphicsPipeline = _mapGraphicsPipelineIdToPipeline[graphicsPipelineConfigId];
        /// @brief The graphics logical device that created the pipeline objects.
        VkDevice graphicsLogicalDevice = _mapPipelineToLogicDev[graphicsPipeline];
        // Destroy pipeline.
        vkDestroyPipeline(graphicsLogicalDevice, graphicsPipeline, nullptr);
        // Erase.
        _mapGraphicsPipelineIdToPipeline.erase(graphicsPipelineConfigId);
        _mapPipelineToLogicDev.erase(graphicsPipeline);

        /// @brief The handle to the graphics pipeline layout to be destroyed.
        VkPipelineLayout graphicsPipelineLayout = _mapGraphicsPipelineIdToPipelineLayout[graphicsPipelineConfigId];
        // Destroy pipeline layout.
        vkDestroyPipelineLayout(graphicsLogicalDevice, graphicsPipelineLayout, nullptr);
        // Erase.
        _mapGraphicsPipelineIdToPipelineLayout.erase(graphicsPipelineConfigId);
        _mapPipelineLayoutToLogicDev.erase(graphicsPipelineLayout);

        /// @brief The list of shader modules associated with the graphics pipeline configuration identifier.
        const ::std::list<VkShaderModule>& listShaderModules = _mapGraphicsPipelineIdToListShaderModules[graphicsPipelineConfigId];
        // Iterate and destroy.
        for (VkShaderModule shaderModule : listShaderModules) {
            vkDestroyShaderModule(graphicsLogicalDevice, shaderModule, nullptr);
            // Erase reference.
            _mapShaderModuleToLogicDev.erase(shaderModule);
        }
        // Erase.
        _mapGraphicsPipelineIdToListShaderModules.erase(graphicsPipelineConfigId);
    }
}

/// @brief Graphics draw call.
/// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
/// @param numVerticesToDraw The number of vertices to be drawn.
/// @param vertexStride The size of the individual vertex input.
/// @param numVertexElements The number of individual vertices to draw.
/// @param ptrVertexBuffer The pointer to the vertex buffer.
/// @param ptrIndexBuffer The pointer to the index buffer.
void ::celerique::vulkan::internal::Manager::draw(
    PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw, size_t vertexStride,
    size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer
) {
    ::std::shared_lock<::std::shared_mutex> readLock(_sharedMutex);

    /// @brief The container for the thread handles that executes the draw calls for each window.
    ::std::list<::std::thread> listDrawCallThreads;
    // Iterate over all windows to be drawn.
    for (const auto& pairWindowToSurface : _mapWindowToSurface) {
        /// @brief The window handle.
        Pointer windowHandle = pairWindowToSurface.first;
        // Execute drawing on a different thread.
        ::std::thread drawCallThread(::std::bind(
            &Manager::drawOnWindow, this, windowHandle, graphicsPipelineConfigId, numVerticesToDraw,
            vertexStride, numVertexElements, ptrVertexBuffer, ptrIndexBuffer
        ));
        // Collect thread handle.
        listDrawCallThreads.emplace_back(::std::move(drawCallThread));
    }
    // Wait on all draw call threads to finish before exiting.
    for (::std::thread& refDrawCallThread : listDrawCallThreads) {
        refDrawCallThread.join();
    }
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
    createContainersForMeshBufferHandles(windowHandle);
    createSyncObjects(windowHandle);

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
    // Wait for resources to clear out.
    vkDeviceWaitIdle(graphicsLogicalDevice);

    /// @brief The collection of in-flight fences to be destroyed.
    const ::std::vector<VkFence>& vecInFlightFences = _mapWindowToVecInFlightFences[windowHandle];
    // Destroy the in-flight fences.
    for (VkFence inFlightFences : vecInFlightFences) {
        vkDestroyFence(graphicsLogicalDevice, inFlightFences, nullptr);
    }
    _mapWindowToVecInFlightFences.erase(windowHandle);
    celeriqueLogTrace("Destroyed window in-flight fences.");

    /// @brief The collection of render-finished semaphores.
    const ::std::vector<VkSemaphore>& vecRenderFinishedSemaphores = _mapWindowToVecRenderFinishedSemaphores[windowHandle];
    // Destroy the render-finished semaphores.
    for (VkSemaphore renderFinishedSemaphore : vecRenderFinishedSemaphores) {
        vkDestroySemaphore(graphicsLogicalDevice, renderFinishedSemaphore, nullptr);
    }
    _mapWindowToVecRenderFinishedSemaphores.clear();
    celeriqueLogTrace("Destroyed window render finished semaphores.");

    /// @brief The collection of image available semaphores.
    const ::std::vector<VkSemaphore>& vecImageAvailableSemaphores = _mapWindowToVecImageAvailableSemaphores[windowHandle];
    // Destroy the image available semaphores.
    for (VkSemaphore imageAvailableSemaphore : vecImageAvailableSemaphores) {
        vkDestroySemaphore(graphicsLogicalDevice, imageAvailableSemaphore, nullptr);
    }
    _mapWindowToVecImageAvailableSemaphores.clear();
    celeriqueLogTrace("Destroyed window image available semaphores.");

    _mapWindowToVecCommandBuffers.erase(windowHandle);
    celeriqueLogTrace("Removed command buffer references for the window.");

    /// @brief The mesh buffer memory handles to be freed.
    const ::std::vector<VkDeviceMemory>& vecMeshBufferMemories = _mapWindowToVecMeshBufferMemories[windowHandle];
    // Iterate over memories and free.
    for (VkDeviceMemory meshBufferMemory : vecMeshBufferMemories) {
        // Free if not null.
        if (meshBufferMemory != nullptr) {
            vkFreeMemory(graphicsLogicalDevice, meshBufferMemory, nullptr);
        }
    }
    _mapWindowToVecMeshBufferMemories.erase(windowHandle);
    /// @brief The mesh buffers to be destroyed.
    const ::std::vector<VkBuffer>& vecMeshBuffers = _mapWindowToVecMeshBuffers[windowHandle];
    // Iterate over buffers and destroy.
    for (VkBuffer meshBuffer : vecMeshBuffers) {
        // Destroy if not null.
        if (meshBuffer != nullptr) {
            vkDestroyBuffer(graphicsLogicalDevice, meshBuffer, nullptr);
        }
    }
    _mapWindowToVecMeshBuffers.erase(windowHandle);
    celeriqueLogTrace("Removed mesh buffer handles for the window.");

    /// @brief The swapchain frame buffers to be destroyed.
    const ::std::vector<VkFramebuffer>& vecSwapChainFrameBuffers = _mapWindowToVecSwapChainFrameBuffers[windowHandle];
    // Destroy the frame buffers.
    for (VkFramebuffer swapChainFrameBuffer : vecSwapChainFrameBuffers) {
        vkDestroyFramebuffer(graphicsLogicalDevice, swapChainFrameBuffer, nullptr);
    }
    _mapWindowToVecSwapChainFrameBuffers.erase(windowHandle);
    celeriqueLogTrace("Destroyed window swapchain frame buffers.");

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
    _mapWindowToUiProtocol.erase(windowHandle);
    celeriqueLogTrace("Destroyed window surface.");

    celeriqueLogDebug("Removed window from registry.");
}

/// @brief Re-create the swapchain of the specified window.
/// @param windowHandle The handle to the window whose swapchain needs to be recreated.
void celerique::vulkan::internal::Manager::reCreateSwapChain(Pointer windowHandle) {
    ::std::unique_lock<::std::shared_mutex> writeLock(_sharedMutex);

    /// @brief The graphics logical device assigned to the window.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    // Wait for logical device resources to be free.
    vkDeviceWaitIdle(graphicsLogicalDevice);

    /// @brief The physical device that is being represented by the graphics logical device.
    VkPhysicalDevice graphicsPhysicalDevice = _mapLogicDevToPhysDev[graphicsLogicalDevice];
    /// @brief The UI protocol of the window.
    UiProtocol uiProtocol = _mapWindowToUiProtocol[windowHandle];
    /// @brief The assigned graphics command pool for the window.
    VkCommandPool graphicsCommandPool = _mapWindowToGraphicsCommandPool[windowHandle];
    /// @brief The reference to the window's collection of command buffers.
    ::std::vector<VkCommandBuffer>& refVecCommandBuffers = _mapWindowToVecCommandBuffers[windowHandle];
    /// @brief The reference to the window's collection of the swapchain frame buffers.
    ::std::vector<VkFramebuffer>& refVecFrameBuffers = _mapWindowToVecSwapChainFrameBuffers[windowHandle];
    /// @brief The reference to the collection of the window's collection of swapchain image views.
    ::std::vector<VkImageView>& refVecSwapChainImageViews = _mapWindowToVecSwapChainImageViews[windowHandle];
    /// @brief The window's swapchain handle.
    VkSwapchainKHR swapChain = _mapWindowToSwapChain[windowHandle];

    // Free existing command buffers.
    vkFreeCommandBuffers(graphicsLogicalDevice, graphicsCommandPool, static_cast<uint32_t>(refVecCommandBuffers.size()), refVecCommandBuffers.data());
    // Destroy current framebuffers.
    for (VkFramebuffer frameBuffer : refVecFrameBuffers) {
        vkDestroyFramebuffer(graphicsLogicalDevice, frameBuffer, nullptr);
    }
    // Destroy current swapchain image views.
    for (VkImageView swapChainImageView : refVecSwapChainImageViews) {
        vkDestroyImageView(graphicsLogicalDevice, swapChainImageView, nullptr);
    }
    // Destroy swapchain.
    vkDestroySwapchainKHR(graphicsLogicalDevice, swapChain, nullptr);

    createSwapChain(windowHandle, uiProtocol, graphicsPhysicalDevice);
    createSwapChainImageViews(windowHandle);
    createSwapChainFrameBuffers(windowHandle);
    createCommandBuffers(windowHandle);
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

    // Wait for the graphics logical devices's resources to be available.
    for (VkDevice graphicsLogicalDevice : _vecGraphicsLogicDev) {
        vkDeviceWaitIdle(graphicsLogicalDevice);
    }

    destroySyncObjects();
    destroyMeshBufferHandlers();
    destroyPipelines();
    destroySwapChainFrameBuffers();
    destroyRenderPass();
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
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo = {};
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

/// @brief Destroy all sync objects.
void celerique::vulkan::internal::Manager::destroySyncObjects() {
    for (const auto& pairWindowToVecImageAvailableSemaphores : _mapWindowToVecImageAvailableSemaphores) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToVecImageAvailableSemaphores.first;
        /// @brief The handle to the graphics logical device assigned to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The collection of image available semaphores.
        const ::std::vector<VkSemaphore>& vecImageAvailableSemaphores = pairWindowToVecImageAvailableSemaphores.second;
        // Iterate over and destroy.
        for (VkSemaphore imageAvailableSemaphore : vecImageAvailableSemaphores) {
            vkDestroySemaphore(graphicsLogicalDevice, imageAvailableSemaphore, nullptr);
        }
    }
    _mapWindowToVecImageAvailableSemaphores.clear();
    for (const auto& pairWindowToVecRenderFinishedSemaphores : _mapWindowToVecRenderFinishedSemaphores) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToVecRenderFinishedSemaphores.first;
        /// @brief The handle to the graphics logical device assigned to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The collection of render finished semaphores.
        const ::std::vector<VkSemaphore>& vecRenderFinishedSemaphores = pairWindowToVecRenderFinishedSemaphores.second;
        // Iterate over and destroy.
        for (VkSemaphore renderFinishedSemaphore : vecRenderFinishedSemaphores) {
            vkDestroySemaphore(graphicsLogicalDevice, renderFinishedSemaphore, nullptr);
        }
    }
    _mapWindowToVecRenderFinishedSemaphores.clear();
    for (const auto& pairWindowToVecInFlightFences : _mapWindowToVecInFlightFences) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToVecInFlightFences.first;
        /// @brief The handle to the graphics logical device assigned to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The collection of in-flight fences.
        const ::std::vector<VkFence>& vecInFlightFences = pairWindowToVecInFlightFences.second;
        // Iterate over and destroy.
        for (VkFence inFlightFence : vecInFlightFences) {
            vkDestroyFence(graphicsLogicalDevice, inFlightFence, nullptr);
        }
    }
    _mapWindowToVecInFlightFences.clear();

    celeriqueLogTrace("Destroyed all sync objects.");
}

/// @brief Destroy all mesh buffer handlers.
void celerique::vulkan::internal::Manager::destroyMeshBufferHandlers() {
    // Iterate over device memory handles and destroy.
    for (const auto& pairWindowToVecMeshBufferMemories : _mapWindowToVecMeshBufferMemories) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToVecMeshBufferMemories.first;
        /// @brief The handle to the graphics logical device assigned to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The collection of mesh buffer memories to be freed.
        const ::std::vector<VkDeviceMemory>& vecMeshBufferMemories = pairWindowToVecMeshBufferMemories.second;
        // Iterate over and free.
        for (VkDeviceMemory meshBufferMemory : vecMeshBufferMemories) {
            // Free if not null.
            if (meshBufferMemory != nullptr) {
                vkFreeMemory(graphicsLogicalDevice, meshBufferMemory, nullptr);
            }
        }
    }
    _mapWindowToVecMeshBufferMemories.clear();
    // Iterate over buffer handles and destroy.
    for (const auto& pairWindowToVecMeshBuffers : _mapWindowToVecMeshBuffers) {
        /// @brief The handle to the window.
        Pointer windowHandle = pairWindowToVecMeshBuffers.first;
        /// @brief The handle to the graphics logical device assigned to the window.
        VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
        /// @brief The collection of mesh buffers to be destroyed.
        const ::std::vector<VkBuffer>& vecMeshBuffers = _mapWindowToVecMeshBuffers[windowHandle];
        // Iterate over and destroy.
        for (VkBuffer meshBuffer : vecMeshBuffers) {
            // Destroy if not null.
            if (meshBuffer != nullptr) {
                vkDestroyBuffer(graphicsLogicalDevice, meshBuffer, nullptr);
            }
        }
    }
    _mapWindowToVecMeshBuffers.clear();

    celeriqueLogTrace("Destroyed all mesh buffer handlers.");
}

/// @brief Destroy all pipeline related objects.
void celerique::vulkan::internal::Manager::destroyPipelines() {
    // Iterate over pipeline instances.
    for (const auto& pairPipelineToLogicDev : _mapPipelineToLogicDev) {
        /// @brief The pipeline to be destroyed.
        VkPipeline pipeline = pairPipelineToLogicDev.first;
        /// @brief The logical device creator.
        VkDevice logicalDevice = pairPipelineToLogicDev.second;
        // Destroy.
        vkDestroyPipeline(logicalDevice, pipeline, nullptr);
    }
    _mapPipelineToLogicDev.clear();
    _mapGraphicsPipelineIdToPipeline.clear();
    // Iterate over all pipeline layouts.
    for (const auto& pairPipelineToLogicDev : _mapPipelineLayoutToLogicDev) {
        /// @brief The pipeline layout to be destroyed.
        VkPipelineLayout pipelineLayout = pairPipelineToLogicDev.first;
        /// @brief The logical device creator.
        VkDevice logicalDevice = pairPipelineToLogicDev.second;
        // Destroy.
        vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    }
    _mapPipelineLayoutToLogicDev.clear();
    _mapGraphicsPipelineIdToPipelineLayout.clear();
    // Iterate over shader modules.
    for (const auto& pairShaderModuleToLogicDev : _mapShaderModuleToLogicDev) {
        /// @brief The shader module to be destroyed.
        VkShaderModule shaderModule = pairShaderModuleToLogicDev.first;
        /// @brief The logical device creator.
        VkDevice logicalDevice = pairShaderModuleToLogicDev.second;
        // Destroy.
        vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
    }
    _mapShaderModuleToLogicDev.clear();

    celeriqueLogTrace("Destroyed all pipeline related objects.");
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
void celerique::vulkan::internal::Manager::destroyRenderPass() {
    /// @brief The logical device that created the render pass.
    VkDevice logicalDevice = _pairRenderPassToLogicDev.second;
    /// @brief The render pass to be destroyed.
    VkRenderPass renderPass = _pairRenderPassToLogicDev.first;

    // Destroy render pass.
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

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
    _mapLogicDevToPhysDev.clear();

    celeriqueLogTrace("Destroyed logical devices.");
}

/// @brief Destroy the registered surfaces.
void celerique::vulkan::internal::Manager::destroyRegisteredSurfaces() {
    for (const auto& pairWindowToSurface : _mapWindowToSurface) {
        /// @brief The surface to be destroyed.
        VkSurfaceKHR surface = pairWindowToSurface.second;
        vkDestroySurfaceKHR(_vulkanInstance, surface, nullptr);
    }
    _mapWindowToSurface.clear();
    _mapWindowToUiProtocol.clear();
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
    _mapWindowToUiProtocol[windowHandle] = uiProtocol;
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
    _mapLogicDevToPhysDev[graphicsLogicalDevice] = physicalDevice;
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

    /// @brief The index of the being requested.
    uint32_t queueIndex = 0;
    // Retrieve graphics queue handles.
    for (uint32_t queueFamilyIndex : vecUniqueIndices) {
        // The number of queues requested is reached.
        if (queueIndex == numQueue) break;

        /// @brief The handle to the queue to be obtained.
        VkQueue queue = nullptr;
        vkGetDeviceQueue(graphicsLogicalDevice, queueFamilyIndex, queueIndex, &queue);

        // Collect queue with graphics flag.
        if (setQueueFamIndicesGraphics.find(queueFamilyIndex) != setQueueFamIndicesGraphics.end()) {
            vecGraphicsQueues.push_back(queue);

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
            celeriqueLogTrace("Created graphics command pool.");
            queueIndex++;
        }
    }
    // Reset.
    queueIndex = 0;
    // Retrieve present queue handles.
    for (uint32_t queueFamilyIndex : vecUniqueIndices) {
        // The number of queues requested is reached.
        if (queueIndex == numQueue) break;

        /// @brief The handle to the queue to be obtained.
        VkQueue queue = nullptr;
        vkGetDeviceQueue(graphicsLogicalDevice, queueFamilyIndex, queueIndex, &queue);

        // Collect queue with graphics flag.
        if (setQueueFamIndicesPresent.find(queueFamilyIndex) != setQueueFamIndicesPresent.end()) {
            vecPresentQueues.push_back(queue);
            queueIndex++;
        }
    }

    _mapGraphicsLogicDevToVecGraphicsQueues[graphicsLogicalDevice] = ::std::move(vecGraphicsQueues);
    _mapGraphicsLogicDevToVecPresentQueues[graphicsLogicalDevice] = ::std::move(vecPresentQueues);
    celeriqueLogTrace("Retrieved necessary queues for rendering graphics.");

    _mapLogicDevToVecCommandPools[graphicsLogicalDevice] = ::std::move(vecCommandPools);

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
    if (result != VK_SUCCESS) {
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

    _mapWindowToVecSwapChainImages[windowHandle] = ::std::move(vecSwapChainImages);
    _mapWindowToVecSwapChainImageViews[windowHandle] = ::std::move(vecSwapChainImageViews);
    celeriqueLogTrace("Created swapchain image views.");
}

/// @brief Create the render pass for windows implemented in the specified UI protocol.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void ::celerique::vulkan::internal::Manager::createRenderPass(Pointer windowHandle) {
    if (_pairRenderPassToLogicDev.first != nullptr) {
        celeriqueLogDebug("Render pass already created.");
        return;
    }

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
    _pairRenderPassToLogicDev.first = renderPass;
    _pairRenderPassToLogicDev.second = graphicsLogicalDevice;

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
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = _pairRenderPassToLogicDev.first;
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

/// @brief Create the containers for the mesh buffer handles.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void celerique::vulkan::internal::Manager::createContainersForMeshBufferHandles(Pointer windowHandle) {
    /// @brief The number of frames to be rendered.
    size_t numFrames = _mapWindowToVecSwapChainFrameBuffers[windowHandle].size();
    _mapWindowToVecMeshBufferMemories[windowHandle] = ::std::vector<VkDeviceMemory>(numFrames, nullptr);
    _mapWindowToVecMeshBuffers[windowHandle] = ::std::vector<VkBuffer>(numFrames, nullptr);

    celeriqueLogTrace("Created mesh buffer handles.");
}

/// @brief Create synchronization objects.
/// @param windowHandle The UI protocol native pointer of the window to be registered.
void celerique::vulkan::internal::Manager::createSyncObjects(Pointer windowHandle) {
    // Render on the first frame.
    _mapWindowToCurrentFrameIndex[windowHandle] = 0;

    /// @brief The handle to the graphics logical device assigned for the window.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    /// @brief The number of semaphores and fences to create. (will depend on number of frame buffers).
    size_t numOfSyncObjects = _mapWindowToVecSwapChainFrameBuffers[windowHandle].size();

    /// @brief The collection of image available semaphores.
    ::std::vector<VkSemaphore> vecImageAvailableSemaphores;
    vecImageAvailableSemaphores.reserve(numOfSyncObjects);
    /// @brief The collection of render finished semaphores.
    ::std::vector<VkSemaphore> vecRenderFinishedSemaphores;
    vecRenderFinishedSemaphores.reserve(numOfSyncObjects);
    /// @brief The collection of in-flight fences.
    ::std::vector<VkFence> vecInFlightFences;
    vecInFlightFences.reserve(numOfSyncObjects);

    // Create the sync objects.
    for (size_t i = 0; i < numOfSyncObjects; i++) {
        /// @brief The container for the result code from the vulkan api.
        VkResult result;

        /// @brief Information about the image available semaphore.
        VkSemaphoreCreateInfo imageAvailableSemaphoreInfo = {};
        imageAvailableSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        /// @brief The handle to the image available semaphore.
        VkSemaphore imageAvailableSemaphore = nullptr;
        // Create the image available semaphore.
        result = vkCreateSemaphore(graphicsLogicalDevice, &imageAvailableSemaphoreInfo, nullptr, &imageAvailableSemaphore);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create image available semaphore with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecImageAvailableSemaphores.push_back(imageAvailableSemaphore);

        /// @brief Information about the render finished semaphore.
        VkSemaphoreCreateInfo renderFinishedSemaphoreInfo = {};
        renderFinishedSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        /// @brief The handle to the render finished semaphore.
        VkSemaphore renderFinishedSemaphore = nullptr;
        // Create the image available semaphore.
        result = vkCreateSemaphore(graphicsLogicalDevice, &renderFinishedSemaphoreInfo, nullptr, &renderFinishedSemaphore);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create render finished semaphore with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecRenderFinishedSemaphores.push_back(renderFinishedSemaphore);

        /// @brief The information about the in-flight fence.
        VkFenceCreateInfo inFlightFenceInfo{};
        inFlightFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        inFlightFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        /// @brief The handle to the in-flight fence.
        VkFence inFlightFence;
        // Create the in-flight fence.
        result = vkCreateFence(graphicsLogicalDevice, &inFlightFenceInfo, nullptr, &inFlightFence);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create in-flight fence with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        vecInFlightFences.push_back(inFlightFence);
    }
    // Map everything to the window handle.
    _mapWindowToVecImageAvailableSemaphores[windowHandle] = ::std::move(vecImageAvailableSemaphores);
    _mapWindowToVecRenderFinishedSemaphores[windowHandle] = ::std::move(vecRenderFinishedSemaphores);
    _mapWindowToVecInFlightFences[windowHandle] = ::std::move(vecInFlightFences);

    celeriqueLogTrace("Created sync objects.");
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

/// @brief Draw graphics to a window.
/// @param windowHandle The handle to the window to be drawn graphics on.
/// @param graphicsPipelineConfigId The identifier for the graphics pipeline configuration to be used for drawing.
/// @param numVerticesToDraw The number of vertices to be drawn.
/// @param vertexStride The size of the individual vertex input.
/// @param numVertexElements The number of individual vertices to draw.
/// @param ptrVertexBuffer The pointer to the vertex buffer.
/// @param ptrIndexBuffer The pointer to the index buffer.
void celerique::vulkan::internal::Manager::drawOnWindow(
    Pointer windowHandle, PipelineConfigID graphicsPipelineConfigId, size_t numVerticesToDraw,
    size_t vertexStride, size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer
) {
    ::std::shared_lock<::std::shared_mutex> readLock(_sharedMutex);

    /// @brief The container for the result code from the vulkan api.
    VkResult result;
    /// @brief The graphics logical device assigned to the window.
    VkDevice graphicsLogicalDevice = _mapWindowToGraphicsLogicDev[windowHandle];
    /// @brief The current frame index being rendered.
    size_t currentFrameIndex = _mapWindowToCurrentFrameIndex[windowHandle];
    /// @brief The collection of in-flight fences for the window.
    const ::std::vector<VkFence>& vecInFlightFences = _mapWindowToVecInFlightFences[windowHandle];

    // Wait until the previous frame has finished rendering in the GPU.
    result = vkWaitForFences(graphicsLogicalDevice, 1, &vecInFlightFences[currentFrameIndex], VK_TRUE, UINT32_MAX);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to wait for in-flight fence with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The window's swapchain handle.
    VkSwapchainKHR swapChain = _mapWindowToSwapChain[windowHandle];
    /// @brief The collection of image available semaphores.
    const ::std::vector<VkSemaphore>& vecImageAvailableSemaphores = _mapWindowToVecImageAvailableSemaphores[windowHandle];

    /// @brief The index of the image to be rendered.
    uint32_t imageIndex = 0;
    // Obtain next image index.
    result = vkAcquireNextImageKHR(
        graphicsLogicalDevice, swapChain, UINT32_MAX,
        vecImageAvailableSemaphores[currentFrameIndex], VK_NULL_HANDLE, &imageIndex
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Simply return. The engine will eventually re-create the swapchain triggered by certain events.
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        ::std::string errorMessage = "Failed to acquire next image index with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Reset the fence for drawing in the GPU.
    result = vkResetFences(graphicsLogicalDevice, 1, &vecInFlightFences[currentFrameIndex]);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to reset in-flight fence with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The collection of the window's command buffer.
    const ::std::vector<VkCommandBuffer>& vecCommandBuffers = _mapWindowToVecCommandBuffers[windowHandle];
    // Reset the command buffer.
    result = vkResetCommandBuffer(vecCommandBuffers[currentFrameIndex], 0);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to reset command buffer with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The reference to the handle to the buffer containing vertex and index data.
    VkBuffer& refMeshBuffer = _mapWindowToVecMeshBuffers[windowHandle][currentFrameIndex];
    /// @brief The reference to the handle to the memory of the mesh buffer in the GPU.
    VkDeviceMemory& refMeshBufferMemory = _mapWindowToVecMeshBufferMemories[windowHandle][currentFrameIndex];

    fillMeshBuffer(
        numVerticesToDraw, vertexStride, numVertexElements, ptrVertexBuffer, ptrIndexBuffer,
        graphicsLogicalDevice, &refMeshBuffer, &refMeshBufferMemory
    );

    /// @brief Information about how the command buffer begins recording.
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    result = vkBeginCommandBuffer(vecCommandBuffers[currentFrameIndex], &commandBufferBeginInfo);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to begin command buffer with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The window's swapchain extent.
    const VkExtent2D& swapChainExtent = _mapWindowToSwapChainExtent[windowHandle];

    /// @brief The viewport description.
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // Set the viewport
    vkCmdSetViewport(vecCommandBuffers[currentFrameIndex], 0, 1, &viewport);

    /// @brief The scissor rectangle description.
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    // Set the scissor
    vkCmdSetScissor(vecCommandBuffers[currentFrameIndex], 0, 1, &scissor);

    /// @brief The clear value.
    VkClearValue clearValue;
    clearValue.color = {0.0f, 0.0f, 0.0f, 0.01}; // Setting the screen to black.
    /// @brief The window's collection of frame buffers.
    const ::std::vector<VkFramebuffer>& vecSwapChainFrameBuffers = _mapWindowToVecSwapChainFrameBuffers[windowHandle];

    /// @brief Information about beginning render pass.
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = _pairRenderPassToLogicDev.first;
    renderPassBeginInfo.framebuffer = vecSwapChainFrameBuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = _mapWindowToSwapChainExtent[windowHandle];
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearValue;
    // Begin render pass.
    vkCmdBeginRenderPass(vecCommandBuffers[currentFrameIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    /// @brief The handle to the graphics pipeline to be used for rendering.
    VkPipeline graphicsPipeline = _mapGraphicsPipelineIdToPipeline[graphicsPipelineConfigId];
    // Bind the command buffer to the graphics pipeline.
    vkCmdBindPipeline(vecCommandBuffers[currentFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    /// @brief The collection of offset values for the mesh buffer.
    VkDeviceSize arrOffsets[] = {0};
    // Vertex buffer specified.
    if (ptrVertexBuffer != nullptr && refMeshBuffer != nullptr) {
        vkCmdBindVertexBuffers(vecCommandBuffers[currentFrameIndex], 0, 1, &refMeshBuffer, arrOffsets);
    }

    // Index buffer specified.
    if (ptrIndexBuffer != nullptr && refMeshBuffer != nullptr) {
        // Bind the indices.
        vkCmdBindIndexBuffer(
            vecCommandBuffers[currentFrameIndex], refMeshBuffer,
            static_cast<VkDeviceSize>(vertexStride * numVertexElements), VK_INDEX_TYPE_UINT32
        );
        vkCmdDrawIndexed(vecCommandBuffers[currentFrameIndex], static_cast<uint32_t>(numVerticesToDraw), 1, 0, 0, 0);
    }
    // No index buffer specified.
    else {
        vkCmdDraw(vecCommandBuffers[currentFrameIndex], static_cast<uint32_t>(numVerticesToDraw), 1, 0, 0);
    }

    // End the render pass.
    vkCmdEndRenderPass(vecCommandBuffers[currentFrameIndex]);
    // End command buffer recording.
    result = vkEndCommandBuffer(vecCommandBuffers[currentFrameIndex]);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to record command with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief Collection of wait stages.
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    /// @brief The collection of render finished semaphores.
    const ::std::vector<VkSemaphore>& vecRenderFinishedSemaphores = _mapWindowToVecRenderFinishedSemaphores[windowHandle];

    /// @brief Information to be submitted to the graphics queue.
    VkSubmitInfo graphicsQueueSubmitInfo = {};
    graphicsQueueSubmitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
    graphicsQueueSubmitInfo.commandBufferCount = 1;
    graphicsQueueSubmitInfo.pCommandBuffers = &vecCommandBuffers[currentFrameIndex];
    graphicsQueueSubmitInfo.waitSemaphoreCount = 1;
    graphicsQueueSubmitInfo.pWaitSemaphores = &vecImageAvailableSemaphores[currentFrameIndex];
    graphicsQueueSubmitInfo.pWaitDstStageMask = waitStages;
    graphicsQueueSubmitInfo.signalSemaphoreCount = 1;
    graphicsQueueSubmitInfo.pSignalSemaphores = &vecRenderFinishedSemaphores[currentFrameIndex];

    // Submit to the graphics queue. Signals the in-flight fence when graphics rendering is done.
    result = vkQueueSubmit(selectGraphicsQueue(graphicsLogicalDevice), 1, &graphicsQueueSubmitInfo, vecInFlightFences[currentFrameIndex]);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to submit to graphics queue with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief Presentation information.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vecRenderFinishedSemaphores[currentFrameIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    // Waits for the graphics rendering before
    // presenting the image back to the swapchain.
    result = vkQueuePresentKHR(selectPresentQueue(graphicsLogicalDevice), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Simply return. The engine will eventually re-create the swapchain triggered by certain events.
        return;
    } else if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to submit to present with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Update the current frame index.
    _mapWindowToCurrentFrameIndex[windowHandle] = (currentFrameIndex + 1) % vecSwapChainFrameBuffers.size();
}

/// @brief Fill the mesh buffer with vertices and indices to be drawn.
/// @param numVerticesToDraw The number of vertices to be drawn.
/// @param vertexStride The size of the individual vertex input.
/// @param numVertexElements The number of individual vertices to draw.
/// @param ptrVertexBuffer The pointer to the vertex buffer.
/// @param ptrIndexBuffer The pointer to the index buffer.
/// @param graphicsLogicalDevice The graphics logical device used to draw the window.
/// @param ptrMeshBuffer The pointer to the handle to the buffer containing vertex and index data.
/// @param ptrMeshBufferMemory The pointer to the handle to the memory of the mesh buffer in the GPU.
void celerique::vulkan::internal::Manager::fillMeshBuffer(
    size_t numVerticesToDraw, size_t vertexStride, size_t numVertexElements, void* ptrVertexBuffer, uint32_t* ptrIndexBuffer,
    VkDevice graphicsLogicalDevice, VkBuffer* ptrMeshBuffer, VkDeviceMemory* ptrMeshBufferMemory
) {
    // Destroy the old data if they exist.
    if (*ptrMeshBufferMemory != nullptr) {
        vkFreeMemory(graphicsLogicalDevice, *ptrMeshBufferMemory, nullptr);
        *ptrMeshBufferMemory = nullptr;
    }
    if (*ptrMeshBuffer != nullptr) {
        vkDestroyBuffer(graphicsLogicalDevice, *ptrMeshBuffer, nullptr);
        *ptrMeshBuffer = nullptr;
    }

    // Return immediately as there is nothing to fill.
    if (numVerticesToDraw == 0 || vertexStride == 0 || numVertexElements == 0 || ptrVertexBuffer == nullptr) return;

    /// @brief The variable that stores the result of any vulkan function called.
    VkResult result;

    /// @brief The size of the buffer to be allocated.
    VkDeviceSize bufferSize;
    // Without index buffer.
    if (ptrIndexBuffer == nullptr) {
        bufferSize = static_cast<VkDeviceSize>(vertexStride * numVertexElements);
    }
    // With index buffer.
    else {
        bufferSize = static_cast<VkDeviceSize>(
            vertexStride * numVertexElements + sizeof(uint32_t) * numVerticesToDraw
        );
    }

    /// @brief The CPU accessible objects buffer.
    VkBuffer stagingObjectsBuffer = nullptr;
    /// @brief The CPU accessible objects buffer memory.
    VkDeviceMemory stagingObjectsBufferMemory = nullptr;
    // Create resources for staging buffer and memory.
    createBufferAndAllocateMemory(
        graphicsLogicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingObjectsBuffer, &stagingObjectsBufferMemory
    );

    /// @brief The pointer to the CPU accessible buffer of `stagingObjectsBuffer`.
    void* ptrStagingDataSrc = nullptr;
    result = vkMapMemory(graphicsLogicalDevice, stagingObjectsBufferMemory, 0, bufferSize, 0, &ptrStagingDataSrc);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to map memory with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    // Fill in with the vertices data.
    memcpy(ptrStagingDataSrc, ptrVertexBuffer, vertexStride * numVertexElements);
    // If index buffer specified,
    if (ptrIndexBuffer != nullptr) {
        // Append the data buffer with the indices data.
        memcpy(
            reinterpret_cast<void*>(
                reinterpret_cast<Pointer>(ptrStagingDataSrc) +
                static_cast<Pointer>(vertexStride * numVertexElements)
            ),
            ptrIndexBuffer, numVerticesToDraw * sizeof(uint32_t)
        );
    }
    vkUnmapMemory(graphicsLogicalDevice, stagingObjectsBufferMemory);

    if (ptrIndexBuffer != nullptr) {
        createBufferAndAllocateMemory(
            graphicsLogicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ptrMeshBuffer, ptrMeshBufferMemory
        );
    } else {
        createBufferAndAllocateMemory(
            graphicsLogicalDevice, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ptrMeshBuffer, ptrMeshBufferMemory
        );
    }

    /// @brief The handle to the physical device that the logical device represents.
    VkPhysicalDevice physicalDevice = _mapLogicDevToPhysDev[graphicsLogicalDevice];
    /// @brief The command queue used for copy submission. (will be using the graphics queue).
    VkQueue copyCommandQueue = selectGraphicsQueue(graphicsLogicalDevice);

    copyVulkanBufferData(graphicsLogicalDevice, copyCommandQueue, stagingObjectsBuffer, *ptrMeshBuffer, bufferSize);

    // Destroy staging resources.
    vkFreeMemory(graphicsLogicalDevice, stagingObjectsBufferMemory, nullptr);
    vkDestroyBuffer(graphicsLogicalDevice, stagingObjectsBuffer, nullptr);
}

/// @brief Construct a collection shader stage create information structures.
/// @param logicalDevice The handle to the logical device that is used to create the pipeline.
/// @param ptrPipelineConfig The pointer to the pipeline configuration.
/// @return The collection of vulkan pipeline shader stages.
::std::vector<VkPipelineShaderStageCreateInfo> celerique::vulkan::internal::Manager::constructVecShaderStageCreateInfos(
    VkDevice logicalDevice, PipelineConfig* ptrPipelineConfig
) {
    /// @brief The container for the result code from the vulkan api.
    VkResult result;

    /// @brief The collection of shader stages.
    ::std::list<ShaderStage> listShaderStages = ptrPipelineConfig->listStages();
    /// @brief The collection of vulkan pipeline shader stages.
    ::std::vector<VkPipelineShaderStageCreateInfo> vecShaderStageCreateInfos;
    vecShaderStageCreateInfos.reserve(listShaderStages.size());

    // Iterating over shader stages.
    for (ShaderStage shaderStage : listShaderStages) {
        /// @brief The const reference to the shader program of the specified shader stage.
        const ShaderProgram& refShaderProgram = ptrPipelineConfig->shaderProgram(shaderStage);
        /// @brief The information about the shader module.
        VkShaderModuleCreateInfo shaderModuleInfo = {};
        shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleInfo.codeSize = refShaderProgram.size();
        shaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(refShaderProgram.ptrBuffer());
        /// @brief The shader module to be created.
        VkShaderModule shaderModule;
        // Create the shader module.
        result = vkCreateShaderModule(logicalDevice, &shaderModuleInfo, nullptr, &shaderModule);
        if (result != VK_SUCCESS) {
            ::std::string errorMessage = "Failed to create shader module with result " + ::std::to_string(result);
            celeriqueLogError(errorMessage);
            throw ::std::runtime_error(errorMessage);
        }
        _mapShaderModuleToLogicDev[shaderModule] = logicalDevice;

        /// @brief The information about the vulkan pipeline shader stage.
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.module = shaderModule;
        shaderStageCreateInfo.pName = "main";
        switch(shaderStage) {
        case CELERIQUE_SHADER_STAGE_VERTEX:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            celeriqueLogTrace("Created vertex shader module.");
            break;
        case CELERIQUE_SHADER_STAGE_TESSELLATION_CONTROL:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            celeriqueLogTrace("Created tesselation control shader module.");
            break;
        case CELERIQUE_SHADER_STAGE_TESSELLATION_EVALUATION:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            celeriqueLogTrace("Created tesselation evaluation shader module.");
            break;
        case CELERIQUE_SHADER_STAGE_GEOMETRY:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            celeriqueLogTrace("Created geometry shader module.");
            break;
        case CELERIQUE_SHADER_STAGE_FRAGMENT:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            celeriqueLogTrace("Created fragment shader module.");
            break;
        case CELERIQUE_SHADER_STAGE_COMPUTE:
            shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            celeriqueLogTrace("Created compute shader module.");
            break;
        }
        vecShaderStageCreateInfos.emplace_back(::std::move(shaderStageCreateInfo));
    }

    return vecShaderStageCreateInfos;
}

/// @brief Construct a collection of vertex attribute descriptions.
/// @param ptrPipelineConfig The pointer to the pipeline configuration.
/// @return The collection of vertex attribute descriptions.
::std::vector<VkVertexInputAttributeDescription> celerique::vulkan::internal::Manager::constructVecVertexAttributeDescriptions(
    PipelineConfig* ptrPipelineConfig
) {
    /// @brief The collection of input layouts.
    const ::std::list<InputLayout>& listInputLayouts = ptrPipelineConfig->listVertexInputLayouts();
    /// @brief The collection of vertex attribute descriptions.
    ::std::vector<VkVertexInputAttributeDescription> vecVertexAttributeDescriptions;
    vecVertexAttributeDescriptions.reserve(listInputLayouts.size());

    // Populate vertex attributes.
    for (const InputLayout& inputLayout : listInputLayouts) {
        VkVertexInputAttributeDescription vertexAttributeDescription = {};
        vertexAttributeDescription.binding = inputLayout.bindingPoint;
        vertexAttributeDescription.location = inputLayout.location;
        vertexAttributeDescription.offset = inputLayout.offset;

        switch(inputLayout.inputType) {
        case CELERIQUE_PIPELINE_INPUT_TYPE_FLOAT: {
            switch(inputLayout.numElements) {
            case 1:
                vertexAttributeDescription.format = VK_FORMAT_R32_SFLOAT;
                break;
            case 2:
                vertexAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 3:
                vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
                break;
            case 4:
                vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                celeriqueLogWarning(
                    "inputLayout.numElements = " + ::std::to_string(inputLayout.numElements) +
                    " is invalid."
                );
            }
        } break;

        case CELERIQUE_PIPELINE_INPUT_TYPE_INT: {
            switch(inputLayout.numElements) {
            case 1:
                vertexAttributeDescription.format = VK_FORMAT_R32_SINT;
                break;
            case 2:
                vertexAttributeDescription.format = VK_FORMAT_R32G32_SINT;
                break;
            case 3:
                vertexAttributeDescription.format = VK_FORMAT_R32G32B32_SINT;
                break;
            case 4:
                vertexAttributeDescription.format = VK_FORMAT_R32G32B32A32_SINT;
                break;
            default:
                celeriqueLogWarning(
                    "inputLayout.numElements = " + ::std::to_string(inputLayout.numElements) +
                    " is invalid."
                );
            }
        } break;

        case CELERIQUE_PIPELINE_INPUT_TYPE_DOUBLE: {
            switch(inputLayout.numElements) {
            case 1:
                vertexAttributeDescription.format = VK_FORMAT_R64_SFLOAT;
                break;
            case 2:
                vertexAttributeDescription.format = VK_FORMAT_R64G64_SFLOAT;
                break;
            case 3:
                vertexAttributeDescription.format = VK_FORMAT_R64G64B64_SFLOAT;
                break;
            case 4:
                vertexAttributeDescription.format = VK_FORMAT_R64G64B64A64_SFLOAT;
                break;
            default:
                celeriqueLogWarning(
                    "inputLayout.numElements = " + ::std::to_string(inputLayout.numElements) +
                    " is invalid."
                );
            }
        } break;

        case CELERIQUE_PIPELINE_INPUT_TYPE_BOOLEAN: {
            switch(inputLayout.numElements) {
            case 1:
                vertexAttributeDescription.format = VK_FORMAT_R8_UINT;
                break;
            case 2:
                vertexAttributeDescription.format = VK_FORMAT_R8G8_UINT;
                break;
            case 3:
                vertexAttributeDescription.format = VK_FORMAT_R8G8B8_UINT;
                break;
            case 4:
                vertexAttributeDescription.format = VK_FORMAT_R8G8B8A8_UINT;
                break;
            default:
                celeriqueLogWarning(
                    "inputLayout.numElements = " + ::std::to_string(inputLayout.numElements) +
                    " is invalid."
                );
            }
        } break;

        default:
            celeriqueLogWarning(
                "Unknown vertex attribute format for input type of value: " +
                ::std::to_string(inputLayout.inputType)
            );
        }

        vecVertexAttributeDescriptions.emplace_back(vertexAttributeDescription);
    }

    return vecVertexAttributeDescriptions;
}

/// @brief Create a buffer object and allocate memory.
/// @param logicalDevice The logical device used to create the resources.
/// @param deviceSize The size of the memory to be allocated.
/// @param usageFlags The buffer's usage.
/// @param memoryPropertyFlags The memory property flags raised.
/// @param ptrBuffer The pointer to the buffer handle.
/// @param ptrBufferMemory The pointer to the buffer memory handle.
void celerique::vulkan::internal::Manager::createBufferAndAllocateMemory(
    VkDevice logicalDevice,
    VkDeviceSize deviceSize,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    VkBuffer* ptrBuffer,
    VkDeviceMemory* ptrBufferMemory
) {
    /// @brief The variable that stores the result of any vulkan function called.
    VkResult result;

    /// @brief Information about the buffer to be created.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = deviceSize;
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Create the buffer.
    result = vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, ptrBuffer);
    if(result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create buffer with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief The memory requirements for the buffer.
    VkMemoryRequirements memoryRequirements = {};
    // Retrieve buffer's memory requirements.
    vkGetBufferMemoryRequirements(logicalDevice, *ptrBuffer, &memoryRequirements);

    /// @brief The handle to the physical device that the logical device represents.
    VkPhysicalDevice physicalDevice = _mapLogicDevToPhysDev[logicalDevice];

    /// @brief Information about the memory to be allocated.
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(
        physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags
    );

    // Allocate memory.
    result = vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, ptrBufferMemory);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to allocate memory with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Bind the vertex buffer to the memory.
    result = vkBindBufferMemory(logicalDevice, *ptrBuffer, *ptrBufferMemory, 0);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to bind buffer memory with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
}

/// @brief Find the memory type index of a given physical device.
/// @param physicalDevice The physical device specified.
/// @param typeFilter The bit field types that are suitable.
/// @param memoryPropertyFlags The memory property flags raised.
/// @return The memory type index value.
uint32_t celerique::vulkan::internal::Manager::findMemoryTypeIndex(
    VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags memoryPropertyFlags
) {
    /// @brief The container for the memory properties of the specified physical device.
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    // Find a memory type that is suitable for the buffer.
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
            return i;
        }
    }

    const char* errorMessage = "Failed to find a suitable memory type index.";
    celeriqueLogError(errorMessage);
    throw ::std::runtime_error(errorMessage);
}

/// @brief Copy the contents of a source vulkan buffer to a destination vulkan buffer.
/// @param logicalDevice The handle to the logical device to facilitate memory copying.
/// @param commandQueue The queue used for command submissions.
/// @param srcBuffer The buffer where the data is coming from.
/// @param dstBuffer The buffer where the data is to be copied to.
/// @param size The size of the data to be moved.
void celerique::vulkan::internal::Manager::copyVulkanBufferData(
    VkDevice logicalDevice, VkQueue commandQueue,
    VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size
) {
    /// @brief The command buffer for copying.
    VkCommandBuffer copyCommandBuffer = beginSingleTimeCommand(logicalDevice);

    /// @brief Information about how the copy happens.
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommand(logicalDevice, copyCommandBuffer, commandQueue);
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

/// @brief Begin a single time use command.
/// @param logicalDevice The handle to the logical device that manages the command.
/// @return The handle to the single time use command buffer.
VkCommandBuffer celerique::vulkan::internal::Manager::beginSingleTimeCommand(VkDevice logicalDevice) {
    /// @brief The variable that stores the result of any vulkan function called.
    VkResult result;

    /// @brief Information about the command
    VkCommandBufferAllocateInfo singleTimeCommandInfo = {};
    singleTimeCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    singleTimeCommandInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    singleTimeCommandInfo.commandPool = selectSingleTimeCommandPool(logicalDevice);
    singleTimeCommandInfo.commandBufferCount = 1;

    /// @brief The handle to the command buffer that will record the command.
    VkCommandBuffer singleTimeCommandBuffer;
    result = vkAllocateCommandBuffers(logicalDevice, &singleTimeCommandInfo, &singleTimeCommandBuffer);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to create single time use command buffer with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
        
    /// @brief How the command buffer begins recording.
    VkCommandBufferBeginInfo commandBeginInfo = {};
    commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Begin recording.
    result = vkBeginCommandBuffer(singleTimeCommandBuffer, &commandBeginInfo);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to begin command recording with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    return singleTimeCommandBuffer;
}

/// @brief End the single time use command.
/// @param logicalDevice The handle to the logical device that manages the command.
/// @param singleTimeCommandBuffer The handle to the single time use command buffer.
/// @param commandQueue The queue used for command submissions.
void celerique::vulkan::internal::Manager::endSingleTimeCommand(
    VkDevice logicalDevice, VkCommandBuffer singleTimeCommandBuffer, VkQueue commandQueue
) {
    /// @brief The variable that stores the result of any vulkan function called.
    VkResult result;

    // End recording.
    result = vkEndCommandBuffer(singleTimeCommandBuffer);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to end command recording with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    /// @brief Command submission info.
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &singleTimeCommandBuffer;

    result = vkQueueSubmit(commandQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to submit command with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Wait until the queue is done.
    result = vkQueueWaitIdle(commandQueue);
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Failed to wait for queue with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }

    // Free this command buffer as it will
    // no longer be used outside of this scope
    vkFreeCommandBuffers(logicalDevice, selectSingleTimeCommandPool(logicalDevice), 1, &singleTimeCommandBuffer);
}

/// @brief Select the command pool to use for a single time use command.
/// @param logicalDevice The handle to the logical device that manages the command.
/// @return The handle to the command pool to use.
VkCommandPool celerique::vulkan::internal::Manager::selectSingleTimeCommandPool(VkDevice logicalDevice) {
    // TODO: Select the best command pool. Will return the first one for now.
    return _mapLogicDevToVecCommandPools[logicalDevice][0];
}

/// @brief Select the best queue for graphics command submissions.
/// @param graphicsLogicalDevice The specified graphics logical device.
/// @return The handle to the graphics queue.
VkQueue celerique::vulkan::internal::Manager::selectGraphicsQueue(VkDevice graphicsLogicalDevice) {
    // TODO: Select the best graphics queue. Will return the first one for now.
    return _mapGraphicsLogicDevToVecGraphicsQueues[graphicsLogicalDevice][0];
}

/// @brief Select the best queue for present command submissions.
/// @param graphicsLogicalDevice The specified graphics logical device.
/// @return The handle to the present queue.
VkQueue celerique::vulkan::internal::Manager::selectPresentQueue(VkDevice graphicsLogicalDevice) {
    // TODO: Select the best present queue. Will return the first one for now.
    return _mapGraphicsLogicDevToVecPresentQueues[graphicsLogicalDevice][0];
}

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
        ::std::string errorMessage = "Error in calling vkEnumerateDeviceExtensionProperties "
        "with result " + ::std::to_string(result);
        celeriqueLogError(errorMessage);
        throw ::std::runtime_error(errorMessage);
    }
    ::std::vector<VkExtensionProperties> physicalDeviceExtensionProperties(physicalDeviceExtensionPropertiesCount);
    result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &physicalDeviceExtensionPropertiesCount,physicalDeviceExtensionProperties.data());
    if (result != VK_SUCCESS) {
        ::std::string errorMessage = "Error in calling vkEnumerateDeviceExtensionProperties "
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