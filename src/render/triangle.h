#ifndef RENDER_TRIANGLE_H_
#define RENDER_TRIANGLE_H_

#include <cstdint>
#include "vec.h"

namespace render {

struct Vertex {
	Vec3f v;
	Vec3f norm;
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

} /* namespace render */

#endif /* RENDER_TRIANGLE_H_ */
