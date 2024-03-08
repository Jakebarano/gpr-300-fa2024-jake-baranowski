#version 450
out vec4 FragColor; //The color of this fragment
in vec2 UV;

	//bind these in lighting pass from geopass AND refactor deferred frag stuff.
	uniform layout(binding = 0) sampler2D _gPositions;
	uniform layout(binding = 1) sampler2D _gNormals;  //used instead of Surface struct
	uniform layout(binding = 2) sampler2D _gAlbedo;
	uniform layout(binding = 3) sampler2D _shadowMap;

	uniform vec3 _EyePos;
	uniform vec3 _LightDirection;
	uniform vec3 _LightColor = vec3(1.0); //White Light
	uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);
	uniform mat4 _LightViewProj; //view + projection of light source camera

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

	//Light Pointing straight down
	vec3 toLight = -_LightDirection;

	float calcShadow(sampler2D shadowMap, vec4 lightSpacePos, vec3 norm){ //make normal variable input
		//Homogeneous Clip space to NDC [-w,w] to [-1,1]
		vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
		//Convert from [-1,1] to [0,1]
		sampleCoord = sampleCoord * 0.5 + 0.5;

		float bias = max(_Shadow.maxBias * (1.0 - dot(norm,toLight)),_Shadow.minBias);

		float myDepth = sampleCoord.z - bias; 

		float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
		//step(a,b) returns 1.0 if a >= b, 0.0 otherwise
		//return step(shadowMapDepth,myDepth);

		float totalShadow = 0;
		vec2 texelOffset = 1.0 /  textureSize(shadowMap,0);
		for(int y = -1; y <=1; y++){
			for(int x = -1; x <=1; x++){
				vec2 uv = sampleCoord.xy + vec2(x * texelOffset.x, y * texelOffset.y);
				totalShadow+=step(texture(shadowMap,uv).r,myDepth);
			}
		}
		totalShadow/=9.0;

		return totalShadow;
	}

	//Point Light Stuff
	//WIP

	#define MAX_POINT_LIGHTS 64

	struct PointLight{
		vec3 position;
		float radius;
		vec4 color;
	};
	uniform PointLight _PointLights[MAX_POINT_LIGHTS];
	

	float attenuateExponential(float distance, float radius){
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
	
}

	vec3 calcPointLight(PointLight light,vec3 normal, vec3 worldPos){
		vec3 diff = light.position - worldPos;
		//Direction toward light position
		vec3 toLight = normalize(diff);
		//TODO: Usual blinn-phong calculations for diffuse + specular
		float diffuseFactor = max(dot(normal, toLight), 0.0);

		vec3 toEye = normalize(_EyePos - worldPos); //Direction towards Eye

		vec3 h = normalize(toLight + toEye); //Blinn-Phong uses half angle
		float specularFactor = pow(max(dot(normal,h), 0.0), _Material.Shininess);

		float diffuse = _Material.Kd * diffuseFactor;
		float specular = _Material.Ks * specularFactor;

		vec3 lightColor = (diffuse + specular) * light.color.rgb;
		//Attenuation
		float d = length(diff); //Distance to light
		lightColor *= attenuateExponential(d,light.radius); //See below for attenuation options

		return lightColor;
}


	vec3 calculateLighting(vec3 normal, vec3 worldPos){

	vec4 lightSpacePos = _LightViewProj * vec4(worldPos, 1.0f);

		float diffuseFactor = max(dot(normal, toLight), 0.0);

		vec3 toEye = normalize(_EyePos - worldPos); //Direction towards Eye

		vec3 h = normalize(toLight + toEye); //Blinn-Phong uses half angle
		float specularFactor = pow(max(dot(normal,h), 0.0), _Material.Shininess);

		vec3 ambient = _AmbientColor * _Material.Ka;
		float diffuse = _Material.Kd * diffuseFactor;
		float specular = _Material.Ks * specularFactor;

		float shadow = calcShadow(_shadowMap, lightSpacePos, normal); 

		//Combination of specular and diffuse reflection
		vec3 lightColor = ambient + ( diffuse + specular) * (1.0 - shadow);

		return lightColor;

	}

void main(){

	vec3 normal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	vec3 totalLight = vec3(0);
	totalLight += calculateLighting(normal, worldPos);

	for(int i = 0; i < MAX_POINT_LIGHTS; i++){
		totalLight+=calcPointLight(_PointLights[i],normal, worldPos);
	}

	vec3 albedo = texture(_gAlbedo,UV).xyz;


	//vec3 lightColor = calculateLighting(normal, worldPos, albedo);
	//vec3 objectColor = texture(_MainTex, albedo).rgb;

	FragColor = vec4(albedo * totalLight, 0);
}
