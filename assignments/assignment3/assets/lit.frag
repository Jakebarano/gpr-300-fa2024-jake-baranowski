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
	uniform vec3 _LightDirection;
	uniform vec3 _LightColor = vec3(1.0); //White Light
	uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

	struct Shadow {
		float minBias; //Example values! 
		float maxBias;
	};
	uniform Shadow _Shadow;

	struct Material {
		float Ka;  //Ambient coefficient (0-1)
		float Kd;  //Diffuse coefficient (0-1)
		float Ks;  //Specular coefficient (0-1)
		float Shininess; //Affects size of specular highlight
	};
	uniform Material _Material;

	in vec4 LightSpacePos;
	uniform sampler2D _ShadowMap; 

	vec3 normal = normalize(fs_in.WorldNormal);
	//Light Pointing straight down
	vec3 toLight = -_LightDirection;

	float calcShadow(sampler2D shadowMap, vec4 lightSpacePos){
		//Homogeneous Clip space to NDC [-w,w] to [-1,1]
		vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
		//Convert from [-1,1] to [0,1]
		sampleCoord = sampleCoord * 0.5 + 0.5;

		float bias = max(_Shadow.maxBias * (1.0 - dot(normal,toLight)),_Shadow.minBias);

		float myDepth = sampleCoord.z - bias; 

		float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
		//step(a,b) returns 1.0 if a >= b, 0.0 otherwise
		//return step(shadowMapDepth,myDepth);

		float totalShadow = 0;
		vec2 texelOffset = 1.0 /  textureSize(_ShadowMap,0);
		for(int y = -1; y <=1; y++){
			for(int x = -1; x <=1; x++){
				vec2 uv = sampleCoord.xy + vec2(x * texelOffset.x, y * texelOffset.y);
				totalShadow+=step(texture(_ShadowMap,uv).r,myDepth);
			}
		}
		totalShadow/=9.0;

		return totalShadow;
	}



void main(){
	//Make sure fragment normal is still length 1 after interpolation.
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	vec3 toEye = normalize(_EyePos - fs_in.WorldPos); //Direction towards Eye

	vec3 h = normalize(toLight + toEye); //Blinn-Phong uses half angle
	float specularFactor = pow(max(dot(normal,h), 0.0), _Material.Shininess);

	vec3 ambient = _AmbientColor * _Material.Ka;
	float diffuse = _Material.Kd * diffuseFactor;
	float specular = _Material.Ks * specularFactor;

	float shadow = calcShadow(_ShadowMap, LightSpacePos); 


	//Combination of specular and diffuse reflection
	vec3 lightColor = ambient + ( diffuse + specular) * (1.0 - shadow);
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor, 1.0);
}
