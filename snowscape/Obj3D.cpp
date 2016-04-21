#include "Obj3d.h"

Obj3D::Obj3D(char * modelPath,char * texturePath) {
	this->modelPath = modelPath;
	this->texturePath = texturePath;
	speed = vec3(0.0f);
	position = vec3(0.5f);
}

// Basic update function, no dt for now, time step is fixed
void Obj3D::update() {
	position += speed;
}

void Obj3D::init() {
	// Load model if not in cache
	if (Obj3D::modelCache.count(modelPath) == 0) {
		Model newModel;
		loadOBJ(modelPath, newModel.vertices, newModel.UVs, newModel.normals);
		Obj3D::modelCache[modelPath] = newModel;
	}

	model = &Obj3D::modelCache[modelPath];

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * model->vertices.size(), &model->vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &UVBO);
	glBindBuffer(GL_ARRAY_BUFFER, UVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * model->UVs.size(), &model->UVs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &NBO);
	glBindBuffer(GL_ARRAY_BUFFER, NBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * model->normals.size(), &model->normals[0], GL_STATIC_DRAW);

	// Load texture
	if (Obj3D::textureCache.count(texturePath) == 0) {
		const char *path = texturePath;
		Texture = loadDDS(texturePath);
		Obj3D::textureCache[texturePath] = Texture;
	}

	Texture = Obj3D::textureCache[texturePath];
}

Obj3D::~Obj3D() {
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &UVBO);
	glDeleteBuffers(1, &NBO);
	glDeleteVertexArrays(1, &VertexArrayID);
}
