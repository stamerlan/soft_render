#ifndef RENDER_RENDER_H_
#define RENDER_RENDER_H_

#include "line.h"
#include "triangle.h"
#include "zbuf.h"

namespace render {

int init(int w = 600, int h = 600);
void release(void);
void clear(void);
int update(void);

bool is_zbuf_enabled(void);
void zbuf_enable(bool en);

/** Project a geometric vertex to screen space
 * Apply model, view and projection transformations
 *
 * @param v: model vertex
 */
vec3f_t project_to_screen(const vec3f_t& v);

/** Set camera position
 *
 * @param eye: camera position (world coordinates)
 * @param at: point to look at (world coordinates)
 * @param up: vector pointing to the upward direction (world coordinates)
 */
void lookat(const vec3f_t& eye, const vec3f_t& at, const vec3f_t& up);

} /* namespace render */

#endif /* RENDER_RENDER_H_ */
