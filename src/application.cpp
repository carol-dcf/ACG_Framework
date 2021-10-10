#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include <cmath>

bool render_wireframe = false;
Camera* Application::camera = nullptr;
Application* Application::instance = NULL;

Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	scene_exposure = 1;
	output = 0;
	ambient_light = Vector3(1.0, 0.6, 0.3);

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective

	{
		
		// SKYBOX
		skybox = new SkyboxNode("Skybox node");
		skybox->mesh = Mesh::Get("data/meshes/box.ASE.mbin");
		Matrix44 sky_model;
		sky_model.translate(camera->eye.x, camera->eye.y, camera->eye.z);
		sky_model.scale(10.0, 10.0, 10.0);
		skybox->model = sky_model;
		// li assignem un material
		SkyboxMaterial* mat_skybox = new SkyboxMaterial();	
		// li assignem una textura
		Texture* skybox_texture = new Texture();    
		skybox_texture->cubemapFromImages("data/environments/city");
		mat_skybox->texture = skybox_texture;
		mat_skybox->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/skybox.fs");
		skybox->material = mat_skybox;		
		node_list.push_back(skybox);
		
		///// SCENE ELEMENTS
		/// STANDARD MATERIAL : Banquito
		SceneNode* standard_node = new SceneNode("Standard Material");
		StandardMaterial* standard_mat = new StandardMaterial();
		standard_mat->texture = Texture::Get("data/models/bench/albedo.png");
		standard_node->mesh = Mesh::Get("data/models/bench/bench.obj.mbin");
		standard_node->model.setTranslation(-2.5, -0.7, -0.6);
		standard_node->model.scale(1.8, 1.8, 1.8);
		standard_node->material = standard_mat;
		node_list.push_back(standard_node);
		
		/// PHONG MATERIAL : Helmet
		PhongMaterial* mat = new PhongMaterial();
		Texture* albedo = Texture::Get("data/models/helmet/albedo.png");
		//Texture* normal = Texture::Get("data/models/helmet/normal.png");
		mat->texture = albedo;
		//mat->normal_texture = normal;
		mat->k_alpha = 5.0;
		mat->k_ambient = Vector3(1.0, 1.0, 1.0);
		mat->k_difuse = Vector3(1.0, 1.0, 1.0);
		mat->k_specular = Vector3(1.0, 1.0, 1.0);
		SceneNode* node = new SceneNode("Phong Material");
		node->mesh = Mesh::Get("data/models/helmet/helmet.obj.mbin");
		//node->model.scale(5, 5, 5);
		node->material = mat;
		mat->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/phong.fs");
		node_list.push_back(node);

		// LIGHT 1
		Light* light_1 = new Light("first light");
		light_1->specular = Vector3(1.0, 1.0, 1.0);
		light_1->difuse = Vector3(1.0, 1.0, 1.0);
		light_1->position = Vector3(-50.0 , 50.0, 50.0);
		light_list.push_back(light_1);

		// LIGHT 2
		Light* light_2 = new Light("second light");
		light_2->specular = Vector3(1.0, 0.0, 0.0);
		light_2->difuse = Vector3(1.0, 1.0, 1.0);
		light_2->position = Vector3(50.0, 50.0, 50.0);
		light_list.push_back(light_2);
		
		/// REFLECTIVE MATERIAL : sphere
		SceneNode* ref_node = new SceneNode("Reflective Material");
		ReflectiveMaterial* ref_mat = new ReflectiveMaterial();
		ref_mat->texture = skybox_texture;
		ref_node->mesh = Mesh::Get("data/meshes/sphere.obj.mbin");
		ref_node->model.setTranslation(2.5, 0.0, 0.0);
		//ref_node->model.scale(5, 5, 5);
		ref_node->material = ref_mat;
		node_list.push_back(ref_node);

		


		
	}
	
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

//what to do when the image has to be draw
void Application::render(void)
{
	//set the clear color (the background color)
	glClearColor(.1,.1,.1, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set the camera as default
	camera->enable();

	//render skybox
	glDisable(GL_DEPTH_TEST);
	skybox->material->render(skybox->mesh, skybox->model, camera);
	//set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	for (size_t i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
}

void Application::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 1;
	
	//example
	float angle = seconds_elapsed * 10.f * DEG2RAD;
	/*for (int i = 0; i < root.size(); i++) {
		root[i]->model.rotate(angle, Vector3(0,1,0));
	}*/

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT && !ImGui::IsAnyWindowHovered() 
		&& !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())) //is left button pressed?
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	//async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move fast er with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f,-1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f,  1.0f, 0.0f) * speed);

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	//actualitzem el centre del skybox
	skybox->model.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
}

//Keyboard event handler (sync input)
void Application::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: render_debug = !render_debug; break;
		case SDLK_F2: render_wireframe = !render_wireframe; break;
		case SDLK_F5: Shader::ReloadAll(); break; 
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
		case SDL_MOUSEWHEEL:
		{
			if (event.x > 0) io.MouseWheelH += 1;
			if (event.x < 0) io.MouseWheelH -= 1;
			if (event.y > 0) io.MouseWheel += 1;
			if (event.y < 0) io.MouseWheel -= 1;
		}
	}

	if(!ImGui::IsAnyWindowHovered() && event.y)
		camera->changeDistance(event.y * 0.5);
}

void Application::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

void Application::onFileChanged(const char* filename)
{
	Shader::ReloadAll();
}
