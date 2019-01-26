#version 330 core

#define PI acos(-1.)

out vec4 FragColor;
uniform float iTime;
uniform vec2 iResolution;

uniform float j1DownTimeTotal;
uniform float j1UpTimeTotal;
uniform float j1LeftTimeTotal;
uniform float j1RightTimeTotal;
uniform float j1ATimeTotal;
uniform float j1BTimeTotal;
uniform float j1LTimeTotal;
uniform float j1RTimeTotal;

#define DOWN_TIME j1DownTimeTotal
#define UP_TIME j1UpTimeTotal
#define LEFT_TIME j1LeftTimeTotal
#define RIGHT_TIME j1RightTimeTotal
#define ROTATE_TIME j1ATimeTotal
#define ZOOM_TIME j1BTimeTotal
#define CHROMA_TIME (j1LTimeTotal - j1RTimeTotal)

vec3 SG(vec2 uv) {
    return vec3(
           (2. * uv) / (1 + dot(uv, uv)),
           (-1 + dot(uv, uv)) / (1 + dot(uv, uv))
    );
}

mat2 rotate(float a) {
    return mat2(cos(a), -sin(a), sin(a), cos(a));
}

vec2 opRep(vec2 p, float c) {
    float d = c * .5;
    
    return mod(p - d, c) - d; 
}

float sdSquare(vec2 p, float r, float thickness) {
    p = abs(p);

    return abs(max(p.x, p.y) - r) - thickness;
}

float scene(vec2 p) {
    p = opRep(p, 1);
    
    p *= rotate(ROTATE_TIME);

    return sdSquare(p, 0.3, 0.02);
}

void main() {
    vec2 uv = (2. * gl_FragCoord.xy - iResolution) / iResolution.y;
    
    uv *= sin(ZOOM_TIME);
    
    vec3 p = SG(uv);
    p.yz *= rotate(DOWN_TIME - UP_TIME);
    p.xz *= rotate(LEFT_TIME - RIGHT_TIME);
    uv = p.xy / (1 - p.z);
 
    vec3 rgb = vec3(0);
    
    float amount = sin(CHROMA_TIME) * .05 + .05;
    rgb.r = scene(uv - amount);
    rgb.g = scene(uv + amount);
    rgb.b = scene(uv + amount / 2.);
    
    FragColor = vec4(rgb, 1);
}
