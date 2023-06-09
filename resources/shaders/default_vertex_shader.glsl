#version 330

#define MAX_LIGHTS_PER_WORLD 12

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 lights_projection_matrix[MAX_LIGHTS_PER_WORLD];
uniform int lights_count;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec4 fragPosLightSpace[MAX_LIGHTS_PER_WORLD];
// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vertexPosition;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = vertexNormal;
    for (int i = 0; i < lights_count; i++)
    {
        fragPosLightSpace[i] = lights_projection_matrix[i] * vec4(fragPosition, 1.0);
    }
    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}