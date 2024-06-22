#version 450

/// @brief The position of a point of a triangle in the world.
layout(location = 0) in vec4 inPosition;
/// @brief The position of a point of a triangle in the world.
layout(location = 1) in vec4 inColour;

/// @brief The colour value to be fed to the fragment shader.
layout(location = 0) out vec4 outColor;

/// @brief Shader entrypoint.
void main() {
    gl_Position = inPosition;
    outColor = inColour;
}