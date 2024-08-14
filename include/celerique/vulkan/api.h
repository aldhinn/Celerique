/*

File: ./include/celerique/vulkan/api.h
Author: Aldhinn Espinas
Description: This header file contains interfaces to vulkan API related objects.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_VULKAN_API_HEADER_FILE)
#define CELERIQUE_VULKAN_API_HEADER_FILE

#include <celerique/defines.h>
#include <celerique/graphics.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <memory>

namespace celerique { namespace vulkan {
    /// @brief Gets the interface to the vulkan graphics API.
    /// @return The shared pointer to the vulkan graphics API interface.
    CELERIQUE_SHARED_SYMBOL ::std::shared_ptr<IGraphicsAPI> getGraphicsApiInterface();
}}
#endif
// End C++ Only Region.
#endif
// End of file.
// DO NOT WRITE BEYOND HERE.