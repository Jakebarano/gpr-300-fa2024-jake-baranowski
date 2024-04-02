#version 450 core
//Vertex attributes
layout(location = 0) in vec3 vPos;

uniform mat4 _Model; 
uniform mat4 _ViewProjection;

void main(){
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
