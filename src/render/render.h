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

/** Project a geometric vertex to screen space.
 * Apply model, view and projection transformations
 *
 * @param v: model vertex
 */
vec3f_t project_to_screen(const vec3f_t& v);

/** Set camera position.
 *
 * @param eye: camera position (world coordinates)
 * @param at: point to look at (world coordinates)
 * @param up: vector pointing to the upward direction (world coordinates)
 */
void lookat(const vec3f_t& eye, const vec3f_t& at, const vec3f_t& up);

namespace model_mat {

/** Load identity matrix. */
void identity(void);

/** Set model scaling.
 * Multiply current model matrix by scaling matrix.
 *
 * @param x: scale factor for x axis
 * @param y: scale factor for y axis
 * @param z: scale factor for z axis
 */
void scale(float x, float y, float z);

/** Set model translation.
 * Multiply current model matrix by translation matrix. In other words move a
 * model.
 *
 * @param x: move by x axis
 * @param y: move by y axis
 * @param z: move by z axis
 */
void translate(float x, float y, float z);

/** Rotate model around X axis.
 * Multiply current model matrix by rotation matrix.
 *
 * @param angle: angle expressed in radians.
 */
void rotate_x(float angle);

/** Rotate model.
 * Multiply current model matrix by rotation matrix.
 *
 * @param angle: angle expressed in radians.
 * @param x, y, z: specify x, y and z coordinates of a rotation vector.
 */
void rotate(float angle, float x, float y, float z);

} /* namespace model_mat */

} /* namespace render */

#endif /* RENDER_RENDER_H_ */
