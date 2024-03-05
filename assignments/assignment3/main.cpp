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
#include <ew/procGen.h>

//jb namespace includes
#include <jb/frameBuffer.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI(unsigned int shadowMap, jb::FrameBuffer gBuffer);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;
bool GammaCorrectionOn = false; 

//ew objects
ew::Camera camera;
ew::Transform monkeyTransform;
ew::Transform planeTransform;
ew::CameraController cameraController;

ew::Camera shadowCam;

//light variables
glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, 0.0f);
struct Shadow {
	float minBias = 0.0015;
	float maxBias = 0.005;
}shadow;

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


jb::FrameBuffer createGBuffer(unsigned int width, unsigned int height) {
	jb::FrameBuffer framebuffer;
	framebuffer.width = width;
	framebuffer.height = height;

	glCreateFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

	int formats[3] = {
		GL_RGB32F, //0 = World Position 
		GL_RGB16F, //1 = World Normal
		GL_RGB16F  //2 = Albedo
	};
	//Create 3 color textures
	for (size_t i = 0; i < 3; i++)
	{
		glGenTextures(1, &framebuffer.colorBuffers[i]);
		glBindTexture(GL_TEXTURE_2D, framebuffer.colorBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, height);
		//Clamp to border so we don't wrap when sampling for post processing
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//Attach each texture to a different slot.
	//GL_COLOR_ATTACHMENT0 + 1 = GL_COLOR_ATTACHMENT1, etc
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, framebuffer.colorBuffers[i], 0);
	}
	//Explicitly tell OpenGL which color attachments we will draw to
	const GLenum drawBuffers[3] = {
			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, drawBuffers);
	// texture2D depth buffer 4 GBuffer
	
	unsigned int gDepthBuffer;
	glGenTextures(1, &gDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, gDepthBuffer);
	//Create 16 bit depth buffer - must be same width/height of color buffer
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, screenWidth, screenHeight);
	//Attach to framebuffer (assuming FBO is bound)
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthBuffer, 0);

	//Checking for completeness
	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete: %d", fboStatus);
	}

	//Clean up global state

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return framebuffer;
}


