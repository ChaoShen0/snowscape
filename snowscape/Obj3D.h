#ifndef OBJ3D_H
#define OBJ3D_H

#include <vector>
#include <map>
#include <GL/glew.h>

#include <glm/glm.hpp>
using namespace glm;

#include "objloader.hpp"
#include "texture.hpp"

struct Model {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_UVs;
	std::vector<glm::vec3> indexed_normals;
	std::vector<glm::vec3> indexed_tangents;
	std::vector<glm::vec3> indexed_bitangents;

	GLuint VertexArrayID, VBO, UVBO, NBO, elementbuffer, tangentbuffer, bitangentbuffer;

	~Model() {
		printf("Model destructor called \n");
	}
};

class Obj3D {
	public:
		static std::map<std::string, Model*> modelCache;
		static std::map<std::string, GLuint> textureCache;

		Model *model;
		GLuint Texture, NormalTexture;
		char *modelPath, *texturePath, *normalTexturePath;
		vec3 position, speed, rotation, scale;
		bool depthTest;

		Obj3D(char * modelPath, char * texturePath, char * normalTexturePath = "models/default_normal.bmp");
		~Obj3D();
		void init();
		void update();
};

#endif