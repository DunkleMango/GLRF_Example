#pragma once

#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLRF/AppFrame.hpp>
#include <GLRF/Scene.hpp>
#include <GLRF/Camera.hpp>
#include <GLRF/SceneObject.hpp>
#include <GLRF/SceneLight.hpp>
#include <GLRF/Shader.hpp>
#include <GLRF/VertexFormat.hpp>
#include <GLRF/PlaneGenerator.hpp>
#include <GLRF/Material.hpp>

class MyApp : public virtual GLRF::App
{
private:
public:
	MyApp();
	~MyApp();
	void configure(GLFWwindow* window);
	void processUserInput(GLFWwindow* window, glm::vec2 mouse_offset);
	void updateScene();
	void render();
};