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
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void calculateMouseOffset();
void retesselatePlane(SceneMesh<VertexFormat>* obj, PlaneGenerator planeGen, unsigned int new_tesselation, glm::vec3 plane_position, glm::vec3 plane_normal, glm::vec3 plane_direction);
void renderPostFxNDC();

int main()
{
	// ======= INITIALIZATION ======= //
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

	// ======= FRAMEBUFFERS ======= //
	FrameBufferConfiguration fb_main_cfg, fb_blur_cfg, fb_hdr_cfg;
	fb_main_cfg.color_profile = GL_RGBA16F;
	fb_main_cfg.color_type = GL_RGBA;
	fb_main_cfg.data_type = GL_FLOAT;
	fb_main_cfg.use_depth_buffer = true;
	fb_main_cfg.num_color_buffers = 2;
	fb_blur_cfg.use_depth_buffer = false;
	fb_blur_cfg.color_profile = GL_RGBA16F;
	fb_blur_cfg.color_type = GL_RGBA;
	fb_blur_cfg.data_type = GL_FLOAT;
	fb_hdr_cfg.use_depth_buffer = false;
	FrameBuffer fb_main = FrameBuffer(fb_main_cfg, screenResolution);
	FrameBuffer fb_blur_horizontal = FrameBuffer(fb_blur_cfg, screenResolution);
	FrameBuffer fb_blur_vertical = FrameBuffer(fb_blur_cfg, screenResolution);
	FrameBuffer fb_hdr = FrameBuffer(fb_hdr_cfg, screenResolution);
	fb_main.setDebugName("FBO_MAIN");
	fb_blur_horizontal.setDebugName("FBO_BLUR_H");
	fb_blur_vertical.setDebugName("FBO_BLUR_V");
	fb_hdr.setDebugName("FBO_HDR");
	std::cout << "[CreateFramebuffers] " << glGetError() << std::endl;

	// ======= SHADER LOADING ======= //
	Shader shader_main(shaderLib, "parallax.vert", std::nullopt, "parallax.frag");
	Shader shader_post_blur(shaderLib, "gauss.vert", std::nullopt, "gauss.frag");
	Shader shader_post_hdr(shaderLib, "hdr.vert", std::nullopt, "hdr.frag");
	shader_main.setDebugName("SHADER_MAIN");
	shader_post_blur.setDebugName("SHADER_BLUR_H");
	shader_post_hdr.setDebugName("SHADER_HDR");
	std::cout << "[LoadShaders] " << glGetError() << std::endl;

	// ======= SHADER INIT ======= //
	std::map<GLuint, FrameBuffer*> map_shader_fbs;
	ShaderManager& shader_manager = ShaderManager::getInstance();

	shader_manager.useShader(shader_main.getID());
	map_shader_fbs.insert_or_assign(shader_main.getID(), &fb_main);
	shader_manager.useShader(shader_post_blur.getID());
	shader_manager.useShader(shader_post_hdr.getID());

	shader_manager.useShader(0);
	std::cout << "[InitShaders] " << glGetError() << std::endl;

	// ======= SCENE OBJECTS ======= //
	PlaneGenerator planeGen = PlaneGenerator();
	std::shared_ptr<Camera> camera(new Camera(glm::vec3(0.f, 4.f, 10.f), upVector, origin));
	camera->setSensitivityForTranslation(0.4f);
	camera->setSensitivityForRotation(2.f);

	std::shared_ptr<SceneMesh<VertexFormat>> floor(new SceneMesh(planeGen.create(origin, glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f, 0.f, 0.f), 64.f, 10, 5.f), GL_STATIC_DRAW));

	// ======= LIGHTS ======= //
	std::shared_ptr<PointLight> pointLight_white(new PointLight(glm::vec3(1.0f, 1.0f, 1.0f), 1.f));
	std::shared_ptr<PointLight> pointLight_red(new PointLight(glm::vec3(1.f, 0.1f, 0.1f), 0.5f));
	std::shared_ptr<PointLight> pointLight_blue(new PointLight(glm::vec3(0.1f, 0.1f, 1.f), 0.5f));
	std::shared_ptr<PointLight> pointLight_green(new PointLight(glm::vec3(0.1f, 1.f, 0.1f), 2.f));
	std::shared_ptr<PointLight> powerPointLight(new PointLight(glm::vec3(1.f, 1.f, 0.9f), 5.f));
	std::shared_ptr<DirectionalLight> dirLight(new DirectionalLight(0.5f));

	// ======= MATERIALS ======= //
	std::shared_ptr<Material> mat_ground = std::shared_ptr<Material>(new Material());
	mat_ground->height_scale = 1.f;
	// TODO - fix path issues with textures (use absolute path?)
	mat_ground->loadTextures(cmakeSourceDir + "textures/", "imported/PavingStones053_4K", seperator, "jpg");
	floor->setMaterial(mat_ground);

	std::cout << "[Materials] " << glGetError() << std::endl;

	// ======= SCENE ======= //
	Scene scene = Scene(camera);
	auto floor_node = scene.addObject(floor);
	auto pointLight_white_node = scene.addObject(pointLight_white);
	pointLight_white_node->setPosition(glm::vec3(0.f, 0.5f, 1.f));
	auto pointLight_red_node = scene.addObject(pointLight_red);
	pointLight_red_node->setPosition(glm::vec3(2.f, 0.5f, 1.f));
	auto pointLight_blue_node = scene.addObject(pointLight_blue);
	pointLight_blue_node->setPosition(glm::vec3(2.f, 0.5f, 1.4f));
	auto pointLight_green_node = scene.addObject(pointLight_green);
	pointLight_green_node->setPosition(glm::vec3(-2.f, 0.5f, 1.f));
	auto powerPointLight_node = scene.addObject(powerPointLight);
	powerPointLight_node->setPosition(glm::vec3(0.f, 2.f, 0.f));
	auto dirLight_node = scene.addObject(dirLight);
	dirLight_node->rotateDeg(glm::vec3(1.f, 0.f, 0.f), -45.f);
	std::cout << "[Scene] " << glGetError() << std::endl;

	// ======= SET SHADERS - SCENE OBJECTS ======= //
	floor->setShaderID(shader_main.getID());
	std::cout << "[SetShaders] " << glGetError() << std::endl;

	// ======= PRE-RENDERING UPDATES ======= //
	//pre-calculations
	glm::mat4 projection = glm::perspective(glm::radians(55.0f), (GLfloat)screenResolution.width / (GLfloat)screenResolution.height, 0.1f, 100.0f);

	// ======= CONFIGURATION ======= //
	ShaderConfiguration configuration = ShaderConfiguration();
	configuration.setMat4("projection", projection);

	//pre-render updates
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	std::cout << "[PreRendering] " << glGetError() << std::endl;

	// ======= RENDERING ======= //
	while (!glfwWindowShouldClose(window)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, mouse_callback);

		// ======= PROCESS INPUT ======= //
		processInput(window);
		calculateMouseOffset();
		scene.processMouse(xOffset, yOffset);
		scene.processInput(window);

		// ======= RENDERING ======= //
		glClearColor(1.5f, 1.5f, 4.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fb_main.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fb_blur_horizontal.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fb_blur_vertical.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fb_hdr.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		{
			GLenum gl_error = glGetError();
			if (gl_error) std::cout << "[Rendering][Init] " << gl_error << std::endl;
		}

		scene.draw(&configuration, map_shader_fbs);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		bool horizontal = true, first_iteration = true;
		unsigned int amount = 4;
		shader_manager.useShader(shader_post_blur.getID());
		for (unsigned int i = 0; i < amount; i++) {
			(horizontal ? fb_blur_horizontal : fb_blur_vertical).use();
			shader_post_blur.setBool("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);

			// bind texture of other framebuffer (or scene if first iteration)
			glBindTexture(GL_TEXTURE_2D, first_iteration ? fb_main.getColorBufferID(1)
				: (horizontal ? fb_blur_vertical : fb_blur_horizontal).getColorBufferID(0));
			renderPostFxNDC();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader_manager.useShader(shader_post_hdr.getID());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fb_main.getColorBufferID(0));
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? fb_main.getColorBufferID(1) : (horizontal ? fb_blur_vertical : fb_blur_horizontal).getColorBufferID(0));
		//glBindTexture(GL_TEXTURE_2D, fb_main.getColorBufferID(1));
		shader_post_hdr.setFloat("exposure", exposure);
		renderPostFxNDC();

		//handle events and swap buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
		{
			GLenum gl_error = glGetError();
			if (gl_error) std::cout << "[Rendering][Processing] " << gl_error << std::endl;
		}
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
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

void retesselatePlane(SceneMesh<VertexFormat>* obj, PlaneGenerator planeGen, unsigned int new_tesselation, glm::vec3 plane_position, glm::vec3 plane_normal, glm::vec3 plane_direction) {
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