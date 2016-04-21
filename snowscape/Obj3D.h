#pragma once

#include <vector>
#include <map>
#include <GL/glew.h>

#include <glm/glm.hpp>
using namespace glm;

#include "objloader.hpp"
#include "texture.hpp"

struct Model {
	std::vector <vec3> vertices;
	std::vector <vec2> UVs;
	std::vector <vec3> normals;
};

class Obj3D {
	public:
		static std::map<std::string, Model> modelCache;
		static std::map<std::string, GLuint> textureCache;

		Model *model;

		GLuint Texture;
		GLuint VertexArrayID, VBO, UVBO, NBO;
		char *modelPath, *texturePath;
		vec3 position, speed;

		Obj3D(char * modelPath, char * texturePath);
		~Obj3D();
		void init();
		void update();
};

