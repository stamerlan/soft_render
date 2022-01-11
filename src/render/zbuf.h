#ifndef RENDER_ZBUF_H_
#define RENDER_ZBUF_H_

namespace render::zbuf {

/** Initialize depth buffer
 *
 * @param w: buffer width (in screen coordinates).
 * @param h: buffer height (in screen coordinates).
 * @return 0 on success.
 */
int init(int w, int h);

/** Clear depth buffer */
void clear(void);

/** Release depth buffer resources.
 *
 * @note It's safe to invoke the function if init() failed or has never been
 * invoked.
 */
void release(void);

/** Do depth test and update depth for the pixel.
 * If z is less than current depth value of the pixel, store new depth value of
 * the pixel and return true, false otherwise.
 *
 * @param x: x in screen coordinates.
 * @param y: y in screen coordinates.
 * @param z: depth.
 * @return If depth test passed or not.
 */
bool depth_test(int x, int y, float z);

} /* namespace render::zbuf */

#endif /* RENDER_ZBUF_H_ */
