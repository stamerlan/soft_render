#include "triangle.h"
#include <algorithm>
#include <display/display.h>
#include <render/zbuf.h>

/* project geometric vertices to screen space
 * TODO: smth like that?
 *   float nearClippingPlane = 0.1; 
 *   // point in camera space
 *   Vec3f pCamera; 
 *   worldToCamera.multVecMatrix(pWorld, pCamera); 
 *   // convert to screen space
 *   Vec2f pScreen; 
 *   pScreen.x = nearClippingPlane * pCamera.x / -pCamera.z;
 *   pScreen.y = nearClippingPlane * pCamera.y / -pCamera.z;
 *   // now convert point from screen space to NDC space (in range [-1,1])
 *   Vec2f pNDC; 
 *   pNDC.x = 2 * pScreen.x / (r - l) - (r + l) / (r - l);
 *   pNDC.y = 2 * pScreen.y / (t - b) - (t + b) / (t - b);
 *   // convert to raster space and set point z-coordinate to -pCamera.z
 *   Vec3f pRaster; 
 *   pRaster.x = (pScreen.x + 1) / 2 * imageWidth;
 *   // in raster space y is down so invert direction
 *   pRaster.y = (1 - pScreen.y) / 2 * imageHeight;
 *   // store the point camera space z-coordinate (as a positive value)
 *   pRaster.z = -pCamera.z; 
 */
static Vec3f project_to_screen(const render::Vertex& v)
{
	auto [width, height] = display::get_resolution();
	float x = (v.v.x + 1.f) * width / 2;
	float y = (1.f - v.v.y) * height / 2;
	float z = v.v.z;
	return { x, y, z };
}

/* return signed area of the triangle ABP multiplied by 2.
 * if point p at the right hand side of AB the result is positive. If the point
 * exactly on the line - 0, otherwise - negative.
 */
static float edge_function(const Vec3f& a, const Vec3f& b, const Vec3f& p)
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

void render::triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	auto [width, height] = display::get_resolution();

	auto p0 = project_to_screen(v0);
	auto p1 = project_to_screen(v1);
	auto p2 = project_to_screen(v2);

	/* bounding box */
	Vec2i bbox_min = { (int)std::min({p0.x, p1.x, p2.x}), (int)std::min({p0.y, p1.y, p2.y}) };
	Vec2i bbox_max = { (int)std::max({p0.x, p1.x, p2.x}), (int)std::max({p0.y, p1.y, p2.y}) };

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
			Vec3f p{ x + 0.5f, y + 0.5f, 0.f };

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
			if (!zbuf::depth_test(x, y, p.z))
				continue;

			/* calculate normal */
			Vec3f n{ w0 * v0.norm + w1 * v1.norm + w2 * v2.norm };

			/* calculate color */
			float intensity = n.z;
			/* back-face culling */
			if (intensity < 0.f)
				continue;
			uint32_t color = make_color(intensity);

			/* TODO: GET COLOR FROM TEXTURE */
			/* TODO: CALCULATE LIGHTING */

			//float r = w0 * get_r(v0.color) + w1 * get_r(v1.color) + w2 * get_r(v2.color);
			//float g = w0 * get_g(v0.color) + w1 * get_g(v1.color) + w2 * get_g(v2.color);
			//float b = w0 * get_b(v0.color) + w1 * get_b(v1.color) + w2 * get_b(v2.color);
			//display::put(x, y, make_color(r, g, b));
			display::put(x, y, make_color(intensity));
		}
	}
}

void render::triangle(const std::vector<::Model::Face>& faces,
	const std::vector<std::vector<float>>& vertices,
	const std::vector<std::vector<float>>& normals)
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

		triangle(v[0], v[1], v[2]);
	}
}
