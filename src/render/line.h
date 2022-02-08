#ifndef RENDER_LINE_H_
#define RENDER_LINE_H_

#include <cstdint>
#include <vector.h>

namespace render {

/** Draw a line in screen coordinates
 *
 * @param x0:
 * @param y0:
 * @param x1:
 * @param y1:
 * @param color: the line color in RGB888 format
 */
void line(int x0, int y0, int x1, int y1, uint32_t color);

/** Draw a line in normalized device coordinates (-1.0, 1.0)
 *
 * @param p0: point 0 vector
 * @param p1: point 1 vector
 * @param color: the line color in RGB888 format
 */
void line(vec2f_t p0, vec2f_t p1, uint32_t color);

/** Draw a line in normalized device coordinates (-1.0, 1.0)
 *
 * @param p0: point 0 vector
 * @param p1: point 1 vector
 * @param color: the line color in RGB888 format
 */
void line(vec3f_t p0, vec3f_t p1, uint32_t color);

} /* namespace render */

#endif /* RENDER_LINE_H_ */
