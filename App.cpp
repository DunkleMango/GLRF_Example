#include "App.hpp"

using namespace GLRF;

static const std::string cmakeSourceDir = "../../../";
static const std::string shaderLib = cmakeSourceDir + "shaders/";
static const std::string seperator = "_";

unsigned int tesselation = 0;
float lastX, lastY, currentX, currentY;
float xOffset, yOffset;
const glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 origin = glm::vec3(0.0f);
const glm::mat4 noRotation = glm::mat4(1.0f);

bool useHDR = true;
float exposure = 1.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void calculateMouseOffset();
void retesselatePlane(SceneMesh * obj, PlaneGenerator planeGen, unsigned int new_tesselation, glm::vec3 plane_position, glm::vec3 plane_normal, glm::vec3 plane_direction);

void renderPostFxNDC();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	ScreenResolution screenResolution;
	screenResolution.width = 1280;
	screenResolution.height = 720;
	lastX = (float)screenResolution.width / 2.0f;
	lastY = (float)screenResolution.height / 2.0f;
	currentX = lastX;
	currentY = lastY;

	GLFWwindow* window = glfwCreateWindow(screenResolution.width, screenResolution.height, "OpenGL", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//create scene
	PlaneGenerator planeGen = PlaneGenerator();
	std::shared_ptr<Camera> camera(new Camera(glm::vec3(0.f, 4.f, 10.f), upVector, origin));

	std::shared_ptr<SceneMesh> floor(new SceneMesh(planeGen.create(origin, glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 64.f, 10, 5.f), GL_STATIC_DRAW, GL_TRIANGLES));
	SceneNode<SceneMesh> floor_node(floor);
	std::shared_ptr<PointLight> pointLight_white(new PointLight(glm::vec3(1.0f, 1.0f, 1.0f), 1.f));
	SceneNode<PointLight> pointLight_white_node(pointLight_white);
	pointLight_white_node.setPosition(glm::vec3(0.f, 0.5f, 1.f));
	std::shared_ptr<PointLight> pointLight_red(new PointLight(glm::vec3(1.f, 0.1f, 0.1f), 0.5f));
	SceneNode<PointLight> pointLight_red_node(pointLight_red);
	pointLight_red_node.setPosition(glm::vec3(2.f, 0.5f, 1.f));
	std::shared_ptr<PointLight> pointLight_blue(new PointLight(glm::vec3(0.1f, 0.1f, 1.f), 0.5f));
	SceneNode<PointLight> pointLight_blue_node(pointLight_blue);
	pointLight_blue_node.setPosition(glm::vec3(2.f, 0.5f, 1.4f));
	std::shared_ptr<PointLight> pointLight_green(new PointLight(glm::vec3(0.1f, 1.f, 0.1f), 2.f));
	SceneNode<PointLight> pointLight_green_node(pointLight_green);
	pointLight_green_node.setPosition(glm::vec3(-2.f, 0.5f, 1.f));
	std::shared_ptr<PointLight> powerPointLight(new PointLight(glm::vec3(1.f, 1.f, 0.9f), 5.f));
	SceneNode<PointLight> powerPointLight_node(powerPointLight);
	powerPointLight_node.setPosition(glm::vec3(0.f, 2.f, 0.f));
	std::shared_ptr<DirectionalLight> dirLight(new DirectionalLight(1.f));
	SceneNode<DirectionalLight> dirLight_node(dirLight);
	dirLight_node.rotateRad(glm::vec3(0.f, 1.f, 0.f), M_PI / 4.f);

	Material mat = Material();
	mat.height_scale = 1.f;
	// TODO - fix path issues with textures (use absolute path?)
	mat.loadTextures(cmakeSourceDir + "textures/", "imported/PavingStones053_4K", seperator, "jpg");
	floor_node.getObject()->setMaterial(mat);

	Scene scene = Scene(camera);
	scene.addObject(floor_node);
	scene.addObject(pointLight_white_node);
	scene.addObject(pointLight_red_node);
	scene.addObject(pointLight_blue_node);
	scene.addObject(pointLight_green_node);
	scene.addObject(powerPointLight_node);
	//scene.addObject(dirLight_node);

	ShaderOptions sceneShaderOptions;
	sceneShaderOptions.useFrameBuffer = true;
	sceneShaderOptions.isFrameBufferHDR = true;
	sceneShaderOptions.texColorBufferAmount = 2;
	sceneShaderOptions.screenResolution = screenResolution;
	//Shader sceneShader(shaderLib, "base.vert", "base.frag", sceneShaderOptions);
	Shader sceneShader(shaderLib, "parallax.vert", "parallax.frag", sceneShaderOptions);

	ShaderOptions postBlurShaderOptions;
	postBlurShaderOptions.useDepthBuffer = false;
	postBlurShaderOptions.useFrameBuffer = true;
	postBlurShaderOptions.useMultipleFrameBuffers = true;
	postBlurShaderOptions.texColorBufferAmount = 2;
	postBlurShaderOptions.screenResolution = screenResolution;
	Shader postBlurShader(shaderLib, "gauss.vert", "gauss.frag", postBlurShaderOptions);

	Shader postHDRShader(shaderLib, "hdr.vert", "hdr.frag", ShaderOptions());
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//pre-calculations
	glm::mat4 projection = glm::perspective(glm::radians(55.0f), (GLfloat)screenResolution.width / (GLfloat)screenResolution.height, 0.1f, 100.0f);

	//pre-render updates
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	sceneShader.use();

	postBlurShader.use();
	postBlurShader.setValue("image", 0);

	postHDRShader.use();
	postHDRShader.setValue("scene", 0);
	postHDRShader.setValue("bloomBlur", 1);

	sceneShader.use();

	//rendering-loop
	while (!glfwWindowShouldClose(window)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mouse_callback);
		
		//input
		processInput(window);
		calculateMouseOffset();
		scene.processMouse(xOffset, yOffset);
		scene.processInput(window);

		//glfwSetScrollCallback(window, scroll_callback);
		//retesselatePlane(&obj, planeGen, next_tesselation, plane_position, plane_normal, plane_direction

		//updates
		sceneShader.setValue("projection", projection);

		//rendering
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, sceneShader.getFrameBuffer(0));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		sceneShader.use();
		scene.draw(sceneShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 0;
		postBlurShader.use();
		for (unsigned int i = 0; i < amount; i++) {
			unsigned int bufferOne = horizontal ? 0 : 1;
			unsigned int bufferTwo = horizontal ? 1 : 0;
			glBindFramebuffer(GL_FRAMEBUFFER, postBlurShader.getFrameBuffer(bufferOne));
			postBlurShader.setValue("horizontal", horizontal);
			postBlurShader.setValue("first_iteration", first_iteration);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? sceneShader.getColorBuffer(1) : postBlurShader.getColorBuffer(bufferTwo));  // bind texture of other framebuffer (or scene if first iteration)
			renderPostFxNDC();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		postHDRShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sceneShader.getColorBuffer(0));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, postBlurShader.getColorBuffer(horizontal ? 1 : 0));
		postHDRShader.setValue("exposure", exposure);
		renderPostFxNDC();
		
		//handle events and swap buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	currentX = (float)xpos;
	currentY = (float)ypos;
}

void calculateMouseOffset() {
	xOffset = currentX - lastX;
	yOffset = lastY - currentY;
	lastX = currentX;
	lastY = currentY;
}

void retesselatePlane(SceneMesh * obj, PlaneGenerator planeGen, unsigned int new_tesselation, glm::vec3 plane_position, glm::vec3 plane_normal, glm::vec3 plane_direction) {
	if (new_tesselation != tesselation) {
		obj->update(planeGen.create(plane_position, plane_normal, plane_direction, 1.0, new_tesselation, 1.0f));
		tesselation = new_tesselation;
	}
}

GLuint quadVAO = 0;
GLuint quadVBO;
void renderPostFxNDC() {
	if (quadVAO == 0) {
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}