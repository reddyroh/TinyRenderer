#pragma once

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3i> faces_;
	std::vector<Vec3f> vertstex_;
	std::vector<Vec3i> facestex_;

public: 
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3i face(int i);
	Vec3f vertTex(int i);
	Vec3i faceTex(int i);
};