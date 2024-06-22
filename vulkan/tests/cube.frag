#version 450

/// @brief The colour of a vertex of a triangle.
layout(location = 0) in vec4 inColour;

/// @brief The calculated colour value of the pixel.
layout(location = 0) out vec4 outColor;

/// @brief Shader entrypoint.
void main() {
    outColor = inColour;
}