#include "Obj3d.h"
#include "vboindexer.hpp"
#include "tangentspace.hpp"

Obj3D::Obj3D(char * modelPath,char * texturePath, char * normalTexturePath) {
	this->modelPath = modelPath;
	this->texturePath = texturePath;
	this->normalTexturePath = normalTexturePath;

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

		// Read object from file
		loadOBJ(modelPath, newModel->vertices, newModel->UVs, newModel->normals);
		computeTangentBasis(
			newModel->vertices, newModel->UVs, newModel->normals, // input
			newModel->tangents, newModel->bitangents    // output
		);

		// VBO indexing
		indexVBO_TBN(newModel->vertices, newModel->UVs, newModel->normals, newModel->tangents, newModel->bitangents,
			// Output
			newModel->indices, newModel->indexed_vertices, newModel->indexed_UVs, 
			newModel->indexed_normals, newModel->indexed_tangents, newModel->indexed_bitangents);

		glGenVertexArrays(1, &newModel->VertexArrayID);
		glBindVertexArray(newModel->VertexArrayID);

		glGenBuffers(1, &newModel->VBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * newModel->indexed_vertices.size(), &newModel->indexed_vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->UVBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->UVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * newModel->indexed_UVs.size(), &newModel->indexed_UVs[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->NBO);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->NBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * newModel->indexed_normals.size(), &newModel->indexed_normals[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->tangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->tangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, newModel->indexed_tangents.size() * sizeof(glm::vec3), &newModel->indexed_tangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->bitangentbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, newModel->bitangentbuffer);
		glBufferData(GL_ARRAY_BUFFER, newModel->indexed_bitangents.size() * sizeof(glm::vec3), &newModel->indexed_bitangents[0], GL_STATIC_DRAW);

		glGenBuffers(1, &newModel->elementbuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newModel->elementbuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, newModel->indices.size() * sizeof(unsigned short), &newModel->indices[0], GL_STATIC_DRAW);
	}

	model = Obj3D::modelCache[modelPath];

	// Load texture
	if (Obj3D::textureCache.count(texturePath) == 0) {
		const char *path = texturePath;
		Texture = loadDDS(texturePath);
		Obj3D::textureCache[texturePath] = Texture;
	}
	Texture = Obj3D::textureCache[texturePath];

	// Load normal texture
	if (normalTexturePath != NULL) {
		if (Obj3D::textureCache.count(normalTexturePath) == 0) {
			const char *path = normalTexturePath;
			NormalTexture = loadBMP_custom(path);
			Obj3D::textureCache[normalTexturePath] = NormalTexture;
		}
		NormalTexture = Obj3D::textureCache[normalTexturePath];
	}
}

Obj3D::~Obj3D() {
	
}
