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
#include <glm/gtc/matrix_transform.hpp>
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

void createObjects() {
	for (int i = 0; i <= 10; ++i) {
		objects.push_back(Obj3D("models/rock/model1.obj", "models/rock/texture.dds"));
		objects[i].init();

		objects[i].position = vec3(rand() % 6, rand() % 6, rand() % 6);	
		objects[i].speed = vec3(rand() % 100 / 10000.f, rand() % 100 / 10000.f, rand() % 100 / 10000.f);
	}
}

void updateLoop() {
	for (std::vector<Obj3D>::iterator obj = objects.begin(); obj != objects.end(); ++obj) {
		obj->update();
	}
}

// Shader uniform identifiers
GLuint MatrixID, ViewMatrixID, ModelMatrixID, LightID, TextureID;

void drawLoop(GLuint programID, vec3 lightPos) {
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

	glUseProgram(programID);

	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();

	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();


	// Send MVP
	
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	for (std::vector<Obj3D>::iterator obj  = objects.begin(); obj != objects.end(); ++obj) {
		
		// Set the position of our model
		glm::mat4 ModelMatrix = translate(mat4(1.0f), obj->position);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		// Bind our texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->Texture);
		glUniform1i(TextureID, 0);

		// Vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, obj->VBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, obj->UVBO);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		// Normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, obj->NBO);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

		glDrawArrays(GL_TRIANGLES, 0, obj->model->vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(3);
	}

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();

}

std::map<std::string, Model> Obj3D::modelCache;
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

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT/ 2);

	// Dark blue background
	glClearColor(0.0f, 0.05f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);


	// Compile our shaders
	GLuint programID = LoadShaders("light.vertexshader", "light.fragmentshader");

	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	TextureID = glGetUniformLocation(programID, "myTextureSampler");

	vec3 lightPos(50, 50, 50);

	createObjects();
	
	lastTime = glfwGetTime();

	srand(time((unsigned int)0));

	do {
		updateLoop();
		drawLoop(programID, lightPos);
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	
	// Delete all objects
	objects.clear();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

