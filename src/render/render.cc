#include "render.h"
#include <display/display.h>

static bool zbuf_enabled = true;

int render::init(int w, int h)
{
	if (display::init(w, h))
		return 1;
	if (zbuf::init(w, h)) {
		display::release();
		return 1;
	}

	return 0;
}

void render::release(void)
{
	zbuf::release();
	display::release();
}

void render::clear(void)
{
	display::clear();
	zbuf::clear();
}

int render::update(void)
{
	return display::update();
}

bool render::is_zbuf_enabled(void)
{
	return zbuf_enabled;
}

void zbuf_enable(bool en)
{
	zbuf_enabled = en;
}
