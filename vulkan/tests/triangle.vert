#version 450

layout(location = 0) out vec3 fragColour;

vec3 positions[3] = vec3[3](vec3(0.0, -0.5, 0.0), vec3(0.5, 0.5, 0.0), vec3(-0.5, 0.5, 0.0));
vec3 colors[3] = vec3[3](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

/// @brief Shader entrypoint.
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 1.0);
    fragColour = colors[gl_VertexIndex];
}