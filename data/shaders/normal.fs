
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform sampler2D u_normal_texture;

// material
uniform vec3 u_ka;
uniform vec3 u_kd;
uniform vec3 u_ks;
uniform float u_alpha;

// llum
uniform vec3 u_ia;
uniform vec3 u_id;
uniform vec3 u_is;
uniform vec3 u_light_pos;

// camera
uniform vec3 u_camera_position;

void main()
{
	vec4 albedo = texture2D(u_texture, v_uv);	
	//vec4 normal = v_normal;
	//if (u_normal_texture != nullptr)
	vec3 normal = texture2D(u_normal_texture, v_uv).xyz;	
	vec3 ambient = u_ka * u_ia; // component ambient

	// component difusa
	vec3 N = normalize(normal);
	vec3 L = normalize(u_light_pos - v_world_position);
	float LdotN = clamp(dot(L,N), 0.0, 1.0);
	vec3 difuse = u_kd * LdotN * u_id;
	
	// component especular
	vec3 V = normalize(u_camera_position - v_world_position);
	vec3 R = normalize(reflect(-L, N));
	float RdotV = clamp(dot(R,V), 0.0, 1.0);
	vec3 specular = u_ks * pow(RdotV, u_alpha) * u_is;
	
	// total
	vec3 ip = ambient + difuse + specular;
	gl_FragColor = vec4(ip, 1.0) * albedo;
	
	//gl_FragColor = albedo * vec4(u_ia, 1.0);
}
