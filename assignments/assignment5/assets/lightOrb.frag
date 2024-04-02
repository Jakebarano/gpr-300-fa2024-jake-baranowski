#version 450 core
out vec4 FragColor;

uniform vec3 _Color;

void main(){
	FragColor = vec4(_Color,1.0);
}


