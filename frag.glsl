#version 410

layout (location = 0) out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

uniform sampler2D img0;
uniform sampler2D lastOut;
uniform bool firstPass;

in vec2 texcoord;

#define MAX_STEPS 100

mat2 rotate(float a) {
    return mat2(cos(a), -sin(a), sin(a), cos(a));
}

void main() {
    vec2 uv = texcoord * 2 - 1.;
    
    vec2 st = normalize(vec2(uv.y, uv.y - uv.x));
    float a = atan(dot(st, vec2(0, 1)));
    
    uv += 0.12;
    uv.x += 0.08;
    uv *= rotate(a * 2);
    uv *= rotate(abs(a));
    uv *= rotate(0.75);
    uv /= 1.3;
    
    vec3 col = texture(img0, uv * .5 + .5).rgb;
    
    FragColor.rgb = col;
    FragColor.a = 1;
}
