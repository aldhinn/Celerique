/*

File: ./core/tests/pipeline.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the the pipeline functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/pipeline.h>
#include <celerique/logging.h>

#include <gtest/gtest.h>

#include <list>
#include <thread>
#include <mutex>
#include <utility>
#include <unordered_set>

namespace celerique {
    /// @brief The GTest unit test suite for the generic pipeline API tests.
    class PipelineUnitTestCpp : public ::testing::Test {
    protected:
        ShaderProgram _mockShaderProgram = ShaderProgram(2, new Byte[2]);
    };

    TEST_F(PipelineUnitTestCpp, deferredShaderStageSetting) {
        using Pointer = CeleriquePointer;

        /// @brief The pipeline config of which shader stage is to be after initialization.
        PipelineConfig pipelineConfig;
        /// @brief The value of the pointer of the mock shader program data buffer.
        Pointer ptrBuffer = reinterpret_cast<Pointer>(_mockShaderProgram.ptrBuffer());
        // Assign mock shader program to the pipeline configuration's specified shader stage type.
        pipelineConfig.shaderProgram(CELERIQUE_SHADER_STAGE_UNSPECIFIED) = ::std::move(_mockShaderProgram);

        celeriqueLogDebug("ptrBuffer address = " + ::std::to_string(ptrBuffer));
        // Check shader program pointer value.
        GTEST_ASSERT_EQ(ptrBuffer, reinterpret_cast<Pointer>(pipelineConfig.shaderProgram(CELERIQUE_SHADER_STAGE_UNSPECIFIED).ptrBuffer()));
    }

    TEST_F(PipelineUnitTestCpp, verifyShaderSrcExtensionParsing) {
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file"), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file.cpp.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("some/file.glsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("glsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl."), CELERIQUE_SHADER_SRC_LANG_NULL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("glsl.hlsl"), CELERIQUE_SHADER_SRC_LANG_HLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
        GTEST_ASSERT_EQ(fileExtToShaderSrcLang("hlsl.hlsl.glsl"), CELERIQUE_SHADER_SRC_LANG_GLSL);
    }

    TEST_F(PipelineUnitTestCpp, uniquePipelineConfigIdentifiers) {
        /// @brief The limit as to how many iterations we should generate id's.
        const size_t iterations = 1000;
        /// @brief The list of id's generated.
        ::std::list<PipelineConfigID> listGeneratedIds;
        /// @brief The mutex object that controls access to `listGeneratedIds`.
        ::std::mutex listGeneratedIdsMutex;
        /// @brief The list of threads that would generate ids.
        ::std::list<::std::thread> listWorkerThreads;

        // Generate id's in separate threads.
        for (size_t i = 0; i < iterations; i++) {
            /// @brief The worker thread for this iteration to generate an id.
            ::std::thread workerThread([&](){
                ::std::lock_guard<::std::mutex> writeLock(listGeneratedIdsMutex);
                listGeneratedIds.emplace_back(genPipelineConfigID());
            });
            // Collect the worker thread.
            listWorkerThreads.emplace_back(::std::move(workerThread));
        }
        // Wait for each worker thread to finish.
        for (::std::thread& refWorkerThread : listWorkerThreads) {
            refWorkerThread.join();
        }

        /// @brief The unordered set to contain the generated id's.
        ::std::unordered_set<PipelineConfigID> setGeneratedIds;
        // Iterate over the generated id's.
        for (PipelineConfigID generatedId : listGeneratedIds) {
            // generated id should never be null.
            GTEST_ASSERT_NE(generatedId, CELERIQUE_PIPELINE_CONFIG_ID_NULL);
            // Fail the test if found as it means the generated id is a repeat.
            if (setGeneratedIds.find(generatedId) != setGeneratedIds.end()) {
                GTEST_ASSERT_FALSE(true);
            }
            // Add if not yet found.
            setGeneratedIds.emplace(generatedId);
        }

        // The sizes should match.
        GTEST_ASSERT_EQ(listGeneratedIds.size(), setGeneratedIds.size());
    }

    TEST_F(PipelineUnitTestCpp, uniqueGpuBufferIdentifiers) {
        /// @brief The limit as to how many iterations we should generate id's.
        const size_t iterations = 1000;
        /// @brief The list of id's generated.
        ::std::list<GpuBufferID> listGeneratedIds;
        /// @brief The mutex object that controls access to `listGeneratedIds`.
        ::std::mutex listGeneratedIdsMutex;
        /// @brief The list of threads that would generate ids.
        ::std::list<::std::thread> listWorkerThreads;

        // Generate id's in separate threads.
        for (size_t i = 0; i < iterations; i++) {
            /// @brief The worker thread for this iteration to generate an id.
            ::std::thread workerThread([&](){
                ::std::lock_guard<::std::mutex> writeLock(listGeneratedIdsMutex);
                listGeneratedIds.emplace_back(genGpuBufferId());
            });
            // Collect the worker thread.
            listWorkerThreads.emplace_back(::std::move(workerThread));
        }
        // Wait for each worker thread to finish.
        for (::std::thread& refWorkerThread : listWorkerThreads) {
            refWorkerThread.join();
        }

        /// @brief The unordered set to contain the generated id's.
        ::std::unordered_set<GpuBufferID> setGeneratedIds;
        // Iterate over the generated id's.
        for (GpuBufferID generatedId : listGeneratedIds) {
            // generated id should never be null.
            GTEST_ASSERT_NE(generatedId, CELERIQUE_GPU_BUFFER_ID_NULL);
            // Fail the test if found as it means the generated id is a repeat.
            if (setGeneratedIds.find(generatedId) != setGeneratedIds.end()) {
                GTEST_ASSERT_FALSE(true);
            }
            // Add if not yet found.
            setGeneratedIds.emplace(generatedId);
        }

        // The sizes should match.
        GTEST_ASSERT_EQ(listGeneratedIds.size(), setGeneratedIds.size());
    }
}