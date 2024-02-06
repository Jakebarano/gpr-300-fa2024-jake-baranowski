//Invert effect fragment shader
#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;
void main(){
	vec3 color = 1.0-texture(_ColorBuffer,UV).rgb;
	FragColor = vec4(color,1.0);
}

