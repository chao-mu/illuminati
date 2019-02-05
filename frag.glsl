#version 410

layout (location = 0) out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

uniform sampler2D img0;

in vec2 texcoord;
in vec2 texcoordL;
in vec2 texcoordR;
in vec2 texcoordT;
in vec2 texcoordTL;
in vec2 texcoordTR;
in vec2 texcoordB;
in vec2 texcoordBL;
in vec2 texcoordBR;

vec4 applyKernel(in mat3 kernel) {
	vec4 bottomColor = texture(img0, texcoordB);
	vec4 bottomLeftColor = texture(img0, texcoordBL);
	vec4 bottomRightColor = texture(img0, texcoordBR);
	vec4 centerColor = texture(img0, texcoord);
	vec4 leftColor = texture(img0, texcoordL);
	vec4 rightColor = texture(img0, texcoordR);
	vec4 topColor = texture(img0, texcoordT);
	vec4 topRightColor = texture(img0, texcoordTR);
	vec4 topLeftColor = texture(img0, texcoordTL);

	vec4 resultColor = topLeftColor * kernel[0][0] + topColor * kernel[0][1] + topRightColor * kernel[0][2];
	resultColor += leftColor * kernel[1][0] + centerColor * kernel[1][1] + rightColor * kernel[1][2];
	resultColor += bottomLeftColor * kernel[2][0] + bottomColor * kernel[2][1] + bottomRightColor * kernel[2][2];

	return resultColor;
}

const mat3 edgeDetect = mat3(
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
);

void main() {
    vec3 col = texture(img0, texcoord).rgb; //applyKernel(edgeDetect).rgb;

	FragColor.rgb = texcoord.x > 0.5 ? vec3(0, 1, 0) : vec3(0,0,1);
    FragColor.r = texcoord.y > 0.5 ? 1 : 0;
    FragColor.rgb += 0.5;
    FragColor.rgb *= col;
    FragColor.a = 1;
}
