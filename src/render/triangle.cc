#include "triangle.h"
#include <algorithm>
#include <display/display.h>
#include <matrix.h>
#include <render/render.h>
#include <render/zbuf.h>

/* return signed area of the triangle ABP multiplied by 2.
 * if point p at the right hand side of AB the result is positive. If the point
 * exactly on the line - 0, otherwise - negative.
 */
static float edge_function(const vec3f_t& a, const vec3f_t& b, const vec3f_t& p)
{
	return (a.x - b.x) * (p.y - a.y) - (a.y - b.y) * (p.x - a.x);
}

static uint8_t get_r(uint32_t color)
{
	return (color & 0xFF0000) >> 16;
}

static uint8_t get_g(uint32_t color)
{
	return (color & 0x00FF00) >> 8;
}

static uint8_t get_b(uint32_t color)
{
	return color & 0x0000FF;
}

static uint32_t make_color(float r, float g, float b)
{
	return (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
}

static uint32_t make_color(float intensity)
{
	uint32_t c = (uint32_t)(intensity * 255);

	return c << 16 | c << 8 | c;
}

/* TODO: move to suitable place */
static struct {
	const uint32_t *color = nullptr; /* colors in RGB888 format */
	size_t w;
	size_t h;

	uint32_t operator()(size_t u, size_t v)
	{
		assert(u < w);
		assert(v < h);

		return color[v * w + u];
	}

	uint32_t operator()(float u, float v)
	{
		assert(u <= 1.f);
		assert(v <= 1.f);

		size_t ui = (size_t)(u * (w - 1));
		size_t vi = (size_t)(v * (h - 1));

		return color[vi * w + ui];
	}
} texture;

void render::set_texture(const std::vector<uint32_t>& image, size_t width, size_t height)
{
	texture.color = image.data();
	texture.w = width;
	texture.h = height;
}

void render::triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	auto [width, height] = display::get_resolution();

	auto p0 = project_to_screen(v0.v);
	auto p1 = project_to_screen(v1.v);
	auto p2 = project_to_screen(v2.v);

	/* bounding box */
	vec2i_t bbox_min = { (int)std::min({p0.x, p1.x, p2.x}), (int)std::min({p0.y, p1.y, p2.y}) };
	vec2i_t bbox_max = { (int)std::max({p0.x, p1.x, p2.x}), (int)std::max({p0.y, p1.y, p2.y}) };

	bbox_min.x = std::max(bbox_min.x, 0);
	bbox_min.x = std::min(bbox_min.x, width - 1);
	bbox_min.y = std::max(bbox_min.y, 0);
	bbox_min.y = std::min(bbox_min.y, height - 1);
	bbox_max.x = std::max(bbox_max.x, 0);
	bbox_max.x = std::min(bbox_max.x, width - 1);
	bbox_max.y = std::max(bbox_max.y, 0);
	bbox_max.y = std::min(bbox_max.y, height - 1);

	for (int y = bbox_min.y; y <= bbox_max.y; y++) {
		/* TODO: if we have found at least on point that belongs to a triangle
		 * in current row and current point doesn't belong to the triangle then
		 * there is no reason to continue scanning this row.
		 */
		/* TODO: it's possible to remember left border on the previous row and
		 * start scanning to the left from this position. it's possible there
		 * are few or none pixels which belong to the triangle
		 */
		for (int x = bbox_min.x; x <= bbox_max.x; x++) {
			vec3f_t p{ x + 0.5f, y + 0.5f, 0.f };

			/* to barycentric coordinates */
			auto edge = p2 - p1;
			/* w0: signed area of the triangle v1v2p multiplied by 2 */
			auto w0 = edge_function(p1, p2, p);
			/* check if edge function is positive, if the point is on the edge,
			 * check if it is a top or left edge
			 */
			if (w0 < 0 || (w0 == 0 && ((edge.y == 0 && edge.x <= 0) || edge.y < 0)))
			//if (w0 < 0)
				continue;

			edge = p0 - p2;
			/* w1: signed area of the triangle v2v0p multiplied by 2 */
			auto w1 = edge_function(p2, p0, p);
			/* check if edge function is positive, if the point is on the edge,
			 * check if it is a top or left edge
			 */
			if (w1 < 0 || (w1 == 0 && ((edge.y == 0 && edge.x <= 0) || edge.y < 0)))
			//if (w1 < 0)
				continue;

			edge = p1 - p0;
			/* w2: signed area of the triangle v0v1p multiplied by 2 */
			auto w2 = edge_function(p0, p1, p);
			/* check if edge function is positive, if the point is on the edge,
			 * check if it is a top or left edge
			 */
			if (w2 < 0 || (w2 == 0 && ((edge.y == 0 && edge.x <= 0) || edge.y < 0)))
			//if (w2 < 0)
				continue;

			/* If we are here the point{x, y} is inside the triangle{p0, p1, p2}
			 * Calculate area of the triangle multiplied by 2 and normalize
			 * coefficients
			 */
			auto area = edge_function(p0, p1, p2);
			w0 /= area;
			w1 /= area;
			w2 /= area;

			/* depth test */
			p.z = w0 * v0.v.z + w1 * v1.v.z + w2 * v2.v.z;
			if (!zbuf::put(x, y, p.z))
				continue;

			/* calculate normal */
			vec3f_t n{ w0 * v0.norm + w1 * v1.norm + w2 * v2.norm };

			/* calculate light intensity */
			float intensity = n.z; /* TODO: multiply by light vector */
			/* back-face culling */
			if (intensity < 0.f)
				continue;

			/* calculate color */
			uint32_t color;
			if (texture.color != nullptr) {
				/* calculate texture coordinate */
				vec2f_t tex = w0 * v0.tex + w1 * v1.tex + w2 * v2.tex;

				uint32_t t = texture(tex.u, tex.v);
				float r = intensity * get_r(t);
				float g = intensity * get_g(t);
				float b = intensity * get_b(t);

				color = make_color(r, g, b);
			} else {
				/* calculate color from vertex color */
				//float r = w0 * get_r(v0.color) + w1 * get_r(v1.color) + w2 * get_r(v2.color);
				//float g = w0 * get_g(v0.color) + w1 * get_g(v1.color) + w2 * get_g(v2.color);
				//float b = w0 * get_b(v0.color) + w1 * get_b(v1.color) + w2 * get_b(v2.color);
				//display::put(x, y, make_color(r, g, b));

				color = make_color(intensity);
			}
			display::put(x, y, color);
		}
	}
}

void render::triangle(const std::vector<::model_t::Face>& faces,
	const std::vector<std::vector<float>>& vertices,
	const std::vector<std::vector<float>>& normals,
	const std::vector<std::vector<float>>& texture_uv)
{
	for (auto& face : faces) {
		Vertex v[3];

		for (size_t i = 0; i < 3; i++)
			v[i].v = { vertices[(size_t)face.v_idx[i] - 1][0],
				vertices[(size_t)face.v_idx[i] - 1][1],
				vertices[(size_t)face.v_idx[i] - 1][2] };

		for (size_t i = 0; i < 3; i++)
			v[i].norm = { normals[(size_t)face.n_idx[i] - 1][0],
				normals[(size_t)face.n_idx[i] - 1][1],
				normals[(size_t)face.n_idx[i] - 1][2] };

		for (size_t i = 0; i < 3; i++)
			v[i].tex = { texture_uv[(size_t)face.tex_idx[i] - 1][0],
				texture_uv[(size_t)face.tex_idx[i] - 1][1] };

		triangle(v[0], v[1], v[2]);
	}
}
