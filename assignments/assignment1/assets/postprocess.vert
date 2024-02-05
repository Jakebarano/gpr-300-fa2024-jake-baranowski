#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Vertex position in model space
layout(location = 1) in vec3 vNormal; //Vertex position in model space
layout(location = 2) in vec2 vTexCoord; //Vertex texture coordinate (UV)

uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

out vec2 UV;

vec4 vertices[3] = {
	vec4(-1,-1,0,0), //Bottom left (X,Y,U,V)
	vec4(3,-1,2,0),  //Bottom right (X,Y,U,V)
	vec4(-1,3,0,2)   //Top left (X,Y,U,V)
};


void main(){
	
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));

	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;

	UV = vertices[gl_VertexID].zw;

	//Transform vertex position to homogeneous clip space
	gl_Position = _ViewProjection * _Model * vec4(vertices[gl_VertexID].xy,0,1);
}
