#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <cstdint>
#include <tuple>
#include <message_queue.h>

namespace display {

/** Initialize display.
 *
 * @param w: display width
 * @param h: display height
 * @return 0 on success.
 * 
 * @note The function should be called before any other in this module.
 */
/* TODO: return std::errc. std::errc() on success */
int init(int w = 600, int h = 600);

/** Release display resources.
 *
 * @note It's safe to invoke the function if init() failed or has never been
 * invoked.
 */
void release(void);

/** Clear display buffer.
 *
 * @param color: color in RGB888 format to fill display buffer with.
 */
void clear(uint32_t color = 0);

/** Flush data from frame buffer to display
 *
 * @return 0 on success.
 */
int update(void);

/** Draw a point at frame buffer.
 *
 * @param x: column number (0 - the leftmost column)
 * @param y: row number (0 - the topmost row)
 * @param color: the point color in RGB888 format
 *
 * @note the point will not be shown until update() invoked.
 */
void put(int x, int y, uint32_t color);

/** Get screen resolution
 *
 * @return tuple: {width, height}
 */
std::tuple<int, int> get_resolution(void);

/** Get an event from display
 * Display may be represented by touchscreen or by a window in OS. Those devices
 * can post some events (mouse click, touch of the screen, etc.)
 *
 * @param m: a reference to an object to place message to.
 * @return 1 if there is a pending event or 0 if there are no events in queue.
 *
 * @note The function never returns any message if module is not initialized.
 */
int get_msg(Message& m);

} /* namespace display */

#endif /* DISPLAY_H_ */
