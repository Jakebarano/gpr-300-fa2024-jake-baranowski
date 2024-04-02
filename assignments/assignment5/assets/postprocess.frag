//Invert effect fragment shader
#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

struct gammaPower {
	float Kp; //power of 0-2.2
};
uniform gammaPower _gammaPower;

void main(){

	vec3 color = texture(_ColorBuffer,UV).rgb;
	color = pow(color,vec3(1.0/_gammaPower.Kp));

	FragColor = vec4(color,1.0);
}

