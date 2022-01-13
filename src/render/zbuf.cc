#include "zbuf.h"
#include <vector>

static unsigned width;
static unsigned height;

static std::vector<float> zbuffer;

int render::zbuf::init(int w, int h)
{
	if (w <= 0 || h <= 0)
		return 1;

	zbuffer.resize(w * h);
	width = w;
	height = h;
	return 0;
}

void render::zbuf::release(void)
{
	zbuffer.resize(0);
}

void render::zbuf::clear(void)
{
	std::fill(zbuffer.begin(), zbuffer.end(), std::numeric_limits<float>::lowest());
}

bool render::zbuf::depth_test(int x, int y, float z)
{
	if (x < 0 || (unsigned)x >= width || y < 0 || (unsigned)y >= height)
		return false;

	if (z > zbuffer[(size_t)y * width + x]) {
		zbuffer[(size_t)y * width + x] = z;
		return true;
	}
	return false;
}
