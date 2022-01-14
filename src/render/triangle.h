#ifndef RENDER_TRIANGLE_H_
#define RENDER_TRIANGLE_H_

#include <cstdint>
#include <vector>
#include <model/model.h>
#include "vec.h"

namespace render {

struct Vertex {
	Vec3f v;
	Vec3f norm;
	Vec2f tex;
	uint32_t color;
};

/** Render a triangle in normalized device coordinates (-1.0, 1.0)
 * Vertices passed counter clockwise.
 * 
 * @param v0:
 * @param v1:
 * @param v2:
 */
void triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

/** Render triangles
 *
 * @param faces: an array of faces
 * @param faces_cnt: faces array size. must be multiple of 3.
 * @param vertices: an array of vertex coordinates.
 * @param normals: an array of normal coordinates.
 * @param texture_uv: an array of texture coordinates.
 *
 * @note The function doesn't check if input arrays are valid.
 */
void triangle(const std::vector<::Model::Face>& faces,
	const std::vector<std::vector<float>>& vertices,
	const std::vector<std::vector<float>>& normals,
	const std::vector<std::vector<float>>& texture_uv);

/* TODO: it's temporary */
void set_texture(const std::vector<uint32_t>& image, size_t width, size_t height);

} /* namespace render */

#endif /* RENDER_TRIANGLE_H_ */
