#ifndef RENDER_TRIANGLE_H_
#define RENDER_TRIANGLE_H_

#include <cstdint>
#include <vector>
#include <model/model.h>
#include <vector.h>

namespace render {

/** A triangle vertex descriptor */
struct Vertex {
	vec3f_t v;      /**< Geometric vertex */
	vec3f_t norm;   /**< Vertex normal */
	vec2f_t tex;    /**< Texture coordinates */
	uint32_t color; /**< Vertex color. Not used if texture is set */
};

/** Render a triangle in model coordinates
 * Vertices passed counter clockwise. Texture has to be set by set_texture()
 * call. If texture is not set light intensity is used to color the triangle.
 * 
 * @param v0: A triangle vertex
 * @param v1: A triangle vertex
 * @param v2: A triangle vertex
 */
void triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);

/** Render triangles
 *
 * @param faces: an array of faces.
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

/** Set current texture
 *
 * @param image: texture data in RGB888 format.
 * @param width: texture width.
 * @param height: texture height.
 *
 * @note Texture data must be available while texture is used. The function
 * doesn't perform a copy of the texture, but stores a pointer to it.
 * @todo it's temporary
 */
void set_texture(const std::vector<uint32_t>& image, size_t width, size_t height);

} /* namespace render */

#endif /* RENDER_TRIANGLE_H_ */
