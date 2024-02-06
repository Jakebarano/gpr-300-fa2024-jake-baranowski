#version 450
out vec2 UV;
vec4 vertices[3] = {
	vec4(-1,-1,0,0), //Bottom left (X,Y,U,V)
	vec4(3,-1,2,0),  //Bottom right (X,Y,U,V)
	vec4(-1,3,0,2)   //Top left (X,Y,U,V)
};
void main(){
	UV = vertices[gl_VertexID].zw;
	gl_Position = vec4(vertices[gl_VertexID].xy,0,1);
}

