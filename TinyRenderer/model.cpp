#include "model.h"
#include <fstream>
#include <sstream>

Model::Model(const char* filename) : verts_(), faces_(), vertstex_(), facestex_() {

	std::ifstream file;
	file.open(filename, std::ifstream::in);
	if (file.fail()) {
		std::cerr << "File not found" << std::endl;
		return;
	}

	std::string datatype, line;

	while (!file.eof()) {
		getline(file, line);
		std::istringstream stream(line);
		stream >> datatype;

		if (datatype == "v") {
			Vec3f v;
			stream >> v.x >> v.y >> v.z;
			verts_.push_back(v);
		}
		else if (datatype == "vt") {
			Vec3f vt;
			stream >> vt.x >> vt.y >> vt.z;
			vertstex_.push_back(vt);
		}
		else if (datatype == "f") {
			Vec3i face, facetex;
			char sep;
			int itrash, index, texindex;

			for (int j = 0; j < 3; j++) {
				stream >> index >> sep >> texindex >> sep >> itrash;
				face.raw[j] = index - 1; // in wavefront obj, all indices start at 1, not 0
				facetex.raw[j] = texindex - 1;
			}	

			faces_.push_back(face);
			facestex_.push_back(facetex);
		}
	}

	std::cerr << "# v# " << verts_.size() << " f# " << faces_.size()  << " vt# " << vertstex_.size() << std::endl;

}

Model::~Model() {
}

int Model::nverts() {
	return static_cast<int>(verts_.size());
}

int Model::nfaces() {
	return static_cast<int>(faces_.size());
}

Vec3f Model::vert(int i) {
	return verts_[i];
}

Vec3i Model::face(int i) {
	return faces_[i];
}

Vec3f Model::vertTex(int i) {
	return vertstex_[i];
}

Vec3i Model::faceTex(int i) {
	return facestex_[i];
}