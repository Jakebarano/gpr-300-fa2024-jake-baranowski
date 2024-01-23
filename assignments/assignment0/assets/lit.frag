#version 450
out vec4 FragColor; //The color of this fragment
in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	}fs_in;

	uniform sampler2D _MainTex; //2D texture sampler
	uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
	uniform vec3 _LightColor = vec3(1.0); // White Light

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);

	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight), 0.0);
	vec3 diffuseColor = _LightColor * diffuseFactor;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * diffuseColor, 1.0);
}
