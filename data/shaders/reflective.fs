
varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform samplerCube u_texture;
uniform vec3 u_camera_position; // camera

void main()
{
	vec3 view = normalize(v_world_position-u_camera_position);	//calculem el vector direcció
	vec3 R = reflect(view, v_normal);
	vec4 environment = textureCube(u_texture, R);            //llegim la textura

	gl_FragColor = environment;
}