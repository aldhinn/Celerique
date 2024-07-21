#version 450

/// @brief The calculated normal vector for the fragment shader.
layout(location = 0) in vec3 inCalculatedNormal;

/// @brief The calculated colour value of the pixel.
layout(location = 0) out vec4 outColor;

/// @brief Shader entrypoint.
void main() {
    vec3 normalizedNormal = normalize(inCalculatedNormal);

    if (normalizedNormal.x > 0.9f) {
        // Right face (x-axis positive)
        outColor = vec4(0.0f, 0.3f, 0.5f, 1.0f);
    } else if (normalizedNormal.y > 0.9f) {
        // Front face (y-axis positive)
        outColor = vec4(0.0f, 0.4f, 0.6f, 1.0f);
    } else if (normalizedNormal.z > 0.9f) {
        // Top face (z-axis positive)
        outColor = vec4(0.0f, 0.6f, 0.8f, 1.0f);
    } else {
        // Black color for other faces
        outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}