int main() {
	GLFWwindow* window = initWindow("Assignment 3", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader shadowMapShader = ew::Shader("assets/depthOnly.vert", "assets/depthOnly.frag");
	ew::Shader geometryShader = ew::Shader("assets/geometryPass.vert", "assets/geometryPass.frag");

	ew::Shader LitShader = ew::Shader("assets/lit.vert", "assets/lit.frag");  //links vert and frag   //ARCHIV

	//ew::Shader deferredLitShader = ew::Shader("assets/deferredLit.vert", "assets/deferredLit.frag"); //Use instead of Lit
	ew::Shader Postprocess = ew::Shader("assets/postprocess.vert", "assets/postprocess.frag");  //links vert and frag


	ew::Model monkeyModel = ew::Model("assets/suzanne.obj"); //load the rock Monkey
	ew::Mesh planeMesh = ew::Mesh(ew::createPlane(10, 10, 5));
	planeTransform.position = glm::vec3(0, -1.1f, 0);

	//Main Camera

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;  //maybe move into framebuffer?
	camera.fov = 60.0f; //Vertical field of view, in degrees

	//ShadowCam
	shadowCam.target = glm::vec3(0.0f, 0.0f, 0.0f);
	shadowCam.position = shadowCam.target - lightDir * 5.0f; //check this
	shadowCam.orthographic = true;
	shadowCam.orthoHeight = 10.0f;
	shadowCam.nearPlane = 0.001f;
	shadowCam.farPlane = 15.0f;
	shadowCam.aspectRatio = 1.0f;

	//Handles to OpenGL object are unsigned integers
	GLuint brickTexture = ew::loadTexture("assets/Rock051_2K-JPG_Color.jpg"); //Stopped Right here (12/24pgs)

	//Global OpenGL variables
	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); //Back face culling
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
	unsigned int shadowFBO, shadowMap;
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

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//Geometry Creation
	unsigned int gFBO;
	jb::FrameBuffer gBuffer = createGBuffer(screenWidth, screenHeight);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime(); 
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		//SHADOW PASS

		glCullFace(GL_FRONT);

		shadowCam.position = shadowCam.target - lightDir * 5.0f;

		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glViewport(0, 0, 2048, 2048);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightProjMat = shadowCam.projectionMatrix();
		glm::mat4 lightViewMat = shadowCam.viewMatrix();

		glm::mat4 lightViewProj = lightProjMat * lightViewMat;

		shadowMapShader.use();
		shadowMapShader.setMat4("_ViewProjection", lightViewProj);
		shadowMapShader.setMat4("_Model", monkeyTransform.modelMatrix());

		monkeyModel.draw();

		shadowMapShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		glCullFace(GL_BACK);
	
		//monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0)); 
		cameraController.move(window, &camera, deltaTime);
		glBindTextureUnit(0, brickTexture);
		glBindTextureUnit(1, shadowMap);

		//END OF SHADOW PASS
		
		//GEOMETRY PASS

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.fbo); 
		glViewport(0, 0, gBuffer.width, gBuffer.height); 
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		
		geometryShader.use();
		LitShader.setInt("_MainTex", 0);
		geometryShader.setMat4("_Model", monkeyTransform.modelMatrix());
		geometryShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		monkeyModel.draw(); //Draws monkey model using current shader
		geometryShader.setMat4("_Model", planeTransform.modelMatrix());
		planeMesh.draw();

		//END OF GEOMETRY PASS
		
		//LIGHTING PASS

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		LitShader.use();   

		LitShader.setInt("_ShadowMap", 1);
		LitShader.setInt("_MainTex", 0);  //Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
		LitShader.setVec3("_EyePos", camera.position);
		LitShader.setVec3("_LightDirection", lightDir);

		LitShader.setFloat("_Material.Ka", material.Ka);
		LitShader.setFloat("_Material.Kd", material.Kd);
		LitShader.setFloat("_Material.Ks", material.Ks);
		LitShader.setFloat("_Material.Shininess", material.Shininess);
		LitShader.setFloat("_Shadow.minBias", shadow.minBias);
		LitShader.setFloat("_Shadow.maxBias", shadow.maxBias);

		LitShader.setMat4("_Model", monkeyTransform.modelMatrix());
		LitShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		LitShader.setMat4("_LightViewProj", lightViewProj);
		monkeyModel.draw(); //Draws monkey model using current shader

		LitShader.setMat4("_Model", planeTransform.modelMatrix());

		planeMesh.draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//END OF LIGHTING PASS
		
		//POSTPROCESS PASS

		Postprocess.use();
		Postprocess.setFloat("_gammaPower.Kp", gammaPower.Kp);
	
		glBindTextureUnit(0, colorBuffer);
		glBindVertexArray(dummyVAO);

		//6 vertices for quad, 3 for triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
		drawUI(shadowMap, gBuffer);

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

void drawUI(unsigned int shadowMap, jb::FrameBuffer gBuffer) {
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
	if (ImGui::CollapsingHeader("Light")) {
		ImGui::SliderFloat3("Direction", (float*)&lightDir, -1.0f, 1.0f);
		ImGui::SliderFloat("Minimum Bias", &shadow.minBias, 0.001f, 0.5f);
		ImGui::SliderFloat("Maximum Bias", &shadow.maxBias, 0.001f, 0.5f);
	}
	if (ImGui::CollapsingHeader("Gamma Correction")) {
		ImGui::SliderFloat("Power", &gammaPower.Kp, 1.0f, 2.2f);
	}
	ImGui::End(); 

	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)shadowMap, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Begin("GBuffers"); 
	{
		ImVec2 texSize = ImVec2(gBuffer.width / 4, gBuffer.height / 4);
		for (size_t i = 0; i < 3; i++)
		{
			ImGui::Image((ImTextureID)gBuffer.colorBuffers[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}

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

