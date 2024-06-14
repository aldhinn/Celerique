/*

File: ./core/tests/pipeline.gtest.cpp
Author: Aldhinn Espinas
Description: This tests the the pipeline functionalities.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#include <celerique/pipeline.h>
#include <celerique/logging.h>

#include <gtest/gtest.h>

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
}