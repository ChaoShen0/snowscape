// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include<glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

// Our premade functions
#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "objloader.hpp"

// High level, helper functions
#include "Obj3D.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

int nbFrames;
double lastTime;

std::vector<Obj3D> objects;
std::vector<Obj3D> objects_shader1;

void createObjects() {
	const float pi_over_2 = half_pi<float>();

	objects_shader1.push_back(Obj3D("models/skybox/model.obj", "models/skybox/texture2.dds"));
	objects_shader1.rbegin()->rotation.y = three_over_two_pi<float>();
	objects_shader1.rbegin()->scale = vec3(10.0f);
	objects_shader1.rbegin()->position.z = -20.0f;
	objects_shader1.rbegin()->depthTest = false;
	objects_shader1.rbegin()->init();

	objects_shader1.push_back(Obj3D("models/skybox/model.obj", "models/skybox/texture.dds"));
	objects_shader1.rbegin()->rotation.y = pi_over_2;
	objects_shader1.rbegin()->scale = vec3(10.0f);
	objects_shader1.rbegin()->depthTest = false;
	objects_shader1.rbegin()->init();
	
	for (int i = 0; i <= 10; ++i) {
		objects.push_back(Obj3D("models/rock/model1.obj", "models/rock/texture.dds", "models/rock/texture_normals.bmp"));
		objects.rbegin()->init();

		objects.rbegin()->position = vec3(rand() % 10, rand() % 10, rand() % 10);	
		objects.rbegin()->speed = vec3(rand() % 100 / 10000.f, rand() % 100 / 10000.f, rand() % 100 / 10000.f);
	}

	objects.push_back(Obj3D("models/house1/model1.obj", "models/house1/texture.dds", "models/house1/texture_normals.bmp"));
	objects.rbegin()->init();
}

void updateLoop() {
	for (std::vector<Obj3D>::iterator obj = objects.begin(); obj != objects.end(); ++obj) {
		obj->update();
	}
}

// Shader uniform identifiers
GLuint MatrixID, ViewMatrixID, ModelMatrixID, LightID, TextureID, NormalTextureID, ModelView3x3MatrixID;
GLuint programID, textureShaderID;
GLuint MatrixID_2, TextureID_2;

void drawLoop(vec3 lightPos) {
	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;
	if (currentTime - lastTime >= 1.0) {
		printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		nbFrames = 0;
		lastTime += 1.0;
	}

	// Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();

	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();

	// Texture only shader
	glUseProgram(textureShaderID);

	for (std::vector<Obj3D>::iterator obj = objects_shader1.begin(); obj != objects_shader1.end(); ++obj) {

		// Set the position of our model
		mat4 ModelViewMatrix = ViewMatrix * obj->getModelMatrix();
		mat4 MVP = ProjectionMatrix * ModelViewMatrix;

		if (obj->depthTest) {
			glDepthMask(GL_TRUE);
		}
		else {
			glDepthMask(GL_FALSE);
		}

		glUniformMatrix4fv(MatrixID_2, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->Texture);
		glUniform1i(TextureID_2, 0);

		// Verticies
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->UVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->model->elementbuffer);

		glDrawElements(GL_TRIANGLES, obj->model->indices.size(), GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	// Normal light shader
	glUseProgram(programID);

	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	for (std::vector<Obj3D>::iterator obj  = objects.begin(); obj != objects.end(); ++obj) {
		
		// Set the position of our model
		mat4 ModelMatrix = obj->getModelMatrix();
		mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
		mat3 ModelView3x3Matrix = mat3(ModelViewMatrix);
		mat4 MVP = ProjectionMatrix * ModelViewMatrix;

		if (obj->depthTest) {
			glDepthMask(GL_TRUE);
		}
		else {
			glDepthMask(GL_FALSE);
		}

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);

		// Bind our texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->Texture);
		glUniform1i(TextureID, 0);

		// Texture normals
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, obj->NormalTexture);
		glUniform1i(NormalTextureID, 1);
		
		// Vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->UVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->NBO);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Tangents
		glEnableVertexAttribArray(3);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->tangentbuffer);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Bitangents
		glEnableVertexAttribArray(4);
		glBindBuffer(GL_ARRAY_BUFFER, obj->model->bitangentbuffer);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj->model->elementbuffer);

		glDrawElements(GL_TRIANGLES, obj->model->indices.size(), GL_UNSIGNED_SHORT, (void*)0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);
	}

	
	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();

}

std::map<std::string, Model*> Obj3D::modelCache;
std::map<std::string, GLuint> Obj3D::textureCache;

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snowscape", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Vsync on
	glfwSwapInterval(1);
	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT/ 2);

	// Dark blue background
	glClearColor(0.0f, 0.05f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Compile our shaders
	programID = LoadShaders("light.vertexshader", "light.fragmentshader");
	textureShaderID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");

	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");
	NormalTextureID = glGetUniformLocation(programID, "normalTextureSampler");
	ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");

	MatrixID_2 = glGetUniformLocation(textureShaderID, "MVP");
	TextureID_2 = glGetUniformLocation(textureShaderID, "myTextureSampler");


	vec3 lightPos(-25, 50, 25);

	createObjects();
	
	lastTime = glfwGetTime();

	srand(time((unsigned int)0));

	vec2 dir(1);
	do {
		updateLoop();
		
		lightPos.y += 0.1f * dir.y;
		if (lightPos.y > 50.0f) lightPos.y = 50.0f, dir.y = -1;
		if (lightPos.y < 30.0f) lightPos.y = 30.0f, dir.y = 1;

		lightPos.x += 0.15f * dir.x;
		if (lightPos.x > 40.0f) lightPos.x = 40.0f, dir.x = -1;
		if (lightPos.x < -40.0f) lightPos.x = -40.0f, dir.x = 1;
		
		objects[3].position = lightPos;

		drawLoop(lightPos);
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDeleteProgram(programID);
	glDeleteProgram(textureShaderID);
	glDeleteTextures(1, &TextureID);
	glDeleteTextures(1, &NormalTextureID);
	
	// Delete all objects
	objects.clear();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

