
varying vec3 v_world_position;

uniform samplerCube u_texture;

// camera
uniform vec3 u_camera_position;

void main()
{
	vec3 view = normalize(v_world_position-u_camera_position);	//calculem el vector direcció
	vec4 environment = textureCube(u_texture, view);            //llegim la textura

	gl_FragColor = environment;	
}