/**
 * Model class implementation to load a model from file and allocate its data
 * on heap using STL containers.
 */
#include "model.h"
#include <cassert>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

static void trim(std::string &s)
{
	/* left trim */
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) {
			return !std::isspace(c);
		}));
	/* right trim */
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) {
			return !std::isspace(c);
		}).base(), s.end());
}

static std::vector<float> parse_coord(std::string_view v)
{
	std::vector<float> ret;

	for (auto p = v.data(); p < v.data() + v.size();) {
		float f;
		auto [next, ec] = std::from_chars(p, v.data() + v.size(), f);
		if (ec != std::errc())
			return {};
		ret.push_back(f);

		/* skip spaces between numbers */
		for (; next < v.data() + v.size() && isspace(*next); next++);
		p = next;
	}

	return ret;
}

static std::vector<int> parse_faces(std::string_view v)
{
	std::vector<int> ret;

	for (auto p = v.data(); p < v.data() + v.size();) {
		int f;
		auto [next, ec] = std::from_chars(p, v.data() + v.size(), f);
		if (ec != std::errc())
			return {};
		ret.push_back(f);

		/* skip until space */
		for (; next < v.data() + v.size() && !isspace(*next); next++);
		/* skip spaces */
		for (; next < v.data() + v.size() && isspace(*next); next++);
		p = next;
	}

	return ret;
}

Model::Model(const char *name) noexcept
{
	is_loaded_ = false;

	std::ifstream file(name);
	if (!file.is_open()) {
		std::cerr << "Failed to open " << std::quoted(name) << "\n";
		return;
	}

	std::string str;

	while (std::getline(file, str)) {
		/* TODO: use string_view to avoid copy */
		trim(str);

		if (str[0] == '#' || str.empty()) {
			/* a comment or empty line */
			continue;
		}

		if (str.starts_with("v ")) {
			/* TODO: list of geometric vertices, with (x, y, z [,w])
			 * coordinates, w is optional and defaults to 1.0.
			 */
			auto coord = parse_coord({ str.begin() + 2, str.end() });
			if (coord.size() != 3) {
				std::cerr << "Unexpected vertex: " << std::quoted(str) << ". {";
				for (auto f : coord) {
					std::cerr << " " << f;
				}
				std::cerr << " }\n";
				return;
			}
			vertices_.insert(vertices_.end(), coord.begin(), coord.end());
			continue;
		}

		if (str.starts_with("vn ")) {
			/* TODO: List of vertex normals in (x,y,z) form; normals might not
			 * be unit vectors. */
			continue;
		}

		if (str.starts_with("vt ")) {
			/* TODO: List of texture coordinates, in (u, [,v ,w]) coordinates,
			 * these will vary between 0 and 1. v, w are optional and default to
			 * 0. */
			continue;
		}

		if (str.starts_with("f ")) {
			/* TODO: Polygonal face element (see below)
			 * f 1 2 3             # Vertex indices
			 * f 3/1 4/2 5/3       # Vertex texture coordinate indices
			 * f 6/4/1 3/5/3 7/6/5 # Vertex normal indices
			 * f 7//1 8//2 9//3    # Vertex normal indices w/o texture coordinates
			 */
			auto faces = parse_faces({ str.begin() + 2, str.end() });
			if (faces.size() != 3) {
				std::cerr << "Unexpected faces: " << std::quoted(str) << ". {";
				for (auto f : faces) {
					std::cerr << " " << f;
				}
				std::cerr << " }\n";
				return;
			}
			/* TODO: check if faces are valid */
			faces_.insert(faces_.end(), faces.begin(), faces.end());
			continue;
		}

		if (str.starts_with("g ") || (str.size() == 1 && str[0] == 'g')) {
			/* skip group name */
			continue;
		}

		if (str.starts_with("s ")) {
			/* skip smooth shading */
			continue;
		}

		if (str.starts_with("mtllib")) {
			/* skip material lib */
			continue;
		}

		if (str.starts_with("usemtl ")) {
			/* skip material name */
			continue;
		}

		/* TODO: Log warning */
		assert(false);
	}

	std::cerr << "Model " << std::quoted(name) << " loaded. Faces: " <<
		faces_.size() << ", Vertices: " << vertices_.size() << "\n";
	is_loaded_ = true;
}

bool Model::is_loaded(void) const noexcept
{
	return is_loaded_;
}
