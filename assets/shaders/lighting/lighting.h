
#define MAX_SPOT_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_DIRECTIONAL_LIGHTS 10

struct Color
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Light
{
  vec3 direction;
  vec3 position;

  Color color;  
  float shininess;
  float ambient_strength;

  float constant;
  float linear;
  float exponent;
};
uniform Light point_lights[MAX_POINT_LIGHTS];
int num_point_lights;

uniform struct
{
  Light light;
	
  float cutoff;
  float outer_cutoff; 
} spot_lights[MAX_SPOT_LIGHTS];
int num_spot_lights;

struct DirectionalLight
{
  vec3 direction;
  vec3 position;	
};
uniform directional_lights[MAX_DIRECTIONAL_LIGHTS];
int num_directional_lights;

vec3 CalculateDiffuseComponent(const in vec3 L, const in vec3 N, const in vec3 Kd)
{
  float diffuseStrength = max(dot(N, L), 0.0);
  return diffuseStrength *  
}

float CalculateSpecularComponent(const in vec3 V, const in vec3 L, const in vec3 N, const in vec3 Ks, const in float shininess)
{
  if(dot(N, L) > 0.0)
  {
      vec3 H = normalize(V + L);
      float specularStrength = pow(max(dot(H, N), 0.0f), shininess);
      return specularStrength * Ks; 
  }
  return vec3(0.0);
}

float CalculateAttenuation(const in float constant, const in float linear, const in float exponent, const in float D)
{
  float attenuation = constant + linear * D + exponent * D * D;
  return max(1.0f, attenuation);
}



vec3 CalculatePointLight(const in vec3 Kd, const in vec3 Ks, const in vec3 N)
{
   return vec3(0.0);
}

vec3 CalculateDirectionalLight()
{
   return vec3(0.0);
}

vec3 CalculateSpotLight()
{
  return vec3(0.0);
}