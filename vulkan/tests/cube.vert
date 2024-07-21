#version 450

/// @brief The position of a point of a triangle in the world.
layout(location = 0) in vec3 inPosition;
/// @brief The normal vector of the surface.
layout(location = 1) in vec3 inNormal;

/// @brief The calculated normal vector for the fragment shader.
layout(location = 0) out vec3 outCalculatedNormal;

/// @brief Shader entrypoint.
void main() {
    mat4 model = mat4(1.0f); // Identity matrix for model

    // Manually create the view matrix for camera position at (2, 2, 1.8) looking at the origin (0, 0, 0)
    vec3 cameraPos = vec3(2.0f, 2.0f, 1.8f);
    vec3 cameraTarget = vec3(0.0f, 0.0f, 0.0f);
    vec3 up = vec3(1.0f, 1.0f, 0.0f);

    /// @brief The "forward" vector
    vec3 zaxis = normalize(cameraPos - cameraTarget);
    /// @brief The "right" vector
    vec3 xaxis = normalize(cross(up, zaxis));
    /// @brief The "up" vector
    vec3 yaxis = cross(zaxis, xaxis);

    mat4 view = mat4(1.0f);
    view[0][0] = xaxis.x; view[1][0] = xaxis.y; view[2][0] = xaxis.z; view[3][0] = -dot(xaxis, cameraPos);
    view[0][1] = yaxis.x; view[1][1] = yaxis.y; view[2][1] = yaxis.z; view[3][1] = -dot(yaxis, cameraPos);
    view[0][2] = zaxis.x; view[1][2] = zaxis.y; view[2][2] = zaxis.z; view[3][2] = -dot(zaxis, cameraPos);

    // Perspective projection matrix
    float fov = radians(45.0f);
    float aspectRatio = 1.0f; // Assuming square viewport for simplicity
    float near = 0.1f;
    float far = 100.0f;
    float tanHalfFov = tan(fov / 2.0f);

    mat4 projection = mat4(0.0);
    projection[0][0] = 1.0f / (aspectRatio * tanHalfFov);
    projection[1][1] = 1.0f / tanHalfFov;
    projection[2][2] = -(far + near) / (far - near);
    projection[2][3] = -1.0f;
    projection[3][2] = -(2.0f * far * near) / (far - near);

    outCalculatedNormal = mat3(transpose(inverse(model))) * inNormal;
    gl_Position = projection * view * model * vec4(inPosition, 1.0f);
}