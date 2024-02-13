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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;
bool GammaCorrectionOn = false;


//Postprocess variables
unsigned int fbo, colorBuffer, depthBuffer;

//Shadow Map variables
unsigned int shadowFBO, shadowMap;

//ew objects

ew::Camera camera;
ew::Transform monkeyTransform;
ew::CameraController cameraController;

ew::Camera shadowCam;

//Material Struct
struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

struct gammaPower {
	float Kp = 1.0;
}gammaPower;


int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");  //links vert and frag
	ew::Shader Postprocess = ew::Shader("assets/postprocess.vert", "assets/postprocess.frag");  //links vert and frag
	ew::Model monkeyModel = ew::Model("assets/suzanne.obj"); //load the rock Monkey

	//Main Camera

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;  //maybe move into framebuffer?
	camera.fov = 60.0f; //Vertical field of view, in degrees

	//ShadowCam
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);

	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/Rock051_2K-JPG_Color.jpg"); //Stopped Right here (12/24pgs)


	//Global OpenGL variables
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	unsigned int fbo, colorBuffer;
	//Create Framebuffer Object
	glCreateFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//Create 8 bit RGBA color buffer
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, screenWidth, screenHeight);
	//Attach color buffer to framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorBuffer, 0);

	unsigned int depthBuffer;
	glGenTextures(1, &depthBuffer);
	glBindTexture(GL_TEXTURE_2D, depthBuffer);
	//Create 16 bit depth buffer - must be same width/height of color buffer
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, screenWidth, screenHeight);
	//Attach to framebuffer (assuming FBO is bound)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

	//DummyVAO 
	unsigned int dummyVAO;
	glCreateVertexArrays(1, &dummyVAO);

	//Shadow Map Creation
	glCreateFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	//16 bit depth values, 2k res
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Pixels outside frustum should have max distance (white).
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);



	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime(); 
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0)); 
		cameraController.move(window, &camera, deltaTime);

		shader.use();

		shader.setInt("_MainTex", 0);  //Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
		shader.setVec3("_EyePos", camera.position);

		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);

		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		monkeyModel.draw(); //Draws monkey model using current shader

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Postprocess.use();
		Postprocess.setFloat("_gammaPower.Kp", gammaPower.Kp);
	
		glBindTextureUnit(0, colorBuffer);
		glBindVertexArray(dummyVAO);

		//6 vertices for quad, 3 for triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);

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

void toggleGammaCorrection()
{
	if (GammaCorrectionOn == true)
	{
		GammaCorrectionOn = false;
	}
	else
	{
		GammaCorrectionOn = true;
	}
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
	if (ImGui::CollapsingHeader("Gamma Correction")) {
		ImGui::SliderFloat("Power", &gammaPower.Kp, 1.0f, 2.2f);
	}
	/*if (ImGui::Button("Toggle Gamma Correction")) {
		toggleGammaCorrection();
	}*/


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

