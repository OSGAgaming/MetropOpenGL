#version 330 core
out vec4 FragColor;
in vec3 color;
in vec2 texCoord;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform int Frame;

void main() {
    // Sample from both textures to combine current and previous frames
    vec4 currentColor = texture(tex0, texCoord);
    vec4 previousColor = texture(tex1, texCoord);
    vec4 averageColor = texture(tex2, texCoord);

    FragColor = currentColor; // Example blend of current and old frames
}