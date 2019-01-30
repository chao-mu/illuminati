#version 410

out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iResolutionCap0;

uniform sampler2D cap0;

vec3 fetch(vec2 uv) {
    uv = uv * .5 + .5;
    return texture(cap0, uv * iResolution / iResolutionCap0 * 1.4).rgb;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / iResolution) * 2. - 1;
    
    vec2 polar = vec2(length(uv), atan(uv.y, uv.x));
    
    vec3 rgb = fetch(polar);
 
    FragColor.rgb = rgb.rgb; 
    FragColor.a = 1;
}
