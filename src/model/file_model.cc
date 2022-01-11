/**
 * Model class implementation to load a model from file and allocate its data
 * on heap using STL containers.
 */
#include "model.h"
#include <cassert>
#include <charconv>
#include <fstream>
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

Model::Model(const char *name) noexcept
{
	is_loaded_ = false;

	std::ifstream file(name);
	std::string str;

	std::vector<std::vector<float>> v;

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
			if (coord.size() != 3)
				return;
			v.push_back(coord);
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
			continue;
		}

		if (str.starts_with("g ")) {
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

	is_loaded_ = true;
}

bool Model::is_loaded(void) const noexcept
{
	return false;
}
