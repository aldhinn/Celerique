/*

File: ./vulkan/tests/manager.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the vulkan resource management implementations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/vulkan/internal/manager.h>
#include <celerique/logging.h>

#include <gtest/gtest.h>
#include <algorithm>

namespace celerique { namespace vulkan {
    /// @brief The GTest unit test suite for the vulkan resource management system.
    class ManagerUnitTestCpp : public ::testing::Test {
    protected:
        internal::Manager& refManager = internal::Manager::getRef();
    };

    TEST_F(ManagerUnitTestCpp, retrieveInstance) {
        internal::Manager& localRefManager = internal::Manager::getRef();
    }

    TEST_F(ManagerUnitTestCpp, queryVulkanInstanceExtensions) {
        // Query instance extensions
        unsigned int extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
        VkExtensionProperties* extensions = static_cast<VkExtensionProperties*>(
            alloca(sizeof(VkExtensionProperties) * extensionCount)
        );
        vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
        // Log info all the extension names.
        for (unsigned int i = 0; i < extensionCount; i++) {
            celeriqueLogInfo(
                "Vulkan extension available [" + ::std::to_string(i) + "]: " +
                extensions[i].extensionName
            );
        }
    }

    TEST_F(ManagerUnitTestCpp, checkGetUniqueIndicesCorrectness) {
        ::std::vector<uint32_t> vecIndices1;
        ::std::vector<uint32_t> vecIndices2;
        ::std::vector<uint32_t> vecExpectedUniqueIndices;
        ::std::vector<uint32_t> vecActualUniqueIndices = internal::Manager::getUniqueIndices(vecIndices1, vecIndices2);

        // The results have to be sorted as different compilers and systems will order the elements differently.
        ::std::sort(vecActualUniqueIndices.begin(), vecActualUniqueIndices.end());
        GTEST_ASSERT_EQ(vecExpectedUniqueIndices, vecActualUniqueIndices);

        vecIndices1 = { 0, 1, 2, 3 };
        vecIndices2 = { 0, 2, 5, 6 };
        vecExpectedUniqueIndices = { 0, 1, 2, 3, 5, 6 };
        vecActualUniqueIndices = internal::Manager::getUniqueIndices(vecIndices1, vecIndices2);

        // The results have to be sorted as different compilers and systems will order the elements differently.
        ::std::sort(vecActualUniqueIndices.begin(), vecActualUniqueIndices.end());
        GTEST_ASSERT_EQ(vecExpectedUniqueIndices, vecActualUniqueIndices);

        vecIndices1 = { 0, 4, 2, 3 };
        vecIndices2 = { 1, 2, 5, 2 };
        vecExpectedUniqueIndices = { 0, 1, 2, 3, 4, 5 };
        vecActualUniqueIndices = internal::Manager::getUniqueIndices(vecIndices1, vecIndices2);

        // The results have to be sorted as different compilers and systems will order the elements differently.
        ::std::sort(vecActualUniqueIndices.begin(), vecActualUniqueIndices.end());
        GTEST_ASSERT_EQ(vecExpectedUniqueIndices, vecActualUniqueIndices);
    }
}}