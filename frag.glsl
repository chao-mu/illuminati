#version 410

layout (location = 0) out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

uniform sampler2D img0;
uniform sampler2D lastOut;
uniform bool firstPass;

in vec2 texcoord;

void main() {
    vec3 col = vec3(1);
    
    FragColor.rgb = col;
    FragColor.a = 1;
}
