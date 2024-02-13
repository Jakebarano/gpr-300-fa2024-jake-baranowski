#version 450
out vec4 FragColor; //The color of this fragment
in vec2 UV;
in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	}fs_in;

	uniform sampler2D _MainTex; //2D texture sampler
	uniform vec3 _EyePos;
	uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
	uniform vec3 _LightColor = vec3(1.0); //White Light
	uniform vec3 _AmbientColor = vec3(0.8, 0.2, 0.8);


	struct Material {
		float Ka;  //Ambient coefficient (0-1)
		float Kd;  //Diffuse coefficient (0-1)
		float Ks;  //Specular coefficient (0-1)
		float Shininess; //Affects size of specular highlight
	};
	uniform Material _Material;

void main(){
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(fs_in.WorldNormal);

	//Light Pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	vec3 toEye = normalize(_EyePos - fs_in.WorldPos); //Direction towards Eye

	vec3 h = normalize(toLight + toEye); //Blinn-Phong uses half angle
	float specularFactor = pow(max(dot(normal,h), 0.0), _Material.Shininess);

	//Combination of specular and diffuse reflection
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor +=_AmbientColor * _Material.Ka;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor, 1.0);
}
