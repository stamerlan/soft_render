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

} /* namespace render */

#endif /* RENDER_RENDER_H_ */
