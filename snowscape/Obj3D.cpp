#include "Obj3d.h"

Obj3D::Obj3D(char * modelPath,char * texturePath) {
	this->modelPath = modelPath;
	this->texturePath = texturePath;
	//speed = vec3(0.0f);
	//position = vec3(0.0f);
	//rotation = vec3(0.0f);
	scale = vec3(1.0f);
	depthTest = true;
}

// Basic update function, no dt for now, time step is fixed
void Obj3D::update() {
	position += speed;
}

void Obj3D::init() {
	// Load model if not in cache
	if (Obj3D::modelCache.count(modelPath) == 0) {
		Obj3D::modelCache.insert(std::make_pair(modelPath, new Model()));
		Model *newModel = modelCache[modelPath];
		loadOBJ(modelPath, newModel->vertices, newModel->UVs, newModel->normals);
		
		glGenVertexArrays(1, &newModel->VertexArrayID);
		glBindVertexArray(newModel->VertexArrayID);

		glGenBuffers(1, &newModel->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * newModel->vertices.size(), &newModel->vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->UVBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->UVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * newModel->UVs.size(), &newModel->UVs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->NBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->NBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * newModel->normals.size(), &newModel->normals[0], GL_STATIC_DRAW);

		
	}

	model = Obj3D::modelCache[modelPath];

	// Load texture
	if (Obj3D::textureCache.count(texturePath) == 0) {
		const char *path = texturePath;
		Texture = loadDDS(texturePath);
		Obj3D::textureCache[texturePath] = Texture;
	}

	Texture = Obj3D::textureCache[texturePath];
}

Obj3D::~Obj3D() {
	
}
