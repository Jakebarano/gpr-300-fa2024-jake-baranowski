#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//assignment includes
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <jb/transformNode.h>

using namespace jb;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//ew objects

ew::Camera camera;
ew::CameraController cameraController;

jb::TransformNode monkeyTransform;


//Material Struct
struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

jb::Hierarchy hierarchy;

void SolveFK(jb::Hierarchy hierarchy)
{
	for (int i = 0; i < hierarchy.nodeCount; i++)
	{
		if (hierarchy.nodes[i]->parentIndex == -1)
		{
			hierarchy.nodes[i]->globalTransform = hierarchy.nodes[i]->modelMatrix();
		}
		else
		{ 
			hierarchy.nodes[i]->globalTransform = hierarchy.nodes[hierarchy.nodes[i]->parentIndex]->globalTransform * hierarchy.nodes[i]->modelMatrix();
		}
	}
}


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);


	//TODO: add intro code below.

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");  //links vert and frag
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj"); //load the rock Monkey

	//Hierarchy Stuff

	hierarchy.addNode(0, -1, monkeyTransform); //Torso / Original Monkey
	hierarchy.nodes[0]->setValues(glm::vec3(0, 0, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
	//Left
	hierarchy.addNode(1, 0,  monkeyTransform); //ShoulderL
	hierarchy.nodes[1]->setValues(glm::vec3(-1.2f, 0, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));
	hierarchy.addNode(2, 1, monkeyTransform); //ArmL
	hierarchy.nodes[2]->setValues(glm::vec3(-1.2f, -0.5f, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));
	hierarchy.addNode(3, 2, monkeyTransform); //WristL
	hierarchy.nodes[3]->setValues(glm::vec3(-1.2f, -1.0f, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));

	//Right
	hierarchy.addNode(4, 0, monkeyTransform); //ShoulderL
	hierarchy.nodes[4]->setValues(glm::vec3(1.2f, 0, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));
	hierarchy.addNode(5, 4, monkeyTransform); //ArmL
	hierarchy.nodes[5]->setValues(glm::vec3(1.2f, -0.5f, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));
	hierarchy.addNode(6, 5, monkeyTransform); //WristL
	hierarchy.nodes[6]->setValues(glm::vec3(1.2f, -1.0f, 0), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(0.25f));

	//Camera

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;  //maybe move into framebuffer?
	camera.fov = 60.0f; //Vertical field of view, in degrees

	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/Rock051_2K-JPG_Color.jpg"); //Stopped Right here (12/24pgs)


	//Global OpenGL variables
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime(); 
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[0]->rotation = glm::rotate(hierarchy.nodes[0]->rotation, deltaTime, glm::vec3(0.0, 1.0, 1.0));
		hierarchy.nodes[1]->rotation = glm::rotate(hierarchy.nodes[1]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[2]->rotation = glm::rotate(hierarchy.nodes[2]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[3]->rotation = glm::rotate(hierarchy.nodes[3]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[4]->rotation = glm::rotate(hierarchy.nodes[4]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[5]->rotation = glm::rotate(hierarchy.nodes[5]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
		hierarchy.nodes[6]->rotation = glm::rotate(hierarchy.nodes[6]->rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		SolveFK(hierarchy);

		cameraController.move(window, &camera, deltaTime);

		//Bind brick texture to texture unit 0 
		//glActiveTexture(GL_TEXTURE0);
		glBindTextureUnit(0, brickTexture);
		//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
		shader.use();

		shader.setInt("_MainTex", 0);
		shader.setVec3("_EyePos", camera.position);

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);


		for (int i = 0; i < hierarchy.nodeCount; i++)
		{
			shader.setMat4("_Model", hierarchy.nodes[i]->modelMatrix());
			shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
			monkeyModel.draw(); //Draws monkey model using current shader
		}

		//shader.setMat4("_Model", monkeyTransform.modelMatrix());
		//shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		//monkeyModel.draw(); //Draws monkey model using current shader

		drawUI();
		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}



//Camera reset

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}


void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}


	ImGui::End(); 

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

