#version 330 core
out vec4 FragColor;
in vec3 color;
in vec2 texCoord;
in vec3 normal;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform int Frame;

void main() {
    vec4 currentColor = texture(tex0, texCoord);

    FragColor = currentColor;
}